# TODO args

Session `atmos-args` (was `args`); branch `args`. Design: `261118-args-rules.md`.

## Waiting for the author

1. Is the comma needed: `f x y` as `f(x, y)`? Asked by the author.
   It conflicts with nesting, already decided (`print f x` is
   `print(f(x))`): a juxtaposition is either the next argument or a
   call. Recommended: keep the comma.
2. (Resolved by the new standing rule below: atmos-lang measures the
   ports from the branch `args` before the merge.)

## Known, not done

- Measure R2/R3 on the 9 ports (parser variants ready in the scratchpad;
  rebuild from `261118-args-rules.md` section 2 if lost).
- Sub-questions, after item 1: `emit` and `await` vs the rule.

## Reported, not acknowledged

- atmos-lang (2026-11-18) confirmed it keeps away from the call part of
  the parser, offered to measure the ports with a prototype patch, and
  asked for the full suite (tst/run.sh, make test-rt test-env,
  ATMOS_ASAN=1) before the push.
- `abort t, 1` parses today as `(call abort t 1)`, although the manual
  says `abort` takes one argument (`emit :X, 1` is an error, as
  expected). To align with `emit` in the implementation.
- atmos-lang (2026-11-18): the method call `::` was removed from the
  language in main b4424ff (the `::` branch of `parser_2_suf` is gone;
  golden 0212 now calls through the field), so the `o::f "v" ::f \{}`
  break no longer exists. A session `atmos-oo` may propose a tag after
  a parameter (`func area (c :Shape.Circle)`) and is to agree the
  grammar with this session.
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
- 2026-11-18 (standing rule, `TODO.md` 58eafba; checked in atmos-lang's
  transcript: the author asked for branches at 11:40 and answered
  "1. sim" at 11:45): the compiler change goes on the branch `args`;
  plans stay on `main`. Before the merge: latest `main` into the branch,
  full suite (`tst/run.sh`, `make test-rt test-env`, `ATMOS_ASAN=1`),
  atmos-lang measures the ports, the author approves.
- 2026-11-18: `f -x` stays a subtraction (`f(-x)` to pass `-x`).
  Prefix `!x`, `#t`, `~n` start an argument (`f #t` is `f(#t)`),
  proposed with no objection.
- 2026-11-18: a bare `await` pattern extends over one full expression,
  as `emit`: `await 20min + 1s` is `await(20min + 1s)`, `await t` is
  valid, `await T x` spawns `T(x)`. A comma ends it (one pattern, not a
  list), as it ends `emit` today: `val a, b = await :X, 10`.
- 2026-11-18: `(` after a callee is the classic call, with or without
  a space (`f (a+1)*2` is `f(a+1) * 2`).
- 2026-11-18 (proposed by the author, replaces "a comma ends it"): the
  comma in a bare `await` is as in the others: `await 10, :frame` is
  `await(10, :frame)`, the counted pattern. `val a, b = await :X, 10`
  becomes `await(:X, 10)`: to be a compile error (the count is a tag);
  write `(await :X), 10`. Locally no program depends on the old
  reading (only `tst/cases/await-cnt-err-03`, an error case).
