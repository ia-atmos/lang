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
| `#2` | How the types are configured | Decided 2026-11-20 (`#2a`): two independent options with fixed sizes, `ATMOS_REAL=double\|float` and `ATMOS_INT=64\|32`. On `main` (692af43) |
| `#3` | The clock under integers of 32 bits (a clock is a number of microseconds: 2^31 us is 35 min) | Decided 2026-11-20 (`#3a`): a third option for the unit of time, `ATMOS_TIME=us\|ms` (default `us`); a clock literal that does not fit the integer is a compile error. On `main` (692af43). See `#9` |
| `#4` | How the 16 configurations of the runtime are built | Decided 2026-11-20 (`#4a`): `make` builds the default ones, the compiler builds a missing one on first use (and `make` keeps those that exist up to date). On `main` (692af43) |
| `#5` | Print of a 32-bit real: the fewest digits (done) or always `%.9g` | Settled by the session on 2026-11-20, to be confirmed: the fewest digits stay. See "Screens" |
| `#6` | Confirm the 32-bit reals in `sdl-tux` and `env-esp32` | Done 2026-11-20: the badguys of `sdl-tux` are plain Atmos with real arithmetic under `ATMOS_REAL=float` (a6cdb0b) and 69 runs are identical to the official game tick by tick, 29 with badguys; `env-esp32` built the board image |
| `#7` | No reals (`#B`) and fixed point (`#C`) | Open. The measurement that decides was asked of `env-esp32` (the software floating point linked on a target with no FPU). See "Screens" |
| `#8` | An 8-byte value on 64-bit hosts (`#G`, NaN boxing) | Open, not recommended now. See "Screens" |
| `#9` | A clock with a fraction of the unit (`500us` under `ms`, `2.5us` today): a real, or a compile error | Decided 2026-11-20 (`#9a`, the author said "Ok" to the recommendation): a real, in both units, as on the branch |
| `#10` | A portable way to compute with time as a number | Done 2026-11-20 as documentation (d96b8b0): a clock is the unit (`t / 1s`); no new constant. By the session, to be confirmed |
| `#11` | Limits of integers of 32 bits to document | Done 2026-11-20 (d96b8b0): `os.time()` wraps in 2038; the least integer is written `-2147483647 - 1` |
| `#12` | The value of 8 bytes (integers and reals of 32 bits, pointers of 32 bits) | Measured by `env-esp32` on 2026-11-20 (compiled, not run): `sizeof(AVal)` 16 to 8, code -25.4 KB (-16%), static DRAM -912 B. Left: heap and stack at run time, which need a board |
| `#13` | Merge of the branch `float` into `main` | Done 2026-11-20 (`#13a`, approved by the author after the measurement of the ports): `main` is 692af43, fast-forward, suite passing with ASAN over the latest main |
| `#14` | Each environment says which numeric options it supports; its compiler refuses the rest | Prepared by the session on the branch `float` (2447d5f), to be confirmed, NOT merged: `make env ... ENV_NUM="float 32 ms"`; a dedicated compiler refuses an option that is not declared; the plain compiler with `-e` checks nothing. Suite passes with ASAN. `env-sdl` (85b23cc), `env-pico` (4dbf6494), `env-ray` (e45a4ad) and a scratch of `env-esp32` support the three options, each after the same fix: the time emitted in the unit of the build |
| `#15` | The build on demand needs `make`, the sources and a writable `<home>/build` | Done 2026-11-20 as documentation (d96b8b0); the error already names the command |
| `#16` | `sdl-tux` keeps two pieces in C for `fmodf`, `powf`, `sinf`, "because Atmos has no fmod" | Answered 2026-11-20: nothing is missing. `%` on reals is `fmod` (`fmodf` under `float`), and `math.pow`, `math.sin` exist. Told `sdl-tux`; a divergence, if it tries, would be a defect to report here |
| `#17` | A time field of an event of an environment (`:frame [us=]` of `env-pico`, `time` of `env-esp32`): the unit of the build, or the unit in its name | Open, his to decide, with `atmos-envs#16`. Reading of the session: a field named after a unit keeps it |

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

- 2026-11-20, `env-esp32` (`#12`), branch `float` 91d9673 in a scratch
  copy, ESP32 (32-bit pointers), gcc 15.2 `-O2`, `app/init.atm`:

  | Options | `sizeof(AVal)` | Component | Static DRAM |
  |---|---|---|---|
  | none | 16 | 157876 B | 42307 B |
  | `ATM_REAL_FLOAT` | 16 | 158270 B | 42307 B |
  | + `ATM_INT_32` | 8 | 132468 B | 41395 B |
  | + `ATM_TIME_MS` | 8 | 132517 B | 41395 B |

  Heap and stack high-water not measured (no board; the host cannot
  link 32-bit programs). Its host tests pass with `ATMOS_REAL=float
  ATMOS_INT=32`; under `ATMOS_TIME=ms` only the printed times differ.
  For `ms` its environment changed: the step converts `atmos.next()`
  to microseconds with `A_TIME_US`, emits whole milliseconds and keeps
  the remainder; the `time` fields of its events become milliseconds.
  It found that `make` at 91d9673 had no rule for `build/libatmos.a`:
  fixed since 72d3398.
- 2026-11-20, `atmos-lang` (`#13`): `float` 3bec041 against main
  3ae60f9, default configuration. `make test` passes with ASAN on a
  clean copy. The 2116 `.atm` files of the 12 port clones, `-c` with
  both compilers: diagnostics and exit identical, generated C identical
  after the two renamings of the branch (`a_int((AInt)`). The suite of
  sdl-pingus passes. `tonumber("9223372036854775808")` is now a real;
  no port depends on it. No port links `libatmos-f32*.a` by name.
  pico-frescogo and env-esp32 were not cloned there.
- 2026-11-20, `env-sdl` through `atmos-lang` (`#14`, `#10`): `sdl.atm`
  at 85b23cc supports the three options; its one defect was the clock,
  emitted in microseconds whatever the unit (now `acc / A_TIME_US`, the
  rest carried). Colors `0xRRGGBBAA` survive `float` and 32-bit
  integers. Its 23 tests pass under each option and the three together,
  except one whose trace is of the default unit.
- 2026-11-20, `sdl-tux` (`#6`): see the index. The only C left in the
  runs with badguys reads the collision system, plus two pieces with
  `fmodf`, `powf`, `sinf` (the flying snowball), "because Atmos has no
  fmod" (`#16`).
- 2026-11-20, `env-pico` (`#14`), main 4dbf6494 on lang 55b1fee:
  `ATMOS_TIME=ms` was wrong and is fixed (the loop emitted microseconds
  in every build); `:frame [us=]` stays in microseconds (`#17`);
  `ATMOS_INT=32` compiles and runs, with colors `0xRRGGBBAA` to check
  against the wrapping of a hexadecimal literal; `ATMOS_REAL=float`
  nothing found; its C library uses no Atmos type. Sample tests pass
  under each option alone; the full suite only in the default build.
- 2026-11-20, `env-ray` (`#6`, `#14`, `#17`; its `#44`), on lang
  c4e99c4. arkanoid, which rounds by hand with 13 calls of `ray.f32`:
  with `ATMOS_REAL=float` and the calls removed, both runs (1517 and
  21460 frames) stay identical to the C; the same file with doubles
  diverges at line 7531. Not committed in the games. `ray.atm` emitted
  microseconds in every build: fixed in e45a4ad. `tst/env` passes under
  each option alone; the glue builds unchanged. To watch: a color
  computed through a real loses bits under `float`; where the C mixes
  doubles and floats (asteroids, koala_seasons) `ray.f32` stays;
  `:frame [us=]` stays in microseconds (`#17`).
- 2026-11-20, `env-esp32` (`#12`), its real line (one environment,
  modules linked only when included), lang 55b1fee, board build:
  `libmain.a` 138873 B in the default, 128104 B with the three options
  (-10769 B); static DRAM 17782 B to 17078 B (-704 B); no warnings.
  Its host tests give the same output in both. Speed and heap need the
  board: the steps are in `doc/board.md` of `ia-atmos/env-esp32`.

## The matrix of configurations (2026-11-20)

The `cases` and `golden` suites (863 cases with no `.env` of their
own), with ASAN, under each of the seven configurations that are not
the default, against the outputs of the default. Compiler and runtime
of main 692af43.

| `REAL` | `INT` | `TIME` | Pass | Differ |
|---|---|---|---|---|
| double | 64 | ms | 853 | 12 |
| double | 32 | us | 859 | 6 |
| double | 32 | ms | 851 | 14 |
| float | 64 | us | 861 | 4 |
| float | 64 | ms | 850 | 15 |
| float | 32 | us | 857 | 8 |
| float | 32 | ms | 849 | 16 |

The cases that differ under a combination are exactly the union of
those that differ under each of its options alone: no interaction.
Every difference was read; none is a defect, a crash or a report of
the sanitizer.

| Class | Option | Cases |
|---|---|---|
| Unit of time: a time printed, or a bare number emitted as a time | `ms` | `await-cnt-01`, `clk-01`, `dump-01`, `env-01`, `int-03`, `load-01`, `loadx-01`, `timer-02`, `timer-03`, `timer-07`, golden `0490`, `0491` |
| Overflow: wrapping at 32 bits, a decimal that becomes a real | `32` | `bit-01`, `int-01` |
| Overflow: a clock above 35 min is a compile error | `32` | `clk-01`, `dump-01`, `int-03`, `load-01` |
| Precision: a real printed with the digits of a float, `1e100` as infinity | `float` | `arith-01`, `dump-01`, `int-01`, golden `0002` |

## Work with no input (2026-11-20)

The author, in `atmos-lang` (checked in its transcript, 2026-11-20
14:51 UTC): "I want you to review all sessions and propose them changes
that need no input. The sessions may also work suggest by themselves.
No input form me. Go ahead". What this session settled itself is marked
"by the session, to be confirmed" in the index.

## Screens

### `#5` Print of a 32-bit real

- Today: the fewest digits, from 6 to 9, that read back as the same
  float (`0.1`, `0.33333334`); `string.format("%.9g", x)` gives nine.
- The fewest digits identify the float exactly, as nine do, so nothing
  is lost for a comparison of traces; nine digits print `0.1` as
  `0.100000001`.
- Nothing to measure. Settled by the session: the fewest digits stay.
  To undo: one line in `a_num_fmt` (`rt/val.c`).

### `#7` No reals, or fixed point

- What it would buy: on a board with no FPU (ESP32-C3, RP2040), no
  software floating point in the image, and integer speed.
- What it costs: a second arithmetic in the language (`/` changes
  meaning, real literals and most of `math` go away), so libraries and
  environments written with reals do not load.
- The measurement that decides: the code of the software floating
  point linked on such a target with `float` reals (libgcc and the `f`
  functions of libm). A few KB: not worth it. Tens of KB on a board
  with 256 KB or less: worth a discussion. Asked of `env-esp32` for the
  esp32c3.
- Reading until then: not recommended; `ATMOS_REAL=float` already
  removes the double math (-4.6 KB on the ESP32).

### `#8` An 8-byte value on 64-bit hosts (NaN boxing)

- Measured here (64-bit host): `sizeof(AVal)` 16; the value stack is
  static, `A_STK_MAX` (2^18) values, 4 MB of address space (only the
  pages touched are resident); `ATab` 80 bytes, `ATask` 184.
- What it would buy: half the memory of locals, stack and table
  entries on the desktop, where no port has reported a lack of memory.
- What it costs: a rewrite of the representation (`v.type`, `v.u.x`
  are read all over the runtime, the generated C and the C of every
  environment), 48-bit pointers assumed, integers reduced to 51 bits or
  boxed, and it excludes `ATMOS_REAL=float`.
- The measurement that decides: the peak resident memory of a large
  port against its count of live values. If values are under half of
  it, the gain is under a quarter. No port asked for it.
- Reading: not recommended now; the boards get the 8-byte value by the
  options (`#12`).
