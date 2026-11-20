# The environments under the numeric options of the build (#16)

Input for `atmos-float#14` (which options each environment supports).
Facts as of 2026-11-20, from each repository and its session. The
options come from `lang` `692af43`: `ATMOS_INT=32`, `ATMOS_REAL=float`,
`ATMOS_TIME=ms`, all off by default.

| Environment | Time it emits | C types in its glue | Libraries built apart | State |
|---|---|---|---|---|
| `env-sdl` | the unit of the build (`85b23cc`), the rest carried | | SDL2 (no Atmos types) | done: the suite passes under each option and all three, except `tst/clock` under `ms` (its trace is of the default unit). Rocks now divides clocks by `1s` |
| `env-iup` | the unit of the build | | IUP (no Atmos types) | done (`68fbe3c`): the suite passes under each option; with `float` only the printed digits of a timer differ |
| `env-pico` | the unit of the build (`4dbf649`) | | `libpico.a`, built by the environment, no Atmos types | done for the time; the rest not reported |
| `env-uv` | microseconds: `a_int((int64_t)dt * 1000)` | `uv.now()`, sizes and positions of files are 64-bit integers | libuv (no Atmos types) | not done (`env-uv#11`): wrong under `ms`; files over 2 GB truncated under `INT=32` |
| `env-ray` | microseconds: `atm_emit(a_int(us))` | colors packed as `int64_t` (`r<<24`), `double` fields | raylib, patched for the test mode | not done (`env-ray#44`): wrong under `ms`; colors with `r >= 128` overflow under `INT=32`; `ray.f32` could shrink under `REAL=float` |
| `env-esp32` | microseconds: `emit(dt)` from `drv_clock_wait` | `int64_t` arguments of the drivers (`esp_int`) | the drivers in `drv/`, compiled with the program | measured in a scratch copy (`env-esp32#21`): `AVal` 16 to 8 bytes, its component -16%; `ATMOS_REAL_FLOAT` builds (image -4.6 KB); not adopted |

What it takes to support each option:

- `ATMOS_TIME=ms`: emit the time in the unit of the build. The way of
  sdl, iup and pico: compute in microseconds, emit `us / 1ms` (the unit
  of the build), carry the rest to the next step. A program that writes
  `us / 1000000` is wrong under `ms`; `us / 1s` is right in every build
  (found by `env-sdl`).
- `ATMOS_INT=32`: no value above 2^31 through `a_int`. Pack colors as
  unsigned and cast back, keep 64-bit quantities (file sizes, `now`)
  as reals.
- `ATMOS_REAL=float`: nothing breaks by itself; the C of a library still
  computes in its own types.

For the API of environments (to be decided with `atmos-float#14`): the
runtime could check, when an environment registers, a field such as
`builds = [time=:us]` against the build, and fail at once with a clear
error, instead of a program that runs at the wrong speed. Not proposed
yet: the decision of which options are supported comes first.
