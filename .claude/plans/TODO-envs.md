# TODO of the session `atmos-envs`

The environments of this implementation: how a program selects one, how
two share one program, their events, their test mode, and new ones.
Material: `261117-envs.md`. Numbers in parentheses are those of the
compiler's list (`TODO.md`), which do not change.

Last update: 2026-11-20.

## Standing

- **Work with no input** (the author, 2026-11-20, `atmos-lang#95`; his words checked in that session's transcript): do the work that needs no decision of his; where his decision is pending and there is a recommendation, go on with it if reversible and on a branch, marked "by the session, to be confirmed"; do not ask, do not wait. Language changes on `main` stay his.
- The rules of `pin` are stable (the author, 2026-11-19, `atmos-pin#9`; `TODO-pin.md`, "Stable"). Designs here build on them; a proposal that needs a rule of `pin` changed goes to `atmos-pin` first.

## Index

Items are `#N` (standing rule of the author, 2026-11-18): unique in this
session, in order, never reused. The earlier `A1`..`A9` map to `#1`..`#9`.
An item of another session is `session#N` (the compiler's list is
`atmos-lang#N`).

| # | Title | State |
|---|-------|-------|
| #1 | Two environments in one program (`atmos-lang#57`): `atmos.env` | Decided 2026-11-17, done (`81986e7`, pilot uv+iup) |
| #2 | One vocabulary of events, that of pico (`atmos-lang#46`) | Decided 2026-11-17, done (sdl, pico, ray) |
| #3 | Tests of a game by `test` blocks (`atmos-lang#58`) | #3a by the session, to be confirmed (2026-11-20): `env-sdl` first, asked to adopt it on a branch; `sdl-pingus` to rerun `bomber-test` over it. Manual section on branch `envs` (`2d6cc00`). `env-pico` did it on its `main` (env-pico#9, by that session, to be confirmed): a `test` block at the end of `pico.atm` sets `SDL_VIDEODRIVER=dummy`, `SDL_AUDIODRIVER=dummy`, `PICO_TESTS=1` and `pico.test = true`; `atmos-pico -t` needs no display, screenshots byte-identical to xvfb; test `tst/t-test.atm`, doc `doc/api.md` "Tests". Before: pilot `sdl-pingus#15` passed 2026-11-19 (byte-identical trace; `SDL_ATM_FRAMES`, `_KEYS`, `PINGUS_ORDERS` no longer needed). Left: the environment still opens window and audio under `-t` (needs the test flag of `261117-envs-atmos.md`); switches read by `getenv` cannot be set by a test (must be parameters or settable globals). Not covered: polled input, pixels, C callbacks. Put to the author 2026-11-19 (asked through `atmos-lang`), proposal `261117-envs-tests.md`, decisions #3a..#3d |
| #4 | Selection of an environment, `make env`/`-e` (`atmos-lang#59`) | Open |
| #5 | Split `env-ray` from its games; Lessmilk's copy (`atmos-lang#47`) | Open |
| #6 | `emit @t` whose target throws (`atmos-lang#60`) | Open, language: `atmos-lang` |
| #7 | New environments: POSIX, browser, boards (`atmos-lang#52`) | Open; `env-esp32` read 2026-11-20: `261117-envs-boards.md` (what a board needs: no clock, done; a wake that is not an `fd`, see #10; one env or many is `env-esp32#16`) |
| #8 | The order inside a frame | Decided 2026-11-18, done (pico, ray; sdl already) |
| #9 | What `true` from `step` means with secondaries | By the session, to be confirmed: quit requested, from any environment; "nothing can awake" only for a primary alone. Test `env-true-01` (passes today) and manual line, branch `envs`. Alternative from `env-uv#4` (2026-11-20): each step reports "I may still be awoken" and the runtime ends when none can; case `env-uv` `tst/env-primary.atm` |
| #10 | A secondary wakes a waiting primary (an `fd`) | By the session, to be confirmed: `fd` in a secondary, `atm_env_fds` for the primary; failing test `env-fd-01`, branch `envs`. Case of `env-uv#2`: a primary that waits 100 ms makes a loopback echo take 100-200 ms (`env-uv` `tst/pair/iup-echo.atm`; `uv_backend_fd` is its `fd`). Alternative for boards (a `wake` of the primary, `atm_env_wake`) in `261117-envs-boards.md`: to weigh before it is built |
| #11 | A dedicated compiler forces its environment as primary | By the session, to be confirmed: modes settled when the main chunk ends; the primary is the first with `mode.prefer=:primary`, else the first. Failing test `env-prefer-01`, branch `envs`. Case of `env-uv#3`: `atmos-uv prog.atm` with `include "iup.atm"` |
| #12 | "The include is the start" of an environment, in the manual | Done on branch `envs` (`2d6cc00`), to merge by `atmos-lang` |
| #13 | No test of `iup.atm` as a secondary | Asked of `env-iup` 2026-11-20 |
| #14 | `:frame` stays; `:clock` is time only | Decided 2026-11-17 |
| #15 | No `task main` | Decided 2026-11-17 |
| #16 | Build configurations (`ATMOS_INT`, `ATMOS_REAL`, `ATMOS_TIME`; `atmos-lang` `692af43`) and the environments: unit of the time emitted, C types in the glue, libraries built apart. Open with the author as `atmos-float#14`; touches the API of environments (where an environment declares or checks the configuration) | Table of facts in `261117-envs-float.md` (sdl, iup, pico follow the unit; uv, ray, esp32 emit µs), the input for `atmos-float#14` |


## A. Decisions waiting for the author

One at a time, each with the recommended option.

| # | Decision | Recommended |
|---|----------|-------------|
| #3 (A3) | (58) Tests of a game: a `test` block that spawns the game's own task and emits its input and time; the environment knows it is under `-t` by a global set in a `test` block of its own file (both checked, no compiler change; `261117-envs-atmos.md`) | Each environment adopts it, and the `*_ATM_*` variables can go. To propose to the environment sessions after the pilot of A1 |
| #4 (A4) | (59) Confirm the mechanism of selection: `make env` builds `atmos-<env>`, `-e` does it by hand | Confirm. With A1, `make env` takes a list of environments |
| #5 (A5) | (47) `env-ray` holds the binding and its 23 games; `ray-lessmilk` keeps a patched copy (also item 21 of `env-ray`) | Merge Lessmilk's patch into `ray.atm` first; split the games out later |
| #6 (A6) | (60) An `emit @t` whose target throws: the emitter gets it, not the holder | To the holder, as a broadcast does (departs from the reference). Language matter: belongs to `atmos-lang` |
| #7 (A7) | (52) New environments: POSIX (serial, fds, signals), the browser (WebAssembly), embedded boards | `env-posix` first, as the first secondary environment of A1 (FrescoGO! is its case) |
| #9 (A9) | What a `true` from `step` means with secondaries. `env-uv` ends the program as primary when nothing can awake it, which is unsafe with secondaries (they may still have input); it now does so only when alone (reported 2026-11-18) | As in lua-atmos: `true` means "quit requested" (a window closed); "nothing can awake" only from an environment that is alone. A line in the manual, by `atmos-lang` |

## B. Work known and not done

- The pilot passed (`env-uv` `ecc8b48`, `env-iup` `944b9a9`): the pair with
  two includes, no weak symbols, ASAN clean. Four gaps reported by `env-uv`
  (2026-11-17):
  1. The quits run before `atm_quit`, so globals holding handles are
     finalized after the library is closed. Fixed in `atmos-lang` `c12cf52`:
     quits run inside `atm_free`, after the variables, before the constants
     (a `quit` sees the program's variables as nil).
  2. A primary that waits delays the secondaries (IUP's 10 ms tick is the
     network's latency). Design for the author: a secondary gives the
     primary something to wake it (an `fd` field).
  3. A dedicated compiler forces its environment as primary (`atmos-uv`
     with IUP). Design for the author.
  4. Each environment re-invents carrying the error of a C callback out of
     its step. Fixed in `c12cf52`: the first error of an emit from C inside
     a step is kept and becomes the step's status.
  Added by `env-iup` (same day; it confirms 1 and 4):
  5. When an environment starts is implicit (`iup.atm` opens the library
     in a top-level native at its include). Recommended: the convention
     "the include is the start", written in the manual.
  6. The primary must bound its wait (at `atmos.next()` or a short tick),
     or it starves the secondaries. In the manual since `atmos-lang` `129eb6a`.
  7. `mode.current` is awkward to reach from the step (the table must be
     unpinned and named). Fixed in `129eb6a`: `step(:primary)` /
     `step(:secondary)`; `mode.current` stays.
  Cleanups done: `env-iup` (13 tests, ASAN), `env-uv` `63ba2a8` (34 tests, ASAN;
  as primary with secondaries it waits 10 ms at most).
  8. The secondary mode of `iup.atm` has no test (no primary runs before
     IUP).

- (56) Looks done: `env-ray` already renamed `raylib.atm` to `ray.atm`,
  the table to `ray`, and dropped the prefix. To confirm with the `env-ray`
  session and close in the compiler's list.
- `pico-frescogo` chose option A for the serial line (2026-11-16: natives
  over POSIX polled once per frame, no new environment for now). So item
  57 does not block it; `env-posix` is its item B.9, for later.
- The pair of `env-uv` and `env-iup`: latency is the tick of IUP (10 ms);
  no dedicated compiler for a pair; no section in `doc/guide.md`.

## C. Reported and not acknowledged

- `atmos-lang` `81986e7`: `atmos.env` and the generic `main`, with three changes from the draft: the env table must be unpinned, a missing `mode` is an error, the error of the main chunk is printed before the quits.

## D. Decided by the author

| Date | Decision |
|---|---|
| 2026-11-17 | (57) The interface of an environment in Atmos, as in lua-atmos: `atmos.env [step, quit, mode]`, a generic `main` in the runtime, primary and secondary, and the case with no `step`. Done in `atmos-lang` `81986e7` (the table must be unpinned; a missing `mode` is an error; up to 8). Pilot told to `env-uv` and `env-iup` |
| 2026-11-17 | (46) `:frame` stays, in every environment with frames (pico, sdl, ray), for the logic of a frame, before `:draw`; `:clock` is time only (it awakes once per piece of time, many per frame). Environments with no frames (uv, iup, posix) emit none. Told to `env-pico`, `env-ray`, `env-sdl`, `atmos-lang` |
| 2026-11-17 | (46) Done in `env-ray` `4d5f5b5` (57 of 57 runs identical; no `:mouse.motion`, the games poll the position; Lessmilk told). One vocabulary for all, that of pico: `:quit`; `:key.dn`/`:key.up` with `key` the name and `code` the number; `:mouse.motion`, `:mouse.button.dn`/`.up` with `x`, `y`, `button`; `:frame [n, us]` then `:draw`; events of a handle carry `h`. `env-ray` renames (binding, 23 games; Lessmilk follows). Told to `env-ray`, `atmos-lang` |
| 2026-11-18 | (46) The order inside a frame, for all: the input, `:frame`, the time, `:draw` (that of sdl; SNKRX sets the clock factor in `:frame` for the time of the same frame). Done: `env-pico` `10e6783d` (66 tests identical), `env-ray` `8350e6f` (57 of 57 identical; the comparison with the originals holds) |
| 2026-11-17 | No `task main`: the file stays the main task; a game's own task is what tests spawn |
