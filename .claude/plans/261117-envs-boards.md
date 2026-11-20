# A board as an environment: what `env-esp32` needs from the API (#7)

Read on 2026-11-20, `ia-atmos/env-esp32` at `9e69d81` (the C backend:
`env/*.atm`, `drv/esp32/*.c`, `drv/host/host.c`; the Lua version is still
beside it, `env-esp32#6`).

## How it uses the API today

| Part | What it does | Relation to the API |
|---|---|---|
| `env/clock.atm` | the primary: `step` waits in `drv_clock_wait` up to `atmos.next()`, then `emit(dt)` | as designed; time in microseconds (see #16) |
| `env/gpio.atm`, `ir.atm`, `espnow.atm` | secondaries: `step` drains the C queue of the driver and emits | as designed |
| `drv/esp32/clock.c` | the wait blocks on a binary semaphore that every driver gives when it queues an event (also from an interrupt) | its own answer to #10: the secondaries wake the primary |
| `rt/main.c` with `ATM_NO_CLOCK` (`lang` `ee17162`) | the generic `main`, with no `nanosleep`, called from `app_main` in a FreeRTOS task | works; the program never ends on the board |
| `drv/host/host.c` | the drivers simulated on the host, input from a script (`ESP32_HOST_SCRIPT`), traced (`ESP32_HOST_TRACE`) | its test mode is a variable, as `SDL_ATM_*` before #3 |

## What a board needs

- **No clock in the runtime** (`ATM_NO_CLOCK`): done in `lang`.
- **A wake that is not an `fd`** (#10). On a board there are no
  descriptors: the drivers give a semaphore. The `fd` of the proposal
  of #10 (branch `envs`, `tst/cases/env-fd-01`) serves uv and sockets,
  not a board. A form that serves both: the primary gives the runtime a
  function to wake it (`wake` in its table, callable from C, and from an
  interrupt on a board), and a secondary calls `atm_env_wake()` when it
  queues input. On POSIX the primary implements `wake` with a pipe or an
  `eventfd`; on the ESP32 with the semaphore it already has.
  To weigh against the `fd` before #10 is decided.
- **One environment or one per driver** (`env-esp32#16`, asked of the
  author 2026-11-19; recommended there: one). With one environment, the
  board needs no secondaries at all, and the wake stays inside it. The
  API supports both; nothing to change here.
- **`true` from a step** (#9): the clock returns `true` only when its
  wait fails, which never happens on the board; consistent with #9.
- **No quits**: the program never ends on the board, so `quit` never
  runs there; it runs on the host.
- **Tests** (#3): the host drivers read a script from a variable. With
  #3 the tests would be `test` blocks that emit the events of the
  drivers; the script stays useful for the comparison with the Lua
  version. To propose to `env-esp32` after #3 is adopted by `env-sdl`.
- **The time** (#16): emitted in microseconds; under `ATMOS_TIME=ms`,
  which a board may want, it must follow the unit of the build.

## Update 2026-11-20 (from `env-esp32`)

- `env-esp32#16` decided by the author and done: **one environment per
  board**. `env/esp32.atm` is the only `atmos.env` (primary): its step
  waits on the semaphore (timeout `atmos.next()`), emits the time, then
  drains the queued events of all drivers in timestamp order (each
  driver registers a peek of its oldest event, `drv/src.c`). `gpio`,
  `ir`, `espnow`, `preferences`, `debug` are plain includes that
  register a drain in `esp32.drain`. `app/init.atm`: image 989 KB to
  357 KB (no Wi-Fi linked), static RAM 42 KB to 18 KB.
- So the board needs neither secondaries nor #10: the wake stays inside
  one environment.
- A lesson for the design of secondaries: one environment per source
  loses the order of events across sources (the steps run in turn); one
  step that drains by time keeps it. Secondaries remain for libraries
  with loops of their own (uv beside IUP), not for sources of one
  device.
- Watchdog: a reaction that never awaits keeps the main task busy and
  starves the idle task of FreeRTOS (a warning after 5 s, no reset).
  `doc/env.md` of `env-esp32` suggests `await(1ms)` in long loops (`await(0)` does not yield: it ends in the same reaction). A note
  for any environment on an RTOS board.
