# TODO of the session `atmos-cc`

The time of a build with `include`: separate compilation, a
`--verbose`, C flags on the command line. Material: `261123-cc.md`.
Prototypes go to the branch `cc` (cut from `origin/main` at `5d1b6a7`).

Last update: 2026-11-27.

## Index of items

| # | Item | State |
|---|------|-------|
| #1 | Separate compilation: first path | Superseded by `#2` (import). The LTO for a large C (`-flto=auto`) was removed, decided 2026-11-27 ("Sim", after "Acho que não. Vai fazer diferença após o import?"): it does not apply to modules, and at link it would undo their cache. The change to `a_nat_is` stays |
| #2 | `import`: each file compiled apart (the author, 2026-11-26) | Approved 2026-11-26 ("#2 sim"). Prototype on `cc`: Pingus converted in a copy passes 40/40 (section "Import: the prototype") |
| #3 | Which top-level names are local to a file (the author: "usar val para locais") | Answered 2026-11-26 ("Tudo certo"): every top-level name is seen by who imports, as with `include`; no rule for `val` |
| #4 | Native code under `import`: the names of the two preludes | Decided 2026-11-26: `:prelude.h` (every unit) and `:prelude.c` (the unit of its file), his suggestion ("Poderia ser prelude.c e .h?"). On `cc` |
| #4a | `:prelude` with no suffix | Decided 2026-11-26 ("Sempre"): always an error (`invalid native : expected ':prelude.h' or ':prelude.c'`). On `cc`; every environment and port must migrate before a merge |
| #5 | `-c` with modules | Decided 2026-11-26 ("Recomendado"): one C file per module in a directory. On `cc` as `-c -o <dir>` (the paths are printed); `-c` alone refuses a program with modules; with no `import`, the standard output as before. The final spelling (`--emit-c=DIR`) is of `atmos-cli` |
| #6 | Where the C and the objects of the modules are kept | Decided 2026-11-26 ("#6 build/ então"): `build/` in the current directory, or `--build <dir>`; the files stay, under the names of the modules, one directory per command inside it. He first asked for the files directly in the current directory; the costs were put to him. On `cc` |
| #7 | Merge `cc` now, or after `--verbose` and the C flags | Decided 2026-11-27 ("Merge agora"), with: "Mas nao é pra avisar a todos da mudanca. Deixa eles precisarem de uma versao nova". Sent to `atmos-lang` for the merge, and to `atmos` not to relay; the ports are not told |

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
- `env-sdl`, 2026-11-25: fixed the kinds at the source (`ia-atmos/env-sdl`
  main `fdab1ee`): `SDL_KIND_IMG/FNT/SND` are now `static const char X[]`.
  Its 27 tests pass with `-O2 -flto` and ASAN; `bomber-test` with LTO and
  ASAN passes over the new `sdl.atm`. The change to `a_nat_is` on `cc` is
  no longer needed by env-sdl; it still protects other environments
  (and `rt/lio.c`, `FILE_KIND`).

## Import: each file compiled apart (2026-11-26)

**Autor** (2026-11-26):
> Acredito que queremos algo mais sofisticado. Um sistema de import que
> compile cada modulo em separado. É possível?
> Isso tras uma questao de design que sao os namespaces, mas estaria
> satisfeito em assumir que nomes externos nao colidem. Poderiamos usar
> val para locais para dimunir.
> 1. Possivel um import extremamente simples?
> 2. As tasks e vars e funcs e etc globais de Atmos sao possiveis de
> compilar e acessar em separado? Como se compara com os simbolos de C?

Read in the compiler (`main` at `d6b0c62`):

- A top-level variable is `static AVal atm_G<id>_<name>` (`var_raw`),
  accessed directly, never captured or boxed (`resolver.c`, capture
  returns early for a global). A `func` or `task` is not a C symbol the
  program calls: it is a value built when its declaration runs, kept in
  such a variable, and called through `a_call`.
- Strings and tags are interned at run time (`rt/str.c`), so two units
  agree on them with no shared table; `atm_K` can be per unit.
- What the resolver needs of a name of another file: that it exists,
  its kind (`val`, `var`, `pin`), and, for a `spawn`, the body of the
  task (does it await: "task aborted at end of block").
- Names that depend on the whole program today: `atm_G<id>`,
  `atm_T<n>`, `atm_F<n>`, `atm_K[i]`, the indexes of `atm_files`. A
  change in one file renumbers the others.
- The statements of an included file are part of the main task: one C
  function per trail (`atm_T0` has 4465 lines in Pingus), one frame.
- A `:prelude` goes to the top of the one C file: `env-sdl` keeps C
  state there (`static SDL_Window* SDL_WIN`) and `static` functions
  that the natives of other files call (15 files of Pingus have
  natives).
- Top-level statements of the files of Pingus: 98 `task`, 77 `func`,
  33 `pin`, 11 `val`, 1 `var`, and a few that run (`spawn {...}` in
  `engine/engine.atm` and `engine/sdl.atm`, a `loop` in
  `game/pingu/actions.atm`).

Proposed (nothing decided):

- `import "x.atm"`: the compiler still reads and resolves every file
  (0.05 s for Pingus), so there is no interface file and every check
  stays as with `include`. Only the output changes: one C unit per
  file, an object kept in a cache by the text of its C and the flags,
  the changed ones compiled in parallel, then linked.
- A top-level name becomes `AVal atm_G_<name>`, with no number: seen by
  the other units as `extern`. Two files with the same top-level name
  fail at link (the author accepts no namespaces).
- The body of an imported file is a task of its own, started at the
  `import` and kept until the program ends, so its `spawn`, `pin` and
  `defer` live as today. Difference from `include`: an `await` at the
  top level of the file does not hold the importer.
- Names per file (`atm_T_<file>_<n>`), `atm_K` and `atm_files` per
  unit, so that a change in one file leaves the C of the others equal.
- The open cost is the native code: a `:prelude` is seen only by its
  unit. An environment would give its C declarations to the others
  (a part of the prelude that every importer repeats, with no state)
  and keep its state in its own unit.
- Expected, from the split measured on 2026-11-23 (not linked): a
  cold build about 4 s at `-O1` on 4 cores against 21.9 s; after a
  change in one file, only that unit. Not measured with a real import.

## Import: the prototype (2026-11-26, branch `cc`)

What the branch does:

- `import "x.atm"` (a keyword, top level only, relative to the file, a
  file once). The parser keeps its statements as one node; the resolver
  resolves them in the scope of the program (names seen as with
  `include`, hoisting included), as a task with no parameters.
- One C unit per module (`coder.c`, `COut`): its functions (numbered
  per module), its constants, its top-level variables as
  `AVal atm_G_<module>_<name>`, declared `extern` in the units that
  use them. So two modules may have the same top-level name: nothing
  collides at link, and the latest declaration is the one seen, as
  with `include`. A program with no `import` gets the same C as before.
- The body of a module ends with `await(false)` and is started by
  `a_spawn_mod` (`rt/task.c`): pinned to the block of the import, as
  deep as the main chunk (else a task of the program could not take
  what a module pins: "invalid escape"), and left out of the abort of
  the children of a trail, so that it ends in its turn as the block
  unwinds (else a `defer` of the program found the tables of a module
  finalized).
- `main.c`: each unit is compiled to an object named by the hash of
  its C and of the command, in `ATMOS_CACHE`, or `$XDG_CACHE_HOME/atmos`,
  or `~/.cache/atmos`; an object is good while no file its C included
  (the `.d` of the C compiler) is newer; the stale ones are compiled at
  the same time, then all are linked. No LTO in this path.
- Native code: a `:prelude` goes to every unit, in order, as a header;
  a `:prelude.once` only to the unit of its file (provisional, `#4`).
- Tests `tst/cases/import-01..05`, `import-err-01..03`. `make test`
  (plain and `ATMOS_ASAN=1`) and `make test-cfgs` pass.

Pingus, converted in a copy (not pushed): every `include` to `import`
(55 units); in `env-sdl/sdl.atm` and `engine/pingus.h`, the 33 C
variables `static T X = v;` became `extern T X;` with `T X = v;` in a
`:prelude.once`, and `int main` moved there. Same container, 4 cores:

| Pingus | `include`, `main` | `include`, LTO (`#1`) | `import` |
|---|---|---|---|
| build `-O1`, nothing in the cache | 20.6 s | 7.6 s | 11.4 s |
| build again, no change | 20.6 s | 7.6 s | 0.2 s |
| after a change in `engine/engine.atm` | 20.6 s | 7.6 s | 2.2 s |
| after `touch engine/pingus.h` | 20.6 s | 7.6 s | 11.4 s |
| `tst/run.sh` (40 tests, `-O2`) | 121 s | 68 to 78 s | 78 s, then 44 s |
| the same with `ASAN=1` | not measured | 121 to 141 s | 121 s, then 92 s |

All 40 tests pass, also with ASAN. The first build is slower than LTO
because each of the 55 units reads the SDL headers and the preludes
again (the C grows from 91 to 149 thousand lines).

Open, seen in the prototype:

- A function `static` of a shared prelude that keeps a `static`
  variable inside (two in `env-sdl`: a cache of 8 entries, a table of
  constants) has one copy per unit. Harmless there; not in general.
- `a_lin` carries the index of the file in the program: a new file
  changes the C of the units after it.
- `-c` prints the units one after the other, each after a line
  `//// atmos unit : NAME.c` (`doom` and `env-esp32` build from `-c`).
- The cache is never cleaned (45 MB after the suites of Pingus).
- The manual is not updated.
- An `await` at the top level of a module does not hold the importer
  (accepted in `#2`).

## The prelude (2026-11-26)

**Autor**: "Uma pergunta, o prelude é usado? É necessário? Pq?"

Counted in what is cloned here (the search of the organization returned
nothing, so the other environments and ports were not counted):
`env-sdl/sdl.atm` (one prelude of 1080 lines: the SDL headers, 27
variables, the functions that its natives call, `main`), two tests of
`env-sdl`, `sdl-pingus/engine/sdl.atm` (one line, `#include
"engine/pingus.h"`), 17 files of `lang/tst` and `doc`.

Why it exists: the other native literals are statements or expressions
inside a function; an `#include`, a type, a C function or a C variable
needs the file scope. Without it, that C would live in `.h`/`.c` files
given to the C compiler by flags, and a program would no longer be
only its `.atm` files.

## The two preludes (2026-11-26, decided, on `cc`)

- `:prelude.h`: in every unit. `:prelude.c`: only in the unit of its
  file. `:prelude` alone: an error, always (`#4a`). `.once` is gone.
- `lang`: the 15 cases with a prelude use `:prelude.c`, `tst/env/env.atm`
  uses `:prelude.h`; new case `native-err-04`; the manual has the two
  tags and a section "Import"; the README too.
- Pingus converted again with the new tags: 40/40. The script of the
  conversion is `.claude/plans/261126-cc-pingus-import.py` (a copy of
  `sdl-pingus` and `env-sdl` side by side; an experiment, not the
  migration of the ports).
- Before a merge: every repository with a `:prelude` fails to compile
  until it chooses a suffix. Known here: `env-sdl` (`sdl.atm`, 2 tests),
  `sdl-pingus` (`engine/sdl.atm`). The others were not counted.

## `-c` with modules (2026-11-26, decided, on `cc`)

`atmos -c -o <dir> f.atm` writes `<dir>/main.c` and one
`<dir>/<module>.c` per module, and prints their paths. Checked by hand:
the two files of `tst/cases/import-01` compile with `cc` and the
runtime, and run. `atmos -c f.atm` on a program with modules is an
error (`import-err-04`). The manual ("Execution") has it, and
`ATMOS_CACHE`.

A slip of this session, fixed: the script of the Pingus experiment
followed the link to `lang` in the copy and rewrote
`tst/cases/native-err-04.atm` (its `:prelude` became `:prelude.h`), which
was committed so in `c` of the previous step; the case and the script
are corrected.

## The build directory (2026-11-26, on `cc`, to be confirmed)

Replaces the cache by hash (`ATMOS_CACHE` is gone).

- The C, the object and the list of included files of each module stay
  in `.atmos/<hash of the command>/` of the current directory, or in
  the directory of `--build <dir>` (also `--build=<dir>`), as
  `<module>.c`, `.o`, `.d`; the program itself under the name of its
  file (`main_atm.c`, `tst_engine_atm.c`), so that two programs of a
  directory do not take the place of each other. The file `cmd` of the
  directory has the command. A directory per command: a build with
  ASAN or with `-t` (which changes the C) does not replace the other.
- A unit is compiled again if its C differs from the kept file, or a
  file in its `.d` is newer than its object.
- If the directory cannot be written, all goes to the temporary
  directory and is removed, as a program with no `import`.
- A program with no `import` leaves nothing, as before.
- For the same module to have the same C from two programs: the path
  of a file is normalized (`tst/../engine/x.atm` is `engine/x.atm`,
  also in the messages), and what the compiler adds to a module (its
  final `await(false)`) is at line 1 of its file, not at the import.
- `.atmos/` is in `.gitignore`; `tst/run.sh` no longer sets a cache.

Pingus converted, same container (`tst/run.sh`, 40 tests, all pass):
first run 81 s, second 47 s; with `ASAN=1`, 133 s then 103 s (measured
before the last fix). Build `-O1`: 12.4 s from nothing, 0.2 s with no
change, 2.2 s after a change in `engine/engine.atm`. `.atmos` has 18 to
54 MB with 2 or 3 commands.

`#6` decided: the default directory is `build/` (not `.atmos/`, which
the section above names). `lang` already ignores `build/`; each port
adds it to its `.gitignore`.

## State of `cc` (2026-11-27)

Merged with `main` (`origin/main` of 2026-11-27). `make test` (plain
and `ATMOS_ASAN=1`: lex 33, ast 97, cases 560, golden 443, env 1, guide
21) and `make test-cfgs` (48) pass.

| `sdl-pingus/tst/run.sh` (40 tests) | Time | Result |
|---|---|---|
| `include`, only `:prelude` -> `:prelude.c` | 125 s | all pass |
| `import` (converted copy), first run | 75 s | all pass |
| `import`, second run (from `build/`) | 43 s | all pass |

The original Pingus and env-sdl fail on `cc` with `invalid native :
expected ':prelude.h' or ':prelude.c'` (`#4a`): they must migrate at
the merge.

Not started, of the author's request: `--verbose` (his point 2), C flags
on the command line (his point 3).
