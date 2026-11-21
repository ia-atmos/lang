# TODO of the session `atmos-cli`

The command line of Atmos: the program `atmos`, its options, how it is
configured, built, installed, and told which environment to use. The
material is `261120-cli.md`. The implementation goes to the branch
`cli`; this file and the plans go straight to `main`.

Last update: 2026-11-21.

## Index of items

| # | Item | State |
|---|------|-------|
| #1 | What the author has in mind for the command line | Answered 2026-11-21: "Não quero -x, quero --xyz" (long options instead of short ones) |
| #2 | The names of the long options | Asked 2026-11-21; recommended: `--emit-c`, `--out`, `--test`, `--env`, with `--help`, `--version`, `--lex`, `--ast`, `--`; the value after a space or `=` |
| #4 | Why `--emit-c` and not `--c`; who uses `-c` today | Answered 2026-11-21: two ports, `doom/Makefile` (lines 63, 79) and `env-esp32/esp32/main/CMakeLists.txt` (line 62), both to build the C themselves; none of the other 14 repositories of `ia-atmos` nor the tests of `lang` |
| #3 | What becomes of `-c`, `-o`, `-t`, `-e` | Not asked yet (after #2): the ports call them |

## Observed on 2026-11-20 (`main` at `5123f9a`)

Each option and each variable was run once. Facts, not items: one
becomes an item only if the author takes it up.

- `--help` and `--version` are unknown options: the usage, on the
  standard error, with status 1. So does `--`, `-` (the standard
  input), and `-o` with nothing after it, with no message of its own.
- `-c -o out f.atm` prints the C and ignores `-o` silently.
- Options after the input go to the program (`atmos f.atm -t` passes
  `-t`), as documented.
- A run of a one-line program takes 0.1 s, all of it the C compiler
  (`-c` alone takes 1 ms). The first run with `ATMOS_REAL=float` took
  5 s: it built `libatmos-f32.a` in the checkout.
- `ATMOS_HOME` pointing nowhere prints the error of `make`, then
  `runtime not found`, then `C compilation failed`.
- A copy of the executable alone, somewhere else, still works: it falls
  back to the checkout where it was built.
- An error in a native literal points at the `.atm` file; the lines of
  the generated C that follow point at `/tmp/atmos-XXXXXX/main.c`,
  which is removed.
- The ports: `sdl-pingus/run.sh` looks for the compiler in four sibling
  directories and sets `ATMOS_CFLAGS` and `ATMOS_LDFLAGS`; `env-sdl`
  and `env-pico` need `make ATMOS=/path/to/lang` to build their
  dedicated compiler, and their tests call `../atmos-sdl`,
  `../atmos-pico -t`.

## From other sessions (data, to check before use)

- `atmos-lang`, 2026-11-20: the branch `float` renames three variables
  (`ATMOS_REAL=double|float` to `ATMOS_FLOAT=64|32`, `ATMOS_INT` to
  `ATMOS_INTEGER`, `ATMOS_TIME` to `ATMOS_CLOCK`; a former name is an
  error) and adds `ENV_NUM` to `make env` (what an environment
  supports). Seen in `origin/float` (`src/main.c`, `Makefile`). On
  `main` since `d19d40f` (2026-11-21), without `ENV_NUM`: an
  environment refuses an option with `#error`. Anything proposed here about the variables starts from
  those names, and `cli` will be rebased over it once merged.
- `atmos-envs`, 2026-11-20, on the selection of an environment (its
  recommendations, none decided by the author): for `atmos-envs#4`,
  keep both mechanisms and let them take a list, in order
  (`make env ENV="a.atm b.atm"`, `-e a.atm -e b.atm`), the first being
  the primary; for `atmos-envs#11` (`atmos-uv prog.atm` with
  `include "iup.atm"` makes uv the primary whatever the program
  wants), settle the modes when the main chunk ends, with
  `mode.prefer`, or, from the command line, an option that names the
  primary (`-p iup`), or register the environment of the dedicated
  compiler after those of the program. Dedicated compilers in use:
  `atmos-pico`, `atmos-sdl`, `atmos-ray`, `atmos-uv`.
