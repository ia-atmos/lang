# TODO args

Session `args`. Design: `261118-args-rules.md`.

## Waiting for the author

1. `f -x`: subtraction as today, or a call when `-` is glued to `x`
   and spaced from `f` (Ruby, MoonScript). Recommended: subtraction.
2. Measuring the ports: attaching them here was refused, and sending
   atmos-lang a prototype patch so it could run the diff on its clones
   (its offer) was refused too. The author decides: grant access, or
   allow the patch to be sent to atmos-lang.

## Known, not done

- Measure R2/R3 on the 9 ports (parser variants ready in the scratchpad;
  rebuild from `261118-args-rules.md` section 2 if lost).
- Sub-questions, after item 1: prefix `!`/`#`/`~` as argument
  starts; `f (x), y` vs `return (x), y`; `emit` and `await` vs the rule.

## Reported, not acknowledged

- atmos-lang (2026-11-18) confirmed it keeps away from the call part of
  the parser, offered to measure the ports with a prototype patch, and
  asked for the full suite (tst/run.sh, make test-rt test-env,
  ATMOS_ASAN=1) before the push.
- The table and measurements of `261118-args-rules.md`.

## Decided

- 2026-11-18: R3 with nesting. A juxtaposed argument list extends as
  far as possible, in any position (`print f x` is `print(f(x))`,
  `g(f x, y)` is `g(f(x, y))`), as `return` does. Not R4 (outermost
  position only). Not sent to atmos-lang yet: waiting for the
  sub-questions, so the rule goes complete.
- 2026-11-18: constructor arguments are greedy too (`print "a" ++ s` is
  `print("a" ++ s)`, `type [] == :t` is `type([] == :t)`; `f "a" "b"`
  becomes an error).
- 2026-11-18: this session implements the change itself (parser, tests,
  manual), and atmos-lang migrates the ports. atmos-lang was told to keep
  away from the call part of the parser until the push.
