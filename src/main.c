// Atmos compiler: command line.
//
//   atmos f.atm [args]     compiles, builds, and runs
//   atmos -c f.atm         emits C only (to stdout)
//   atmos -c -o dir f.atm  emits C only, one file per module in `dir`
//   atmos -o out f.atm     builds only
//   atmos -t f.atm         enables `test` blocks
//   atmos -e env.atm f.atm includes the file of an environment before
//                          the program
//
// A compiler dedicated to an environment (`make env`, as `atmos-pico`)
// is this same program built with the file of the environment, and the
// flags that it needs, as defaults: ATMOS_ENV, ATMOS_ENV_CFLAGS,
// ATMOS_ENV_LDFLAGS, and ATMOS_NAME.
//
// Environment:
//   ATMOS_HOME     root with `rt/` and `build/` (default: where the
//                  compiler is, or else where it was built)
//   ATMOS_CC       C compiler (default: cc)
//   ATMOS_CFLAGS   C flags (default: -O1)
//   ATMOS_LDFLAGS  linker flags, after the program (e.g. -lSDL2)
//   ATMOS_ASAN     if set, builds with the address sanitizer
//   ATMOS_FLOAT    if `32`, a float is a C float of 32 bits (default `64`)
//   ATMOS_INTEGER  if `32`, an integer has 32 bits (default `64`)
//   ATMOS_CLOCK    if `ms`, a clock is in milliseconds (default `us`)

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "atmc.h"

#ifndef ATMOS_HOME
#define ATMOS_HOME "."
#endif
#ifndef ATMOS_NAME
#define ATMOS_NAME "atmos"
#endif
#ifndef ATMOS_ENV
#define ATMOS_ENV NULL
#endif
#ifndef ATMOS_ENV_CFLAGS
#define ATMOS_ENV_CFLAGS ""
#endif
#ifndef ATMOS_ENV_LDFLAGS
#define ATMOS_ENV_LDFLAGS ""
#endif

static char* slurp (const char* path, size_t* len) {
    FILE* f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, ATMOS_NAME " : cannot open '%s'\n", path);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = xalloc(n + 1);
    if (fread(buf, 1, n, f) != (size_t)n) {
        fprintf(stderr, ATMOS_NAME " : cannot read '%s'\n", path);
        exit(1);
    }
    fclose(f);
    buf[n] = '\0';
    *len = n;
    return buf;
}

// Dumps one token per line: LIN SEP TAG STR.
static void dump_lex (void) {
    Tk tk;
    while (lexer_token(&tk)) {
        printf("%d %d %s %s", tk.lin, tk.sep, lexer_tag(tk.tag), tk.str);
        if (tk.tag == TK_CLK) {
            printf(" %.0f", tk.clk);
        }
        printf("\n");
    }
}

static const char* env (const char* name, const char* def) {
    const char* v = getenv(name);
    return (v!=NULL && v[0]!='\0') ? v : def;
}

// Root with `rt/` and `build/`: the variable ATMOS_HOME, or the
// directory of the compiler itself, if the runtime is there (so that a
// checkout may be moved), or the directory where it was built.
static const char* home_dir (void) {
    const char* v = getenv("ATMOS_HOME");
    if (v!=NULL && v[0]!='\0') {
        return v;
    }
    char exe[4096];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe)-1);
    if (n > 0) {
        exe[n] = '\0';
        char* sep = strrchr(exe, '/');
        if (sep != NULL) {
            *sep = '\0';
            char* hdr = xfmt("%s/rt/atmos.h", exe);
            if (access(hdr, R_OK) == 0) {
                return xfmt("%s", exe);
            }
        }
    }
    return ATMOS_HOME;
}

// An option of the build with two values: 0 for `v0` (the default), 1
// for `v1`. Any other value is an error. `old` is a former name of the
// variable, which is refused, so that it is not ignored in silence.
static int opt2 (const char* name, const char* old,
                 const char* v0, const char* v1)
{
    if (getenv(old) != NULL) {
        fprintf(stderr, ATMOS_NAME " : invalid %s : "
            "renamed to %s ('%s' or '%s')\n", old, name, v1, v0);
        exit(1);
    }
    const char* v = env(name, v0);
    if (strcmp(v, v1) == 0) {
        return 1;
    }
    if (strcmp(v, v0) != 0) {
        fprintf(stderr, ATMOS_NAME " : invalid %s : "
            "expected '%s' or '%s'\n", name, v1, v0);
        exit(1);
    }
    return 0;
}

// The numeric types of the build: the program and the runtime agree on
// them.
static int OPT_F32 = 0;
// ATMOS_PIN=off: the runtime and the program without the checks of pin
static int OPT_PIN_OFF = 0;

static void opts_num (void) {
    OPT_F32     = opt2("ATMOS_FLOAT",   "ATMOS_REAL", "64", "32");
    OPT_INT32   = opt2("ATMOS_INTEGER", "ATMOS_INT",  "64", "32");
    OPT_TIME_MS = opt2("ATMOS_CLOCK",   "ATMOS_TIME", "us", "ms");
    {
        const char* p = env("ATMOS_PIN", "on");
        if (!strcmp(p, "off")) {
            OPT_PIN_OFF = 1;
        } else if (!strcmp(p, "unchecked")) {
            OPT_PIN_OFF = 2;
        } else if (strcmp(p, "on") != 0) {
            fprintf(stderr, ATMOS_NAME " : invalid ATMOS_PIN : "
                "expected 'on', 'unchecked', or 'off'\n");
            exit(1);
        }
    }
}

// The runtime of the configuration, or NULL if it cannot be built.
static const char* runtime (const char* home, int asan) {
    char* rel = xfmt("build/libatmos%s%s%s%s%s.a",
        OPT_F32     ? "-f32" : "",
        OPT_INT32   ? "-i32" : "",
        OPT_TIME_MS ? "-ms"  : "",
        (OPT_PIN_OFF==1) ? "-nopin" : (OPT_PIN_OFF==2) ? "-unchecked" : "",
        asan        ? "-asan" : "");
    char* lib = xfmt("%s/%s", home, rel);
    // only the default configuration is built in advance: another one
    // is built when a program first asks for it, with the output of the
    // build in a log, so that its warnings do not reach the output of
    // the program; the log is shown only if the build fails
    if (access(lib, R_OK) != 0) {
        char* log = xfmt("%s.log", lib);
        char* mk = xfmt("mkdir -p %s/build 2>/dev/null && "
            "make -s -C %s %s >%s 2>&1", home, home, rel, log);
        if (system(mk)!=0 || access(lib, R_OK)!=0) {
            // the log, if the directory could be written
            char* show = xfmt("[ -f %s ] && cat %s >&2", log, log);
            if (system(show) != 0) {
                // no log: the directory cannot be written
            }
            fprintf(stderr, ATMOS_NAME " : runtime not found : %s\n"
                "    build it with : make -C %s %s\n", lib, home, rel);
            return NULL;
        }
    }
    return lib;
}

// The C compiler with its flags: those of the environment of a
// dedicated compiler, then those of the user.
static const char* cc_flags (const char* home, int asan) {
    return xfmt("%s %s %s %s %s %s %s %s -w -I%s/rt",
        env("ATMOS_CC", "cc"),
        env("ATMOS_CFLAGS", "-O1"),
        ATMOS_ENV_CFLAGS,
        asan ? "-fsanitize=address -fno-omit-frame-pointer -g" : "",
        OPT_F32     ? "-DATM_FLOAT_32" : "",
        OPT_INT32   ? "-DATM_INTEGER_32"     : "",
        OPT_TIME_MS ? "-DATM_CLOCK_MS"    : "",
        (OPT_PIN_OFF==1) ? "-DATM_PIN_OFF" : (OPT_PIN_OFF==2) ? "-DATM_PIN_UNCHECKED" : "",
        home);
}

// Builds the C source into an executable. Returns 0 on success.
static int build (const char* c, const char* exe) {
    const char* home = home_dir();
    int asan = (getenv("ATMOS_ASAN") != NULL);
    const char* lib = runtime(home, asan);
    if (lib == NULL) {
        return 1;
    }
    char* cmd = xfmt("%s %s %s -lm %s %s -o %s",
        cc_flags(home, asan), c, lib,
        ATMOS_ENV_LDFLAGS,
        env("ATMOS_LDFLAGS", ""),
        exe
    );
    return system(cmd);
}

///////////////////////////////////////////////////////////////////////////////
// A program with modules (`import`): one C unit per module, each
// compiled to an object that is kept in a cache, so that only the units
// whose C changed are compiled again, at the same time.
///////////////////////////////////////////////////////////////////////////////

// Where the C and the objects of the modules are kept: the option
// `--build`, or `build` in the current directory.
static const char* OPT_BUILD = NULL;

// The file has exactly the text.
static int file_is (const char* path, const char* txt) {
    FILE* f = fopen(path, "rb");
    if (f == NULL) {
        return 0;
    }
    size_t n = strlen(txt);
    char* buf = xalloc(n + 2);
    size_t m = fread(buf, 1, n+1, f);
    fclose(f);
    return m==n && !memcmp(buf,txt,n);
}

// Writes the file aside and renames it, so that another build never
// reads half of it.
static int file_put (const char* path, const char* txt) {
    char* tmp = xfmt("%s.%d", path, (int)getpid());
    FILE* f = fopen(tmp, "w");
    if (f == NULL) {
        return 0;
    }
    fputs(txt, f);
    fclose(f);
    return rename(tmp, path) == 0;
}

// The directory of the build, or NULL if it cannot be written (then
// nothing is kept). Each way of compiling (the command of the C
// compiler, and the test blocks, which change the C) has a directory
// of its own inside it, named by a hash, with the command in the file
// `cmd`: a build with other flags does not take the place of this one.
static const char* build_dir (const char* cc) {
    const char* cmd = xfmt("%s%s", cc, OPT_TEST ? " (-t)" : "");
    unsigned int h = 2166136261u;
    for (const char* c=cmd; *c!='\0'; c++) {
        h = (h ^ (unsigned char)*c) * 16777619u;
    }
    const char* dir = xfmt("%s/%08x",
        (OPT_BUILD != NULL) ? OPT_BUILD : "build", h);
    if (system(xfmt("mkdir -p '%s' 2>/dev/null", dir))!=0 ||
        access(dir, W_OK)!=0)
    {
        return NULL;
    }
    const char* file = xfmt("%s/cmd", dir);
    if (!file_is(file, cmd)) {
        file_put(file, cmd);
    }
    return dir;
}

// The name of the files of the program itself in the build: its path,
// as the name of a module, so that two programs of a directory do not
// take the place of each other.
static const char* main_name (const char* inp) {
    char* name = xfmt("%s", inp);
    for (char* c=name; *c!='\0'; c++) {
        if (!isalnum((unsigned char)*c)) {
            *c = '_';
        }
    }
    return name;
}

static int mtime (const char* path, struct timespec* t) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    *t = st.st_mtim;
    return 1;
}

// An object of the build is good if no file that its C included (the
// list that the C compiler left in `dep`, in the format of make) is
// newer than it. The C itself is not checked here: it comes first in
// the list.
static int cache_ok (const char* obj, const char* dep) {
    struct timespec to;
    if (!mtime(obj, &to)) {
        return 0;
    }
    FILE* f = fopen(dep, "r");
    if (f == NULL) {
        return 0;
    }
    int ok = 1;
    int n = 0;
    char w[4096];
    while (ok && fscanf(f, "%4095s", w)==1) {
        // the continuations, the target, and the C itself, which comes
        // first and was in a temporary directory
        if (!strcmp(w,"\\") || n++<2) {
            continue;
        }
        struct timespec td;
        ok = mtime(w, &td) && (td.tv_sec < to.tv_sec ||
            (td.tv_sec==to.tv_sec && td.tv_nsec<=to.tv_nsec));
    }
    fclose(f);
    return ok;
}

// Runs the commands, at most as many at the same time as there are
// processors. Returns 0 if all succeed.
static int run_all (char** cmds, int n) {
    long max = sysconf(_SC_NPROCESSORS_ONLN);
    if (max < 1) {
        max = 1;
    }
    int fail = 0;
    int live = 0;
    int i = 0;
    while (i<n || live>0) {
        if (i<n && live<max && !fail) {
            pid_t pid = fork();
            if (pid == 0) {
                execl("/bin/sh", "sh", "-c", cmds[i], (char*)NULL);
                _exit(127);
            }
            if (pid < 0) {
                fail = 1;
            } else {
                live++;
            }
            i++;
        } else if (live > 0) {
            int status = 0;
            if (wait(&status) > 0) {
                live--;
                fail = fail || !WIFEXITED(status) || WEXITSTATUS(status)!=0;
            }
        } else {
            break;
        }
    }
    return fail;
}

// Builds the C units into an executable. The C and the object of each
// unit stay in the directory of the build, under the name of its
// module: a unit is compiled again only if its C changed, or a file
// that its C includes.
// If the directory cannot be written, all is built in the temporary
// directory `tmp`, and removed.
static int build_units (CUnit* us, int n, const char* inp, const char* tmp,
                        const char* exe)
{
    const char* home = home_dir();
    int asan = (getenv("ATMOS_ASAN") != NULL);
    const char* lib = runtime(home, asan);
    if (lib == NULL) {
        return 1;
    }
    const char* cc = cc_flags(home, asan);
    const char* dir = build_dir(cc);
    int keep = (dir != NULL);
    if (!keep) {
        dir = tmp;
    }

    char** cmds = xalloc(n * sizeof(char*));
    int ncmds = 0;
    char* objs = xfmt("%s", "");
    for (int i=0; i<n; i++) {
        const char* name = (i == 0) ? main_name(inp) : us[i].name;
        char* src = xfmt("%s/%s.c", dir, name);
        char* obj = xfmt("%s/%s.o", dir, name);
        char* dep = xfmt("%s/%s.d", dir, name);
        objs = xfmt("%s %s", objs, obj);
        if (keep && file_is(src,us[i].c) && cache_ok(obj,dep)) {
            continue;
        }
        unlink(obj);
        if (!file_put(src,us[i].c)) {
            fprintf(stderr, ATMOS_NAME " : cannot write '%s'\n", src);
            return 1;
        }
        cmds[ncmds++] = xfmt(
            "%s -MD -MF %s.%d -c %s -o %s.%d && "
            "mv %s.%d %s && mv %s.%d %s",
            cc, dep, (int)getpid(), src, obj, (int)getpid(),
            dep, (int)getpid(), dep, obj, (int)getpid(), obj);
    }
    int ret = run_all(cmds, ncmds);
    if (ret == 0) {
        ret = system(xfmt("%s%s %s -lm %s %s -o %s",
            cc, objs, lib, ATMOS_ENV_LDFLAGS, env("ATMOS_LDFLAGS",""), exe));
    }
    if (!keep) {
        for (int i=0; i<n; i++) {
            const char* name = (i == 0) ? main_name(inp) : us[i].name;
            unlink(xfmt("%s/%s.c", dir, name));
            unlink(xfmt("%s/%s.o", dir, name));
            unlink(xfmt("%s/%s.d", dir, name));
        }
    }
    return ret;
}

static void usage (void) {
    fprintf(stderr,
        "Usage: " ATMOS_NAME " [-c] [-o <out>] [-t] [-e <env>] [--build <dir>] <input> [<args>] ...\n"
        "\n"
        "Options:\n"
        "   -c          Emit C only (to the standard output, or, with -o <dir>,\n"
        "               one file per module in the directory).\n"
        "   -o <out>    Build only.\n"
        "   -t          Enable test blocks.\n"
        "   -e <env>    Include the file of an environment first.\n"
        "   --build <dir>   Keep the C and the objects of the modules in the\n"
        "               directory (default: build).\n"
    );
    if (ATMOS_ENV != NULL) {
        fprintf(stderr, "\nEnvironment: %s\n", (const char*)ATMOS_ENV);
    }
    exit(1);
}

int main (int argc, char** argv) {
    int lex  = 0;
    int ast  = 0;
    int only = 0;
    const char* exe = NULL;
    const char* inp = NULL;
    OPT_ENV = ATMOS_ENV;
    opts_num();
    int i = 1;
    for (; i<argc && inp==NULL; i++) {
        if (!strcmp(argv[i], "--lex")) {
            lex = 1;
        } else if (!strcmp(argv[i], "--ast")) {
            ast = 1;
        } else if (!strcmp(argv[i], "-c")) {
            only = 1;
        } else if (!strcmp(argv[i], "-t")) {
            OPT_TEST = 1;
        } else if (!strcmp(argv[i], "--build") && i+1<argc) {
            OPT_BUILD = argv[++i];
        } else if (!strncmp(argv[i], "--build=", 8) && argv[i][8]!='\0') {
            OPT_BUILD = argv[i] + 8;
        } else if (!strcmp(argv[i], "-o") && i+1<argc) {
            exe = argv[++i];
        } else if (!strcmp(argv[i], "-e") && i+1<argc) {
            OPT_ENV = argv[++i];
        } else if (argv[i][0] == '-') {
            usage();
        } else {
            inp = argv[i];
        }
    }
    if (inp == NULL) {
        usage();
    }

    if (OPT_ENV!=NULL && access(OPT_ENV,R_OK)!=0) {
        fprintf(stderr, ATMOS_NAME " : cannot read the environment '%s'\n",
            OPT_ENV);
        return 1;
    }

    size_t len;
    char* src = slurp(inp, &len);
    lexer_init(inp, src, len);

    if (lex) {
        dump_lex();
        return 0;
    }

    Expr* prog = parser_main();

    if (ast) {
        for (int j=0; j<prog->u.block.es.n; j++) {
            ast_dump(prog->u.block.es.v[j]);
            printf("\n");
        }
        return 0;
    }

    int nglobals;
    resolver_main(prog, &nglobals);

    CUnit* units;
    int nunits = coder_main(prog, &units);
    char* c = units[0].c;

    if (only) {
        // the C only: to the standard output, or, with `-o <dir>`, one
        // file per unit in the directory, whose paths are printed (what
        // another build needs to compile them)
        if (exe == NULL) {
            if (nunits > 1) {
                fprintf(stderr, ATMOS_NAME " : a program with modules has "
                    "one C file per module : give a directory with -o\n");
                return 1;
            }
            fputs(c, stdout);
            return 0;
        }
        if (system(xfmt("mkdir -p '%s'", exe)) != 0) {
            fprintf(stderr, ATMOS_NAME " : cannot create '%s'\n", exe);
            return 1;
        }
        for (int j=0; j<nunits; j++) {
            char* path = xfmt("%s/%s.c", exe, units[j].name);
            FILE* f = fopen(path, "w");
            if (f == NULL) {
                fprintf(stderr, ATMOS_NAME " : cannot write '%s'\n", path);
                return 1;
            }
            fputs(units[j].c, f);
            fclose(f);
            printf("%s\n", path);
        }
        return 0;
    }

    // builds in a temporary directory, removed at the end: in TMPDIR, as
    // is the convention, or /tmp (a host may mount /tmp with no right to
    // execute: TMPDIR then points elsewhere)
    const char* tmp = getenv("TMPDIR");
    char* dir = xfmt("%s/atmos-XXXXXX", (tmp!=NULL && tmp[0]!='\0') ? tmp : "/tmp");
    if (mkdtemp(dir) == NULL) {
        fprintf(stderr, ATMOS_NAME " : cannot create temporary directory : %s : %s\n",
            dir, strerror(errno));
        return 1;
    }
    char* cfile = xfmt("%s/main.c", dir);
    char* xfile = (exe != NULL) ? (char*)exe : xfmt("%s/main", dir);

    int ret;
    if (nunits > 1) {
        ret = build_units(units, nunits, inp, dir, xfile);
    } else {
        FILE* f = fopen(cfile, "w");
        fputs(c, f);
        fclose(f);
        ret = build(cfile, xfile);
    }
    if (ret != 0) {
        fprintf(stderr, ATMOS_NAME " : C compilation failed\n");
        ret = 1;
    } else if (exe == NULL) {
        // runs the program with the remaining arguments
        pid_t pid = fork();
        if (pid == 0) {
            argv[i-1] = xfile;
            execv(xfile, &argv[i-1]);
            // as a shell: 126 for a file that cannot run (a directory
            // with no right to execute, as a /tmp mounted noexec)
            fprintf(stderr, ATMOS_NAME " : cannot run %s : %s\n",
                xfile, strerror(errno));
            _exit(errno==ENOENT ? 127 : 126);
        }
        int status = 0;
        waitpid(pid, &status, 0);
        ret = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
        unlink(xfile);
    }

    unlink(cfile);
    rmdir(dir);
    return ret;
}
