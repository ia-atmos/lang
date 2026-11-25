# TODO of the session `atmos-cc`

The time of a build with `include`: separate compilation, a
`--verbose`, C flags on the command line. Material: `261123-cc.md`.
Prototypes go to the branch `cc` (cut from `origin/main` at `5d1b6a7`).

Last update: 2026-11-25 (LTO).

## Index of items

| # | Item | State |
|---|------|-------|
| #1 | Separate compilation: split the generated C into units compiled in parallel (first step), cache later | Asked 2026-11-23; recommended: split into N units in parallel. Prototype by the session, to be confirmed (rule `atmos-lang#95`, 2026-11-25): the split done by gcc itself, `-flto=auto`, for a C over 256 KB (section "LTO") |

## Measurements (2026-11-23, this container, 4 cores, gcc 13.3)

`sdl-pingus` `main.atm`, generated C 91 thousand lines, 533 generated
functions (86.5 thousand lines), the rest native code and declarations.
Atmos to C: 0.05 s.

Whole file, one `cc -c`:

| Level | Time | Peak memory (`-ftime-report`) |
|---|---|---|
| `-O0` | 6.1 s | 362 MB |
| `-O1` | 21.9 s | 737 MB |
| `-O2` | 40.0 s | 1004 MB |

Where `-O1` goes (`-ftime-report`, wall): "phase opt and generate"
19.5 s, of which "callgraph functions expansion" 16.9 s (the per
function passes, no single pass dominant); parsing 2.0 s. At `-O2` the
same shape (33.8 of 37.2 s in the per function passes).

Split by a script (`split.py`, in the scratchpad): every unit keeps the
whole preamble (headers, native code, globals, prototypes) and a
balanced share of the generated functions. Not linked; timing only.

| Level | Whole | 4 units, parallel | 8 units, parallel | 8 units, serial sum |
|---|---|---|---|---|
| `-O0` | 6.1 s | 1.7 s | 2.2 s | |
| `-O1` | 21.9 s | 5.1 s | 3.8 s | 8.1 s |
| `-O2` | 40.0 s | 9.7 s | 6.0 s | |

The serial sum (8.1 s) is well under the whole (21.9 s): the cost grows
faster than the size of the unit, mostly through inlining across
functions (`-O1 -fno-inline` on the whole file: 10.5 s;
`-fno-inline-functions-called-once`: 19.9 s). So a split gains twice:
less work per unit and parallelism. To check: the speed of the
resulting program without inlining across units.

Largest functions: `atm_T0` 4465 lines, `atm_T226` 3053, `atm_T145`
949; `atm_consts_init` 698.

Notes for a real split: the generated functions are `static` and
already have prototypes (532 at the top); native code has `static`
globals (`SDL_WIN`, ...) and functions used by the generated code, so
the native part cannot simply be repeated in every unit; generated
names (`atm_T226`) are numbered over the whole program, so a cache per
unit would miss after any change before it unless names become stable.

## From other sessions (data, to check before use)

- `atmos-cli`, 2026-11-23: its #1 is answered (long options only:
  "Não quero -x, quero --xyz"); its #2 (names of today's options,
  recommended `--emit-c`, `--out`, `--test`, `--env`, value after a
  space or `=`) waits for the author. Proposed split, to confirm with
  the author: here `--verbose`, the `--cc-`/`--ld-` flags and how they
  combine with `ATMOS_CFLAGS`/`ATMOS_LDFLAGS`, any cache, separate
  compilation; there the parser of the command line, the names of the
  existing options, `--help`/`--version`, install, environments. It
  implements the parser on branch `cli`; a spelling decided here is to
  be sent to it.
- `atmos`, 2026-11-24: a new managing session (`atmos-lang#119`,
  handoff `261124-atmos.md`). Coordination, status and questions go to
  `atmos` (session_011eECohhJbc4GMpAj3yyeEo); measurements on the ports
  and merges stay with `atmos-lang`. Corrected by `atmos` on
  2026-11-25 (confirmed by the author): item numbers stay local to each
  session (rule #71), seen from outside as `atmos-cc#N`; only `atmos`
  and `atmos-lang` share the sequence of `ISSUES.md`. A pause holds until next week's credits:
  no new work, only what arrives.

## LTO: the split done by the C compiler (2026-11-25, by the session, to be confirmed)

`gcc -flto=auto` on one file: the front end runs once (3.0 s on the
Pingus C), then gcc splits the functions into partitions and optimizes
them in parallel, with no change to the generated C (statics, native
code and all). Same container, 4 cores, gcc 13.3, compile and link:

| Pingus C, compile + link | `-O1` | `-O2` |
|---|---|---|
| whole file | 20.6 s | 29.5 s |
| `-flto=4` | 7.6 s | 11.0 s |
| `-flto=4 --param lto-partitions=8` | 6.9 s | |
| `atmos -o`, through the compiler of the branch | | 11.5 s (30.3 s with `-fno-lto`) |

| `sdl-pingus/tst/run.sh` (40 tests) | Time | Result |
|---|---|---|
| `main` | 121 s | all pass |
| branch `cc` | 68 s | all pass |
| branch `cc`, `ASAN=1` | 121 s | all pass |

- clang accepts `-flto=auto` but gains nothing on one file (20.5 s to
  19.2 s at `-O1`): its parallel form (thin LTO) works per file.
- A small program pays about 40 ms more (63 ms to 100 ms), hence the
  threshold: LTO only for a generated C over 256 KB.
- On the branch: `src/main.c` adds `-flto=auto` before `ATMOS_CFLAGS`
  for a C over 256 KB, so `-fno-lto` there turns it off. No new option
  or variable.
- Found with it: `env-sdl` tells its boxes by the address of a string
  literal (`static const char* SDL_KIND_IMG = "sdl.image"`; also
  `rt/lio.c`, `FILE_KIND`). C does not promise one address per literal:
  under `-flto` with `-fsanitize=address`, each partition had its own
  copy, `sdl.size` returned 0 and `bomber-test` threw "division by
  zero". On the branch, `a_nat_is` (`rt/atmos.h`) compares the address
  and then the text. A fix in `env-sdl` (a `char[]` has one address)
  would also do; to tell `atmos-env-sdl` if the author takes this.
- `make test` (with and without `ATMOS_ASAN=1`) and `make test-cfgs`
  pass; no test of `lang` reaches 256 KB, so the LTO path is covered
  only by the Pingus runs above.
- Not measured yet: `doom`, `sdl-flare`, `sdl-snkrx`; the speed of the
  program built with LTO.
