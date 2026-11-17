# envs: the C interface of an environment, then and now

The author's first discussion of environments (before 2026-11-13, not in
the record of `ceu-atmos`), as he gave it to the session `envs` on
2026-11-17, with what changed since. Checked against `rt/atmos.h`.

## The interface

An environment is the `main` in C. It owns the loop and calls the runtime:

| Function | Role | Needed? |
|---|---|---|
| `atm_init(argc, argv)` | starts the runtime, keeps the arguments | yes |
| `atm_main()` | runs the file until it suspends or ends | yes |
| `atm_alive()` | the program is still pending | yes |
| `atm_emit(evt)` | delivers an event (a number is time in µs) | yes |
| `atm_emit_to(t, evt)` | delivers to one task; an exception goes to its holder | optional (new, 2026-11-13, for `env-uv`) |
| `atm_stop()` | aborts the program, running the `defer`s | yes, when leaving before the end |
| `atm_quit()` | frees the runtime | yes |
| `atm_next()` | µs to the next timer, or -1 | optional (to sleep instead of spin) |
| `atm_step(ms)` | shortcut to pass time in ms | optional |
| `a_error_dump()` | prints the pending error | optional |

On the Atmos side there is no required interface; each environment
offers a table (`pico`, `sdl`, `ray`, `iup`, `uv`) and events by
convention.

## Against lua-atmos

| | lua-atmos | Here |
|---|---|---|
| What it is | a Lua table registered with `atmos.env {step, quit, mode}` | a `main` in C |
| Who owns the loop | the runtime: `loop(body)` calls `env.step()` | the environment: it calls `atm_emit` |
| Choice | at run time, `require "atmos.env.sdl"` | at link time |
| Several environments | yes (primary and secondaries) | no, one only |
| No loop (start, JS style) | a separate mode | the same: `atm_main` returns and the environment emits when it wants |
| End | `quit()` called by the stop | the `main` cleans up after `atm_stop` |
| Body | an explicit `loop(f)`; the `atmos` command wraps the file | the whole file is the main task |

## What changed since

- `lib/sdl.atm` left the compiler (now `ia-atmos/env-sdl`); events have no
  prefix (`:key.dn`, not `:sdl.key.dn`).
- `atm_emit_to` (above).
- The choice is still at link time, but packaged: `make env` builds
  `atmos-<env>`, and `-e <env.atm>` does it by hand (item 59).
- Several environments: one pair works by hand. `env-uv` built with
  `-DUVA_NO_MAIN` has no `main` and emits no time, and offers
  `uva_poll`/`uva_close`; `env-iup` owns `main` and calls them through
  weak symbols (`env-uv`, `tst/pair/iup-echo.atm`). Item 57.
