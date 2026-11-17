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
| A1 | (57) Two environments in one program; the author proposed (2026-11-17) the interface in Atmos, as in lua-atmos: `261117-envs-atmos.md`, proven to run with no compiler change. A pair works today (`env-iup` owns `main` and polls `env-uv`, built with `-DUVA_NO_MAIN`, through weak symbols), but only for that pair | A registry in the runtime, as `step`/`quit` of lua-atmos: an environment that does not own `main` registers a poller and a closer (`atm_poll_add`, `atm_close_add`); the one that owns `main` (the first included, the primary) owns the time and calls `atm_poll()` each turn of its loop. Design document here first, then to `ceu-atmos`, `uv`, `iup` |
| A2 | (46) The names and fields of the events. `env-sdl` adopted the vocabulary of `pico.atm` (decided 2026-11-16); `env-ray` still has `[key=262, name='RIGHT']` and `:mouse.dn`; `pico.atm` has no `:frame` | The vocabulary of pico for all, plus `:frame` (logic) before `:draw`; `key` the name, `code` the number; handle events carry `h` |
| A3 | (58) Tests of a game: a `test` block that spawns the game's own task and emits its input and time works today (`261117-envs-atmos.md`); what is missing is a flag under `-t` so the environment does not open the real window, sound or device | Ask `ceu-atmos` for the flag (as `atmos.test` of the reference); then each environment honors it, and the `*_ATM_*` variables can go |
| A4 | (59) Confirm the mechanism of selection: `make env` builds `atmos-<env>`, `-e` does it by hand | Confirm. With A1, `make env` takes a list of environments |
| A5 | (47) `env-ray` holds the binding and its 23 games; `ray-lessmilk` keeps a patched copy (also item 21 of `env-ray`) | Merge Lessmilk's patch into `ray.atm` first; split the games out later |
| A6 | (60) An `emit @t` whose target throws: the emitter gets it, not the holder | To the holder, as a broadcast does (departs from the reference). Language matter: belongs to `ceu-atmos` |
| A7 | (52) New environments: POSIX (serial, fds, signals), the browser (WebAssembly), embedded boards | `env-posix` first, as the first secondary environment of A1 (FrescoGO! is its case) |

## B. Work known and not done

- (56) Looks done: `env-ray` already renamed `raylib.atm` to `ray.atm`,
  the table to `ray`, and dropped the prefix. To confirm with the `raylib`
  session and close in the compiler's list.
- `pico-frescogo` chose option A for the serial line (2026-11-16: natives
  over POSIX polled once per frame, no new environment for now). So item
  57 does not block it; `env-posix` is its item B.9, for later.
- The pair of `env-uv` and `env-iup`: latency is the tick of IUP (10 ms);
  no dedicated compiler for a pair; no section in `doc/guide.md`.

## C. Reported and not acknowledged

- (none yet)
