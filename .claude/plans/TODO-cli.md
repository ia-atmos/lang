# TODO of the session `atmos-cli`

The command line of Atmos: the program `atmos`, its options, how it is
configured, built, installed, and told which environment to use. The
material is `261120-cli.md`. The implementation goes to the branch
`cli`; this file and the plans go straight to `main`.

Last update: 2026-11-20.

## Index of items

| # | Item | State |
|---|------|-------|
| #1 | What the author has in mind for the command line | Asked on 2026-11-20; waits for him |

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
