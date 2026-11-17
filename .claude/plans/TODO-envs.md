# TODO of the session `envs`

The environments of this implementation: how a program selects one, how
two share one program, their events, their test mode, and new ones.
Material: `261117-envs.md`. Numbers in parentheses are those of the
compiler's list (`TODO.md`), which do not change.

Last update: 2026-11-17.

## A. Decisions waiting for the author

One at a time, each with the recommended option.

| # | Decision | Recommended |
|---|----------|-------------|
| A2 | (46) The rest of the vocabulary: `env-sdl` adopted that of `pico.atm`; `env-ray` still has `[key=262, name='RIGHT']` and `:mouse.dn`/`.up` | The vocabulary of pico for all: `key` the name, `code` the number; `:mouse.motion`, `:mouse.button.dn`/`.up` with `x`, `y`, `button`; handle events carry `h` |
| A3 | (58) Tests of a game: a `test` block that spawns the game's own task and emits its input and time; the environment knows it is under `-t` by a global set in a `test` block of its own file (both checked, no compiler change; `261117-envs-atmos.md`) | Each environment adopts it, and the `*_ATM_*` variables can go. To propose to the environment sessions after the pilot of A1 |
| A4 | (59) Confirm the mechanism of selection: `make env` builds `atmos-<env>`, `-e` does it by hand | Confirm. With A1, `make env` takes a list of environments |
| A5 | (47) `env-ray` holds the binding and its 23 games; `ray-lessmilk` keeps a patched copy (also item 21 of `env-ray`) | Merge Lessmilk's patch into `ray.atm` first; split the games out later |
| A6 | (60) An `emit @t` whose target throws: the emitter gets it, not the holder | To the holder, as a broadcast does (departs from the reference). Language matter: belongs to `ceu-atmos` |
| A7 | (52) New environments: POSIX (serial, fds, signals), the browser (WebAssembly), embedded boards | `env-posix` first, as the first secondary environment of A1 (FrescoGO! is its case) |

## B. Work known and not done

- The pilot passed (`env-uv` `ecc8b48`, `env-iup` `944b9a9`): the pair with
  two includes, no weak symbols, ASAN clean. Four gaps reported by `uv`
  (2026-11-17):
  1. The quits run before `atm_quit`, so globals holding handles are
     finalized after the library is closed. Fixed in `ceu-atmos` `c12cf52`:
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
  Added by `iup` (same day; it confirms 1 and 4):
  5. When an environment starts is implicit (`iup.atm` opens the library
     in a top-level native at its include). Recommended: the convention
     "the include is the start", written in the manual.
  6. The primary must bound its wait (at `atmos.next()` or a short tick),
     or it starves the secondaries: a line for the manual. Sent to
     `ceu-atmos`.
  7. `mode.current` is awkward to reach from the step (the table must be
     unpinned and named). Sent to `ceu-atmos`: pass the mode to `step` as
     its argument.
  8. The secondary mode of `iup.atm` has no test (no primary runs before
     IUP).

- (56) Looks done: `env-ray` already renamed `raylib.atm` to `ray.atm`,
  the table to `ray`, and dropped the prefix. To confirm with the `raylib`
  session and close in the compiler's list.
- `pico-frescogo` chose option A for the serial line (2026-11-16: natives
  over POSIX polled once per frame, no new environment for now). So item
  57 does not block it; `env-posix` is its item B.9, for later.
- The pair of `env-uv` and `env-iup`: latency is the tick of IUP (10 ms);
  no dedicated compiler for a pair; no section in `doc/guide.md`.

## C. Reported and not acknowledged

- `ceu-atmos` `81986e7`: `atmos.env` and the generic `main`, with three changes from the draft: the env table must be unpinned, a missing `mode` is an error, the error of the main chunk is printed before the quits.

## D. Decided by the author

| Date | Decision |
|---|---|
| 2026-11-17 | (57) The interface of an environment in Atmos, as in lua-atmos: `atmos.env [step, quit, mode]`, a generic `main` in the runtime, primary and secondary, and the case with no `step`. Done in `ceu-atmos` `81986e7` (the table must be unpinned; a missing `mode` is an error; up to 8). Pilot told to `uv` and `iup` |
| 2026-11-17 | (46) `:frame` stays, in every environment with frames (pico, sdl, ray), for the logic of a frame, before `:draw`; `:clock` is time only (it awakes once per piece of time, many per frame). Environments with no frames (uv, iup, posix) emit none. Told to `pico-sdl`, `raylib`, `env-sdl`, `ceu-atmos` |
| 2026-11-17 | No `task main`: the file stays the main task; a game's own task is what tests spawn |
