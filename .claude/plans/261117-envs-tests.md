# envs: tests by `test` blocks in the environments (atmos-envs#3)

Proposal for the adoption of `atmos-lang#58` by the environments, asked by
the author on 2026-11-19 (his "sim" in `atmos-lang`). Settled already: no
compiler change; the `test` block is the source of input, time, frames and
drawing (the loop of the environment does not run); the environment knows
it is under `-t` by a global set in a `test` block of its own file
(`261117-envs-atmos.md`). Pilot: `sdl-pingus#15` (byte-identical trace).

## Today (`*_ATM_*` read with `getenv`)

| Environment | Variables | Test mode |
|---|---|---|
| `env-sdl` | `SDL_ATM_FRAMES`, `_KEYS`, `_MOUSE`, `_SHOT`, `_MUTE`, `_RENDER`, `_ACCEPT` | in `sdl.atm` |
| `env-pico` | `PICO_ATM_FRAMES`, `_KEYS`, `_MOUSE`, `_SHOT` | in `pico.atm` |
| `env-ray` | none in `ray.atm` | a patch of raylib (`tools/raylib.patch`) |
| `env-iup` | `IUP_ATM_SCRIPT` | in `iup.atm` |
| `env-uv` | none | tests run on the loopback |

## Decisions, one at a time

- **#3a** Which environment first. Recommended: `env-sdl` (the pilot is
  there, it has four games, and the missing piece, no window or audio
  under `-t`, is in it). Then pico, iup, ray.
- **#3b** What each environment does:
  1. `var <env>_test = false; test { set <env>_test = true }` at the top
     of its file, readable from C.
  2. Under `-t`: no window shown, no audio, no real time; drawing goes to
     an offscreen target, so a screenshot still works.
  3. Functions for what a test cannot `emit`: the polled input
     (`<env>.test.key(name, down)`, `<env>.test.mouse(x, y, buttons)`) and
     the screenshot (`<env>.test.shot(path)`).
  4. The `*_ATM_*` variables keep working until the last client moved.
- **#3c** Which variables go: `_FRAMES`, `_KEYS`, `_MOUSE`, `_MUTE` (the
  test emits them, or `-t` implies them), `IUP_ATM_SCRIPT` (the test emits
  the actions). Become functions: `_SHOT`. Stay as an option of the test
  mode: `SDL_ATM_RENDER`, `SDL_ATM_ACCEPT` (to check with `env-sdl`).
  `env-ray` keeps its raylib patch for the comparison with the originals.
- **#3d** What the ports change: the game is a task with parameters
  (no `getenv` for switches a test needs); one test at a time moves into a
  `test` block beside the old one; a test moves only with its golden output
  identical; the old script is removed when all its tests moved.
