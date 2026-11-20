# atmos-oo: clauses of a function, chosen by the tags of its arguments

A prototype, written on 2026-11-20 by the session `atmos-oo` after the
author said "ok va em frente" to the sketch discussed with him (a tag
after a parameter, one or more parameters, the most specific clause
wins, no grouping and no order). It is on the branch `oo` of
`ia-atmos/lang`, not on main: a change of the language waits for his
approval (item #7 of `TODO-oo.md`).

## 1. What it is

```
func area (s :Shape)        { 0 }
func area (s :Shape.Circle) { 3 * s.r * s.r }
func area (s :Shape.Rect)   { s.w * s.h }

area(:Shape.Circle [r=1])               ;; --> 3
area(:Shape.Line [])                    ;; --> 0   (the parent)

func hit (a :Ship,   b :Rock)   { "ship-rock" }
func hit (a :Entity, b :Entity) { "entities" }
func hit (a, b)                 { "default" }

task Act (p :walker) { ... }            ;; tasks too
task Act (p :faller) { ... }
await Act(p)                            ;; spawns the one of the tag of `p`
```

## 2. The rules, as implemented

| Point | Rule |
|---|---|
| Declaration | `func f (...)`, `task f (...)`, `func m.f (...)` with a tag after one or more parameters adds a clause to the name, instead of replacing its value |
| When a clause applies | each tagged parameter matches its argument as `arg ?? :Tag`: a tagged table by its tag, a tag or string by itself, any value by the name of its type (`:number`, `:table`) |
| Which one is chosen | the most specific, comparing the parameters from the left: a longer tag, then a shorter one, then the name of a type, then no tag |
| Order and grouping | none: clauses may be anywhere, in any order, in any file; a clause must only have been declared before the call that needs it |
| A parameter with no tag | accepts any value |
| A declaration with no tag at all | if the name already has clauses in the program text before it, it is their default clause; otherwise it is a plain function, as today |
| A plain function that then gets a clause | becomes the default clause |
| The same tags twice | error: `invalid clause : 'f' already has it` |
| No clause applies | error: `invalid call : no clause of 'area' for (Shape.Line, number)` |
| Tasks | `spawn T(...)`, `spawn @pool T(...)`, `await T(...)` choose by the arguments |
| Where | named prototypes only; a tag in `val func f`, in `func (...)` or in a lambda is a compile error (`invalid parameter : unexpected tag`) |
| The value | `type(f)` is `function`; it is immutable: a new clause makes a new function, assigned to the name |

## 3. How it is done

- Parser (`src/parser.c`): `func f (x :T, y) {...}` becomes
  `set f = $clause(f, "f", func (x, y) {...}, :T, nil)`. `$clause` is a
  name that only the compiler can write. Nothing else in the compiler
  changes: no new node, the resolver and the coder see a call.
- Runtime (`rt/std.c`): a function with clauses is an ordinary function
  value whose captures hold its clauses and their tags. A call scans
  them once. `rt/task.c`: a spawn resolves such a function to the task
  of the chosen clause.
- Size: 344 lines added in `src/` and `rt/`, 10 changed.
- A defect met on the way, fixed on the branch: an `await T(...)` whose
  spawn fails before the task exists left the pattern marked as owning
  a task (`patt_stop` crashed). On main it cannot be reached; the
  clauses reach it.

## 4. Tests

`tst/cases/clause-01` to `-03` (one and many arguments, types, fields,
tasks, pools, defaults, recursion, closures) and `clause-err-01` to
`-05`. The full suite on the branch: `tst/run.sh` pass=1012 fail=0
under the address sanitizer, `make test-rt test-env` pass.

## 5. Cost

3,000,000 calls of `area(r)` on a `:Shape.Rect`, `-O2`, best of three:

| Form | Time | Per call |
|---|---|---|
| A plain function | 0.163 s | 54 ns |
| A table by tag, `AREA@(s.tag)(s)` | 0.266 s | 89 ns |
| A `match` with 3 arms inside the function | 0.371 s | 124 ns |
| 3 clauses | 0.403 s | 134 ns |

A function with no clause pays nothing. The choice is linear in the
number of clauses, with no cache yet.

## 6. Measured on SuperTux: `collision` of `badguy.atm`

The function with the most tests of `cls.kind` in the ports (10, and
it also tests the kind of the other object: a double dispatch by
hand). In `261118-oo-tux/`: `a.atm` is the function of the port
(sdl-tux `dcd65d8`) with its world as stubs that log; `b.atm` is the
same with clauses `hit (s :Bad..., o :Bad|:Player, bits, w)`; `run.atm`
calls both with every combination (14 badguys, 7 others, 4 worlds, 4
sides, 3 states).

**Result**: 4704 combinations, the same return and the same actions
in both.

| | A (the port) | B (clauses) |
|---|---|---|
| Lines of code | 116 | 106 |
| Tests of the kind | 10 | 3, in the function that holds what is common to every pair |
| `return` | 24 | 5 |
| Deepest nesting | 7 | 3 |
| A new kind of badguy | edits the function in up to 3 places | adds its clauses |
| Units | 1 function | 12 clauses, 5 helpers |

What the rewrite showed:

- The pair (badguy, other) falls into clauses naturally:
  `hit (s :Bad.Stalactite, o :Bad.Stalactite)` replaces a test in the
  middle of an `ifs`.
- **The call to the parent is missing.** The ice block, while
  `:normal`, behaves as a walking badguy. With no form to call the
  clause of `:Bad.Walker` on a `:Bad.Walker.Ice`, the body of the parent
  had to be a named function (`walker_bad`) that both clauses call. It
  works, and it is the one thing the syntax does not say.
- What is common to every pair (grabbed, inactive, the stone of the
  player) stays in a plain function before the dispatch. In the C++ it
  is the method of the base class.
- The methods here take the badguy as a table (`s`). In the port they
  are closures over the task; to use clauses, `badguy.atm` would pass
  its state (`b`, `pub`, `cls`) as one tagged table. That is the real
  cost for a port, not the dispatch.

## 7. Open

- A form to call the parent clause (`super(...)`, or the clause of a
  given tag).
- Clauses of local names (`val func`), and anonymous ones.
- A cache of the choice per combination of tags.
- The manual: nothing written yet ("Prototypes", "Calls", the grammar).
- The grammar next to `atmos-args`: `x :T` is read only inside the
  parentheses of a declaration; a call `f x :T` is not touched. To
  confirm with that session.
- Ports: no program can contain `x :T` in a parameter today (it was a
  syntax error), and a name gets clauses only by that syntax, so no
  existing program should change. To be measured by `atmos-lang`.
