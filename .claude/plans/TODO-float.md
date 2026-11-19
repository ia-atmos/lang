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
| `#1` | Which systems the numeric options must serve | Asked 2026-11-19; recommended: boards of 32 bits (ESP32 family, RP2040, Cortex-M), not the AVR of 8 bits |
| `#2` | Integers of 32 bits (`#A`): one option with the reals, apart, or a profile (`#D`) | Open, after `#1` |
| `#3` | The clock under integers of 32 bits: milliseconds, or a 64-bit timer apart | Open, depends on `#2` |
| `#4` | The runtime built on demand, per configuration (`#E`) | Open, needed with the second option |
| `#5` | Print of a 32-bit real: the fewest digits (done) or always `%.9g` | Open |
| `#6` | Confirm the 32-bit reals in `sdl-tux` and `env-esp32` (an environment built under the option) | Open; neither has run it |
| `#7` | No reals (`#B`) and fixed point (`#C`) | Open; only matter if `#1` includes boards with no FPU as a priority |
| `#8` | An 8-byte value on 64-bit hosts (`#G`, NaN boxing) | Open; independent of the options |

## Checked by this session

- 2026-11-19: `make` builds the four libraries; `ATMOS_REAL=float
  ./atmos tst/cases/real-f32-01.atm` prints 15.000001 / 30.0000019 /
  45.0000038 with `%.9g`, as section 2 of the material says; without
  the option the same program prints 15.0, 30.0, 45.0.
