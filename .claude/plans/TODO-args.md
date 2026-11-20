# TODO args

Session `atmos-args` (was `args`); branch `args`. Design: `261118-args-rules.md`.

## Index of items (`#N`, standing rule of 2026-11-18)

Numbers start with the first item after the rule; what was decided
before it is under "Decided", by date.

| # | Title | State |
|---|---|---|
| #1 | Is the comma needed: `f x y` as `f(x, y)`? | decided 2026-11-19: the comma stays (`f x, y`) |
| #2 | Implement the rule on the branch `args` (parser, tests, manual) | done on the branch at 6ba7568 (suite pass=980 fail=0, test-rt, test-env); not merged |
| #3 | atmos-lang measures the ports from the branch, before the merge | done 2026-11-19 by atmos-lang (branch 6ba7568 vs main 170ef73): 2105 files of 12 ports, 2100 identical trees, none changes silently, 5 become errors, all in env-pico, from 3 sites of `key = f [...],` inside a table constructor |
| #4 | `abort t, 1` is accepted as a list; align with `emit` (one argument) | done on the branch (`expected single argument`) |
| #5 | May the comma be optional in every list (`[1 2 3]`, `f(a b)`)? Asked by the author | moot after #8 |
| #6 | Merge the branch `args` into main | done 2026-11-19: merged by atmos-lang into main as 826e219 (approved by the author, "Ok, lá"); full suite passes; ports told, nothing to migrate |
| #7 | Inside `[...]` and `(...)`, does a comma end the juxtaposed arguments? `[a = f [..], b = 1]` breaks today on the branch (env-pico's style). Options: #7a keep (`[f x, y]` is `[f(x, y)]`, rewrite 3 sites as `f([..])`); #7b the comma belongs to the enclosing brackets (`[f x, y]` is `[f(x), y]`, `g(f x, y)` is `g(f(x), y)`), for calls and statements alike | moot after #8 |
| #8 | The author reconsiders (2026-11-19, after #3): a list with no closing delimiter eats the commas around it, which is harmless only in escapes (`return`, `break`, `throw`, `escape`), after which nothing comes. Proposal: back to the Lua rule, tight, with one argument and no comma, extended to names and numbers (`f x`, `f 10`, `await t`); lists with no parentheses only in escapes. It replaces R3, the greedy constructors, `await E, P`, and #7 | decided 2026-11-19: yes. One atom (literal, name with fields and indexes, constructor), tight, no comma; `f g x`, `f g()` are errors; `#`, `!`, `-` do not start an argument; `await` follows; escapes keep their lists |
| #9 | Reimplement the branch `args` for #8 (new commits on top) | done at 823d843 (suite pass=982 fail=0, test-rt, test-env); locally 1051 files, no tree changes; not merged |
| #10 | atmos-lang measures the ports again, against 823d843 | done 2026-11-19 by atmos-lang (823d843 vs main 41361ea): 2105 files of 12 ports, `--ast` and the generated C identical for all; not covered: pico-frescogo, env-esp32 |
| #11 | The names of the exit statements as keywords of the lexer (atmos-lang#69), proposed by atmos-lang under the rule "work with no input" (2026-11-20) | by the session, to be confirmed: done on the branch `args` at 72ea64e (suite pass=994 fail=0; trees of 1087 local files identical, but two error cases); ports measured by atmos-lang on 2026-11-20 (72ea64e vs main 513767c): 2152 files of 15 clones, generated C and diagnostics identical; not merged, the merge waits for the author |
| #12 | Audit of the manual and README after the single-atom rule (169 code blocks through `--ast`, before and after) | done 2026-11-20: nothing to fix; `abort` added to the list of keywords |
| #13 | One test for each new error message | done: already covered (ast/call-jux-err-*, seq-err, abort-err-1) |

## Waiting for the author

- #11: confirm the exit names as keywords, and its merge, after the
  port count. Recommended: yes. Visible change: `val break = 1` now
  says `expected <id>`.

## Known, not done

- Measure R2/R3 on the 9 ports (parser variants ready in the scratchpad;
  rebuild from `261118-args-rules.md` section 2 if lost).
- Sub-questions, after item 1: `emit` and `await` vs the rule.

## Reported, not acknowledged

- What #2 changed beyond the decisions, on the branch: `f() g()` on one
  line is `f()(g())`; `do 10` is `do(10)`; `watching f {` parses (golden
  0462 removed); `await(:x, :tic)` is a compile error (was a runtime
  one); keywords that start an argument: `nil true false ... func if
  ifs match await`; a tag as a callee only takes a constructor.
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
