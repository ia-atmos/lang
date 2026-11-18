# TODO args

Session `args`. Design: `261118-args-rules.md`.

## Waiting for the author

1. Who implements: this session directly (parser, tests, manual), with
   a note to atmos-lang, or atmos-lang as the rule says. Asked by the
   author. Recommended: here, after the sub-questions, with a note.
2. Access to the ports for this session (attaching was refused), so the
   R3 diff can run on them.

## Known, not done

- Measure R2/R3 on the 9 ports (parser variants ready in the scratchpad;
  rebuild from `261118-args-rules.md` section 2 if lost).
- Sub-questions, after item 1: `f -x`; prefix `!`/`#`/`~` as argument
  starts; `f (x), y` vs `return (x), y`; `emit` and `await` vs the rule.

## Reported, not acknowledged

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
