// Atmos compiler: command line.
//
//   atmos f.atm [args]              compiles, builds, and runs
//   atmos --emit-c f.atm            emits C only (to stdout)
//   atmos --out=out f.atm           builds only
//   atmos --test f.atm              enables `test` blocks
//   atmos --env=env.atm f.atm       includes the file of an environment
//                                   before the program
//   atmos --help, atmos --version
//
// The value of an option follows it, after `=` or as the next argument.
// `--` ends the options. There are no short forms.
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
#ifndef ATMOS_VERSION
#define ATMOS_VERSION "unknown"
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

// Builds the C source into an executable. Returns 0 on success.
static int build (const char* c, const char* exe) {
    const char* home = home_dir();
    int asan = (getenv("ATMOS_ASAN") != NULL);
    // the runtime of the configuration
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
            return 1;
        }
    }
    // the flags of the environment of a dedicated compiler, then those
    // of the user
    char* cmd = xfmt(
        "%s %s %s %s %s %s %s %s -w -I%s/rt %s %s -lm %s %s -o %s",
        env("ATMOS_CC", "cc"),
        env("ATMOS_CFLAGS", "-O1"),
        ATMOS_ENV_CFLAGS,
        asan ? "-fsanitize=address -fno-omit-frame-pointer -g" : "",
        OPT_F32     ? "-DATM_FLOAT_32" : "",
        OPT_INT32   ? "-DATM_INTEGER_32"     : "",
        OPT_TIME_MS ? "-DATM_CLOCK_MS"    : "",
        (OPT_PIN_OFF==1) ? "-DATM_PIN_OFF" : (OPT_PIN_OFF==2) ? "-DATM_PIN_UNCHECKED" : "",
        home, c, lib,
        ATMOS_ENV_LDFLAGS,
        env("ATMOS_LDFLAGS", ""),
        exe
    );
    return system(cmd);
}

static void usage (FILE* out) {
    fprintf(out,
        "Usage: " ATMOS_NAME " [<options>] [--] <input> [<args>] ...\n"
        "\n"
        "Options:\n"
        "   --emit-c        Emit C only (to the standard output).\n"
        "   --out=<out>     Build only, into <out>.\n"
        "   --test          Enable test blocks.\n"
        "   --env=<env>     Include the file of an environment first.\n"
        "   --lex, --ast    Print the tokens, or the tree, of the input.\n"
        "   --help          Print this message.\n"
        "   --version       Print the version.\n"
        "\n"
        "A value may also follow its option as the next argument.\n"
    );
    if (ATMOS_ENV != NULL) {
        fprintf(out, "\nEnvironment: %s\n", (const char*)ATMOS_ENV);
    }
}

static void fail_usage (const char* msg, const char* arg) {
    fprintf(stderr, ATMOS_NAME " : %s '%s'\n\n", msg, arg);
    usage(stderr);
    exit(1);
}

// Matches `argv[*i]` with an option that takes a value, `--name=v` or
// `--name v`. Returns the value, or NULL if the argument is not this
// option.
static const char* opt_val (int argc, char** argv, int* i,
                            const char* name) {
    const char* a = argv[*i];
    size_t n = strlen(name);
    if (!strncmp(a, name, n) && a[n]=='=') {
        if (a[n+1] == '\0') {
            fail_usage("missing the value of", name);
        }
        return &a[n+1];
    }
    if (!strcmp(a, name)) {
        if (*i+1 >= argc) {
            fail_usage("missing the value of", a);
        }
        return argv[++*i];
    }
    return NULL;
}

int main (int argc, char** argv) {
    int lex  = 0;
    int ast  = 0;
    int only = 0;
    const char* exe = NULL;
    const char* inp = NULL;
    const char* v;
    OPT_ENV = ATMOS_ENV;
    int i = 1;
    for (; i<argc && inp==NULL; i++) {
        const char* a = argv[i];
        if (!strcmp(a, "--help")) {
            usage(stdout);
            return 0;
        } else if (!strcmp(a, "--version")) {
            printf(ATMOS_NAME " " ATMOS_VERSION "\n");
            return 0;
        } else if (!strcmp(a, "--")) {
            if (i+1 >= argc) {
                fail_usage("missing the input after", a);
            }
            inp = argv[++i];
        } else if (!strcmp(a, "--lex")) {
            lex = 1;
        } else if (!strcmp(a, "--ast")) {
            ast = 1;
        } else if (!strcmp(a, "--emit-c")) {
            only = 1;
        } else if (!strcmp(a, "--test")) {
            OPT_TEST = 1;
        } else if ((v = opt_val(argc, argv, &i, "--out")) != NULL) {
            exe = v;
        } else if ((v = opt_val(argc, argv, &i, "--env")) != NULL) {
            OPT_ENV = v;
        } else if (a[0] == '-') {
            fail_usage("unknown option", a);
        } else {
            inp = a;
        }
    }
    if (inp == NULL) {
        usage(stderr);
        exit(1);
    }
    opts_num();

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
    Var** globals = resolver_main(prog, &nglobals);

    char* c = coder_main(prog, globals, nglobals);

    if (only) {
        fputs(c, stdout);
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

    FILE* f = fopen(cfile, "w");
    fputs(c, f);
    fclose(f);

    int ret = build(cfile, xfile);
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
