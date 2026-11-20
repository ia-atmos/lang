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
| `#2` | How the types are configured | Decided 2026-11-20 (`#2a`): two independent options with fixed sizes, `ATMOS_REAL=double\|float` and `ATMOS_INT=64\|32`. Implemented on the branch `float` (922062d), not merged |
| `#3` | The clock under integers of 32 bits (a clock is a number of microseconds: 2^31 us is 35 min) | Decided 2026-11-20 (`#3a`): a third option for the unit of time, `ATMOS_TIME=us\|ms` (default `us`); a clock literal that does not fit the integer is a compile error. Implemented on the branch `float`, not merged. See `#9` |
| `#4` | The runtime built on demand, per configuration (`#E`) | Open. The branch prebuilds all 16 libraries (`libatmos[-f32][-i32][-ms][-asan].a`) |
| `#5` | Print of a 32-bit real: the fewest digits (done) or always `%.9g` | Open |
| `#6` | Confirm the 32-bit reals in `sdl-tux` and `env-esp32` (an environment built under the option) | Both ran it: `env-esp32` built the board image (-4.5 KB, speed not measured); the suite of `sdl-tux` is identical, over `env-sdl` too. Left: Atmos code of Tux doing the float arithmetic |
| `#7` | No reals (`#B`) and fixed point (`#C`) | Open; only matter if `#1` includes boards with no FPU as a priority |
| `#8` | An 8-byte value on 64-bit hosts (`#G`, NaN boxing) | Open; independent of the options |
| `#9` | A clock with a fraction of the unit (`500us` under `ms`, `2.5us` today): a real, or a compile error | Asked 2026-11-20. `#3a` said error; the branch keeps the rule of today (a real, as `tst/cases/int-03` expects), so that no existing program changes. Recommended: keep the real |
| `#10` | A portable way to compute with time as a number (a program that emits `1000` as a time depends on the unit) | Open. Under `ATMOS_TIME=ms`, 12 existing cases differ only for this |
| `#11` | Limits of integers of 32 bits to document: `os.time()` wraps in 2038; the least integer has no literal (as with 64 bits) | Open, minor |
| `#12` | The value of 8 bytes (integers and reals of 32 bits, pointers of 32 bits) was not measured: this host has no 32-bit compiler; `env-esp32` can measure it | Open |

## Checked by this session

- 2026-11-19: `make` builds the four libraries; `ATMOS_REAL=float
  ./atmos tst/cases/real-f32-01.atm` prints 15.000001 / 30.0000019 /
  45.0000038 with `%.9g`, as section 2 of the material says; without
  the option the same program prints 15.0, 30.0, 45.0.

- 2026-11-20, branch `float` (91d9673): `make test` passes with ASAN
  (lex 32, ast 90, cases 427, golden 443, rt, env). The `cases` and
  `golden` suites once under each option, against the outputs of the
  default: `ATMOS_INT=32` 858 pass and 6 differ (wrapping at 32 bits,
  clocks above 35 min); `ATMOS_TIME=ms` 852 pass and 12 differ (times
  printed or emitted as bare numbers); the three options together 848
  pass and 16 differ. No crash and no report of the sanitizer.

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
