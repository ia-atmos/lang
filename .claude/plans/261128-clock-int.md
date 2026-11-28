# Time as integers: the material for Wednesday

Gathered on 2026-11-28 by `atmos-lang` for a discussion with the author
on Wednesday 2026-12-02. Nothing here is decided, and nothing is to be
implemented before that conversation.

## 1. The request

**Autor** (2026-11-28, in `atmos-lang`), after the fix of `env-web#77`:
> A solução poderia fazer tudo em inteiros?

and, after the answer:
> Gostaria de mudar pra int. Algo estranho pra mim. Se a arquitetura não
> tem int grande, pq ela teria float? Não faz sentido. Gostaria de
> discutir as decisões antigas e como se relacionam com esse problema.
> De qq forma a implementação vai ficar pra quarta

## 2. The defect that raised it

`env-web#77`: with time emitted in fractional pieces (`emit @:global
333333.3333`), a `watching 5s` lost a tie to an `await 1s` inside it,
20 rounds of 20, which the semantics of synchronous timers forbid. The
cause: each timer counts down in a `double` (`r->u.clk` in
`rt/task.c`), by every piece since it started; two timers that expire
at the same instant end one at `0` and the other at a positive epsilon,
and the tick is cut at the epsilon. `env-web` fixed its side (its
clock emits whole units, ed36663); `rt/main.c` (the default
environment) also emits the fractions of `clock_gettime`.

Branch `clock-whole` (111b2b2, not merged): each tick gives
whole units and keeps the fraction for the next one (`CLK_CARRY`), and
the duration of a timer is rounded up to whole units; the countdowns
then hold only integers, still in `double`. Suite with ASAN, 48
configurations, env-sdl and sdl-pingus pass; the repro goes from 20
lost to 0. It waits for Wednesday (the author wants integers).

## 3. The earlier decisions (session `atmos-float`, 2026-11-19 to 21)

From `TODO-float.md`:

| Item | Decision | Why |
|---|---|---|
| `#1` | No list of target systems: "the architectures may be any; the question is that the types can be configured" | the author |
| `#2a` | Two independent options of fixed size: floats 64 or 32, integers 64 or 32 | memory and code size, not the lack of 64-bit integers |
| `#3a` | A third option for the unit of time, `us` (default) or `ms`: under 32-bit integers a clock in `us` wraps at 35 min | a clock was an integer of the language |
| `#9a` | A clock with a fraction of the unit (`500us` under `ms`, `2.5us`) is a float, in both units | the author's "Ok" to the recommendation |
| `#10` | To compute with time, divide by the unit (`t / 1s`) | portable across units |
| `#12` | With 32-bit integers, floats and pointers, a value is 8 bytes instead of 16 (env-esp32: code -16%) | the reason for 32-bit integers |
| `#17a` | A field of an event named after a unit (`us`) keeps that unit in every build | |

The measurements behind them (env-esp32, ESP32 Xtensa, 32-bit
pointers): 32-bit floats alone gain nothing (-4.6 KB of libc, the
value stays 16 bytes); 32-bit integers with 32-bit floats make the
value 8 bytes (-25 KB of code, -16%). So the 32-bit options exist to
shrink the value, not because a target lacks 64-bit integers: C has
`int64_t` on every target (in software where the hardware is 32-bit),
as it has `double` (in software on the esp32c3, "the soft-float costs
no flash", `#7`).

## 4. How they relate to the defect

- The runtime keeps time in `double` because a clock may carry a
  fraction (`#9a`), and because the environments emit what their clock
  gives (`clock_gettime` in `rt/main.c`, the browser's milliseconds in
  `env-web`). Fractions in the countdown are what broke the tie.
- `#3a` (the unit `ms`) exists only because a clock was an integer of
  the language under `ATMOS_INTEGER=32`. If time inside the runtime is
  an `int64_t` of microseconds whatever the integers of the language
  are, the overflow that `#3a` avoids does not happen inside; it only
  remains for a clock read as a value of the language.
- `#12` (the 8-byte value) concerns values of the language, not the
  internal state of timers: a timer's countdown is a field of the
  runtime and can be `int64_t` with no change to the value.

## 5. Questions for Wednesday (nothing decided)

- **The internal time.** An `int64_t` of microseconds for countdowns,
  the nearest timer and the delay delivered, in every build (`ms`
  included). Cost: none in memory that matters (one field per timer),
  64-bit arithmetic in software on 32-bit targets.
- **A clock with a fraction of the unit (`#9a`).** Keep it a float in
  the language, rounded to whole microseconds when a timer starts (up,
  so it never expires early), or make it a compile error.
- **Time emitted with a fraction** (`emit @:global 333333.333`, the
  default environment's `clock_gettime`). Floor with a carry (the
  branch), or require whole units from the environments (an error), or
  round in `rt/main.c` only.
- **The unit `ms` (`#3a`).** Still needed if the internal time is
  `int64_t`? It changes what a clock literal means as a value of the
  language (`1s` is `1000000` or `1000`), and `env-esp32` uses it.
- **The value seen by `await clock` and the delay** (`val late = await
  1s`): whole units only, or the float as today.
- **The 32-bit options themselves** (the author's "if the architecture
  has no big int, why would it have float?"): the measured reason is
  the size of the value, which needs both at 32 bits; keep, or revisit.
