// Atmos compiler: command line.
//
//   atmos f.atm [args]     compiles, builds, and runs
//   atmos -c f.atm         emits C only (to stdout)
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

// A generated C larger than this (in bytes) is compiled with the link
// time optimization of the C compiler in parallel (`-flto=auto`): gcc
// then splits the one file in partitions compiled at the same time
// (sdl-pingus, 3 MB, -O1: 20.6 s to 7.6 s on 4 cores). Below it, the
// overhead of LTO (about 40 ms) is not worth it. The flag comes before
// ATMOS_CFLAGS, so that `-fno-lto` there turns it off.
#define LTO_MIN (256*1024)

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
static const char* cc_flags (const char* home, int asan, int lto) {
    return xfmt("%s %s %s %s %s %s %s %s %s -w -I%s/rt",
        env("ATMOS_CC", "cc"),
        lto ? "-flto=auto" : "",
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
static int build (const char* c, size_t clen, const char* exe) {
    const char* home = home_dir();
    int asan = (getenv("ATMOS_ASAN") != NULL);
    const char* lib = runtime(home, asan);
    if (lib == NULL) {
        return 1;
    }
    char* cmd = xfmt("%s %s %s -lm %s %s -o %s",
        cc_flags(home, asan, clen > LTO_MIN), c, lib,
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

// Where the objects are kept: ATMOS_CACHE, or the cache of the user.
// NULL if there is nowhere to keep them.
static const char* cache_dir (void) {
    const char* v = getenv("ATMOS_CACHE");
    char* dir = NULL;
    if (v!=NULL && v[0]!='\0') {
        dir = xfmt("%s", v);
    } else if ((v=getenv("XDG_CACHE_HOME"))!=NULL && v[0]!='\0') {
        dir = xfmt("%s/atmos", v);
    } else if ((v=getenv("HOME"))!=NULL && v[0]!='\0') {
        dir = xfmt("%s/.cache/atmos", v);
    } else {
        return NULL;
    }
    if (system(xfmt("mkdir -p '%s' 2>/dev/null", dir))!=0 ||
        access(dir, W_OK)!=0)
    {
        return NULL;
    }
    return dir;
}

static unsigned long long hash (unsigned long long h, const char* s) {
    for (; *s!='\0'; s++) {
        h = (h ^ (unsigned char)*s) * 1099511628211ULL;
    }
    return (h ^ 0xff) * 1099511628211ULL;
}

static int mtime (const char* path, struct timespec* t) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    *t = st.st_mtim;
    return 1;
}

// An object of the cache is good if no file that its C included (the
// list that the C compiler left in `dep`, in the format of make) is
// newer than it. The C itself is not checked: its text is in the name
// of the object.
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

// Builds the C units into an executable, in the temporary directory
// `dir`. Returns 0 on success.
static int build_units (CUnit* us, int n, const char* dir, const char* exe) {
    const char* home = home_dir();
    int asan = (getenv("ATMOS_ASAN") != NULL);
    const char* lib = runtime(home, asan);
    if (lib == NULL) {
        return 1;
    }
    const char* cc = cc_flags(home, asan, 0);
    const char* cache = cache_dir();

    char** cmds = xalloc(n * sizeof(char*));
    char** tmps = xalloc(n * sizeof(char*));
    int ncmds = 0;
    char* objs = xfmt("%s", "");
    for (int i=0; i<n; i++) {
        char* src = xfmt("%s/%s.c", dir, us[i].name);
        FILE* f = fopen(src, "w");
        if (f == NULL) {
            return 1;
        }
        fputs(us[i].c, f);
        fclose(f);
        tmps[i] = src;

        // the name of the object: the text of the C, and how it is
        // compiled
        char* obj;
        char* dep;
        if (cache == NULL) {
            obj = xfmt("%s/%s.o", dir, us[i].name);
            dep = xfmt("%s/%s.d", dir, us[i].name);
        } else {
            unsigned long long h = hash(14695981039346656037ULL, cc);
            h = hash(h, us[i].c);
            obj = xfmt("%s/%016llx.o", cache, h);
            dep = xfmt("%s/%016llx.d", cache, h);
        }
        objs = xfmt("%s %s", objs, obj);
        if (cache!=NULL && cache_ok(obj,dep)) {
            continue;
        }
        // compiled aside and then renamed, so that another build
        // never reads half of an object
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
    for (int i=0; i<n; i++) {
        unlink(tmps[i]);
        if (cache == NULL) {
            unlink(xfmt("%s/%s.o", dir, us[i].name));
            unlink(xfmt("%s/%s.d", dir, us[i].name));
        }
    }
    return ret;
}

static void usage (void) {
    fprintf(stderr,
        "Usage: " ATMOS_NAME " [-c] [-o <out>] [-t] [-e <env>] <input> [<args>] ...\n"
        "\n"
        "Options:\n"
        "   -c          Emit C only (to the standard output).\n"
        "   -o <out>    Build only.\n"
        "   -t          Enable test blocks.\n"
        "   -e <env>    Include the file of an environment first.\n"
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
        for (int j=0; j<nunits; j++) {
            if (nunits > 1) {
                printf("//// atmos unit : %s.c\n", units[j].name);
            }
            fputs(units[j].c, stdout);
        }
        return 0;
    }

    // builds in a temporary directory, removed at the end
    char dir[] = "/tmp/atmos-XXXXXX";
    if (mkdtemp(dir) == NULL) {
        fprintf(stderr, ATMOS_NAME " : cannot create temporary directory\n");
        return 1;
    }
    char* cfile = xfmt("%s/main.c", dir);
    char* xfile = (exe != NULL) ? (char*)exe : xfmt("%s/main", dir);

    int ret;
    if (nunits > 1) {
        ret = build_units(units, nunits, dir, xfile);
    } else {
        FILE* f = fopen(cfile, "w");
        fputs(c, f);
        fclose(f);
        ret = build(cfile, strlen(c), xfile);
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
            _exit(127);
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
