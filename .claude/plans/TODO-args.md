# TODO args

Session `args`. Design: `261118-args-rules.md`.

## Waiting for the author

1. Which rule: R1 (await only), R2 (Lua extended, tight), R3 (calls as
   statements, loose). Recommended: R3, conditional on the port count.
2. Access to the ports for this session (attaching was refused), so the
   R3 diff can run on them.

## Known, not done

- Measure R2/R3 on the 9 ports (parser variants ready in the scratchpad;
  rebuild from `261118-args-rules.md` section 2 if lost).
- Sub-questions after the rule: `f -x`; prefix `!`/`#`/`~` as argument
  starts; `f (x), y` vs `return (x), y`; `emit` and `await` vs the rule.

## Reported, not acknowledged

- The table and measurements of `261118-args-rules.md`.

## Decided

(nothing yet)
