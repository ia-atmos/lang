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

static void opts_num (void) {
    OPT_F32     = opt2("ATMOS_FLOAT",   "ATMOS_REAL", "64", "32");
    OPT_INT32   = opt2("ATMOS_INTEGER", "ATMOS_INT",  "64", "32");
    OPT_TIME_MS = opt2("ATMOS_CLOCK",   "ATMOS_TIME", "us", "ms");
}

// Builds the C source into an executable. Returns 0 on success.
static int build (const char* c, const char* exe) {
    const char* home = home_dir();
    int asan = (getenv("ATMOS_ASAN") != NULL);
    // the runtime of the configuration
    char* rel = xfmt("build/libatmos%s%s%s%s.a",
        OPT_F32     ? "-f32" : "",
        OPT_INT32   ? "-i32" : "",
        OPT_TIME_MS ? "-ms"  : "",
        asan        ? "-asan" : "");
    char* lib = xfmt("%s/%s", home, rel);
    // only the default configuration is built in advance: another one
    // is built when a program first asks for it
    if (access(lib, R_OK) != 0) {
        char* mk = xfmt("make -s -C %s %s >&2", home, rel);
        if (system(mk)!=0 || access(lib, R_OK)!=0) {
            fprintf(stderr, ATMOS_NAME " : runtime not found : %s\n"
                "    build it with : make -C %s %s\n", lib, home, rel);
            return 1;
        }
    }
    // the flags of the environment of a dedicated compiler, then those
    // of the user
    char* cmd = xfmt(
        "%s %s %s %s %s %s %s -w -I%s/rt %s %s -lm %s %s -o %s",
        env("ATMOS_CC", "cc"),
        env("ATMOS_CFLAGS", "-O1"),
        ATMOS_ENV_CFLAGS,
        asan ? "-fsanitize=address -fno-omit-frame-pointer -g" : "",
        OPT_F32     ? "-DATM_FLOAT_32" : "",
        OPT_INT32   ? "-DATM_INTEGER_32"     : "",
        OPT_TIME_MS ? "-DATM_CLOCK_MS"    : "",
        home, c, lib,
        ATMOS_ENV_LDFLAGS,
        env("ATMOS_LDFLAGS", ""),
        exe
    );
    return system(cmd);
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
