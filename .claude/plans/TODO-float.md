# TODO of the session `atmos-float`

The numeric types of a build of Atmos: the reals of 32 bits
(`ATMOS_REAL=float`, commit 4565af1) and whether integers and reals
should be options for other systems. Material:
`.claude/plans/261119-float.md` (its labels `#A` to `#G` are its own).
Implementation goes on the branch `float`; plans go to `main`.

Last update: 2026-11-19.

## Index of items

| Item | Title | State |
|---|---|---|
| `#1` | Which systems the numeric options must serve | Closed 2026-11-20 by the author: the architectures may be any, nobody knows who will use it; the question is that the types can be configured. No list of target systems |
| `#2` | How the types are configured | Decided 2026-11-20 (`#2a`): two independent options with fixed sizes, `ATMOS_REAL=double\|float` (exists) and `ATMOS_INT=64\|32`. Not implemented yet: waits for `#3` |
| `#3` | The clock under integers of 32 bits (a clock is a number of microseconds: 2^31 us is 35 min) | Asked 2026-11-20; recommended: a third option for the unit of time, `ATMOS_TIME=us\|ms`, and a clock literal that does not fit is a compile error |
| `#4` | The runtime built on demand, per configuration (`#E`) | Open, needed with the second option |
| `#5` | Print of a 32-bit real: the fewest digits (done) or always `%.9g` | Open |
| `#6` | Confirm the 32-bit reals in `sdl-tux` and `env-esp32` (an environment built under the option) | Both ran it: `env-esp32` built the board image (-4.5 KB, speed not measured); the suite of `sdl-tux` is identical, over `env-sdl` too. Left: Atmos code of Tux doing the float arithmetic |
| `#7` | No reals (`#B`) and fixed point (`#C`) | Open; only matter if `#1` includes boards with no FPU as a priority |
| `#8` | An 8-byte value on 64-bit hosts (`#G`, NaN boxing) | Open; independent of the options |

## Checked by this session

- 2026-11-19: `make` builds the four libraries; `ATMOS_REAL=float
  ./atmos tst/cases/real-f32-01.atm` prints 15.000001 / 30.0000019 /
  45.0000038 with `%.9g`, as section 2 of the material says; without
  the option the same program prints 15.0, 30.0, 45.0.

## Reported by other sessions (not checked here)

- 2026-11-19, `env-esp32` (`#6`): ESP32 (Xtensa LX6, ESP-IDF v6.1.1,
  gcc 15.2, `-O2`), `app/init.atm` with `env/esp32.atm`, runtime and
  generated C under `ATM_REAL_FLOAT`. Component (runtime, program,
  drivers): 156602 B with `double`, 156831 B with `float` (+229 B).
  `libc.a`: 34764 B to 29985 B (-4779 B, the double math of newlib is
  not linked). Image: 988698 B to 984130 B (-4568 B). `AVal` stays at
  16 bytes. Speed not measured (no board; the program has almost no
  real arithmetic; the PID loop of the robot would be the benchmark,
  not ported). Its host tests pass under the option. Board option:
  `idf.py -DATMOS_REAL_FLOAT=ON build`.
- 2026-11-19, `sdl-tux` (`#6`): under `ATMOS_REAL=float` (4565af1) the
  gravity test gives 15.000001, 30.0000019, 45.0000038, and the whole
  suite of the port is identical with the option on (30 traces
  compared tick by tick with the C++ game, two runners, one over
  `env-sdl`, and the screenshots). The float arithmetic of the port is
  still in C kernels; the blocks and badguys in Atmos with floats come
  next, and it will report.
