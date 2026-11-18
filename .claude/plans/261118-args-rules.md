# args: candidate rules for arguments without parentheses

Session `atmos-args` (was `args`), 2026-11-18. Material: `261118-args.md`. Decisions and
open items: `TODO-args.md`.

## 1. Two independent axes

Every form with no parentheses answers two questions:

| Axis | Lua calls today (`f "s"`) | Statements today (`return e`) | `await` today |
|---|---|---|---|
| A. what may start the argument | a constructor: STR TAG `[` `\` CLK NAT | any expression | a constructor, a call (spawns), `:any`/`:all`, `until`; **not** a name |
| B. how far it extends | one primary (suffix level) | a list, as far as possible | one primary (suffix level) |
| `f x + 1` / `return x + 1` / `await 1s + 1s` | `(f x) + 1` | `return (x + 1)` | `(await 1s) + 1s` |
| `f x, y` | `(f x), y` | `return (x, y)` | n/a (`await(n, P)` is a count) |

So the asymmetry is on both axes: calls and `await` are tight and take
constructors only; statements are loose and take anything.

A name juxtaposed to a name (`f x`) is today a parse error
(`sequence error : expected ';' or new line`), so any rule that only
*adds* juxtapositions of names, numbers, or keywords changes no program
that compiles today. The one exception is `-`: `f -x` is `f - x` today.

## 2. Candidates

**R1: await only.** `await` also accepts a name with fields and indexes
(`await t`, `await t.x`, `await ts@i`), as a value (the same as
`await(t)`). Calls and statements are unchanged.

**R2: Lua extended (tight, as Haskell/ML).** Any primary may start a
juxtaposed argument (names, numbers, `nil`/`true`/`false`, besides the
constructors), at suffix precedence, one at a time:
`f x` is `f(x)`, `f x + 1` is `f(x) + 1`, `f g x` is `f(g)(x)` (as `f
'a' 'b'` is `f('a')('b')` today), `f x, y` is `f(x), y`. `await x`
falls out. Statements stay loose.

**R3: calls are statements (loose, as Ruby/MoonScript).** A callee
followed in the same line by the start of an expression is a call whose
arguments are a list that extends as far as possible, exactly as
`return`: `f x + 1, y` is `f(x + 1, y)`, `print f x` is
`print(f(x))`, `f g x` is `f(g(x))`. `(` right after the callee keeps
the classic call (`f (a+1)*2` is `f(a+1) * 2`, while `return (a+1)*2`
is `return ((a+1)*2)`). `f -x` stays `f - x`. `await` takes its bare
pattern the same way (`await 20min + 1s` becomes `await(20min + 1s)`).

## 3. Measured (local: `exs/`, `tst/`, `lib/`; 970 files that parse)

Method: two parser variants built in a scratchpad (not committed); the
`--ast` of every file compared with today's compiler.

| Rule | files whose parse changes | what |
|---|---|---|
| R1 | 0 | additive; 395 `await(name)` in 253 files could drop the parens |
| R2 | 0 | additive |
| R3 | 2 | `tst/golden/0211-call-call-1.atm` (`f "a" "b"`), `tst/golden/0212-method-7.atm` (`o::f "v" ::f \{}`) |

There are 155 juxtaposed constructor calls in total; under R3, those
followed by a binary operator, a comma, or another argument change
meaning (`type [] == :table`, `f "a" ++ s`, `g(f "a", 1)`).

**Not measured: the ports.** Attaching them (sdl-pingus, sdl-snxkx,
sdl-flare, ray-lessmilk, env-ray, env-pico, doom, sdl-tux, pico-1oom)
was refused in this session. The same diff must run on them before R3
can be adopted.

## 4. Prior art (from the languages' manuals)

| Language | Rule | `f x + 1` | `f x, y` | `f -x` |
|---|---|---|---|---|
| Lua | string or table only | n/a | n/a | `f - x` |
| Haskell/ML | any atom, tightest, curried | `(f x) + 1` | n/a | `f - x` |
| Ruby | any expression, greedy list | `f(x + 1)` | `f(x, y)` | `f(-x)` with a warning |
| MoonScript | any expression, greedy list | `f(x + 1)` | `f(x, y)` | `f(-x)` (space before `-`, none after) |
| CoffeeScript | any expression, greedy list | `f(x + 1)` | `f(x, y)` | `f(-x)`; `f (x)` vs `f(x)` differ: a known pitfall |
| Elixir | any expression, greedy list | `f(x + 1)` | `f(x, y)` | warns; nested `f g x, y` warns, formatter adds parens |
| Nim | "command syntax", greedy | `f(x + 1)` | `f(x, y)` | `f(-x)` |
| Céu / Atmos ref. | `await E`, `emit E` as statements; calls as Lua | | | |

Why C-family languages and Lua avoid `f x`: (1) a juxtaposition that
is an error catches typos (`a b` for `a + b`, a missing `;`); (2)
operators that are both prefix and infix (`f -x`, C's `f *p`) and
brackets that are both call/index and constructor (`f (x)`, `f [x]`)
become ambiguous, or depend on whitespace; (3) where the arguments end
(`g(f x, y)`, `f g x`) needs a rule (currying or greediness), each with
surprises; (4) a name alone must stay the function value, so zero
arguments still need `f()`; (5) Lua has no statement separator, so
`f x` would clash with `f` ⏎ `x = 1`: only a string or a table, which
never start a statement, are safe. Atmos is line-sensitive, so (5) does
not apply to it.

The greedy languages (Ruby, CoffeeScript, Elixir) converged on the same
style: no parentheses at the outermost call of a statement (`print x`,
`puts x, y`), parentheses when nested. Elixir enforces it with
warnings and its formatter.

## 5. Recommendation

R3: one rule for calls, statements and `await` ("a juxtaposed argument
list extends as far as possible"), which answers the author's `f x, y`
and makes `await t` a consequence instead of an exception. Its cost is
the tight binding of Lua constructor calls; locally 2 files, ports to
be measured. If the ports show many breaks, R2 is the fallback (zero
breaks, but `f x + 1` and `return x + 1` keep binding differently).
