# TODO of the session `atmos-loadx`

A second loader of the library `X`, `loadx`: data with definitions of
names and uses of them. Material: `261119-loadx.md`. Implementation on
the branch `loadx`; nothing is implemented before the author approves
the design.

## Checked on 2026-11-19 (main at `26cbf3a`)

- A bare name is an error in `X.load`, as a value, as the value of a
  field, with `...`, and as a definition before the value: `invalid
  load : line 1 : unexpected name : expected value`.
- A field written twice loads today with no error, and the later one
  wins: `[x=1, x=2]` is `[x=2]`. The same for `[1, @0=5]`.

## Index of items

| # | Title | State |
|---|-------|-------|
| 1 | One function or two: `X.load` extended, or a new `X.loadx` | **decided (2026-11-20, #1a)**: a new `X.loadx`, a superset of `X.load`, which stays as it is |
| 2 | The syntax of a definition, and what marks the final value | **decided (2026-11-20, #2a)**: `ID = Value` at the top level, no keyword; the one value with no name is the result, and only the end of the text may follow it; a name is any identifier but `nil`, `true`, `false` |
| 3 | A name as a value: a copy at each use, no shared node | **decided (2026-11-20, #3a)**: a copy at each use, so the result is a tree; a name may hold any value, not only a table |
| 4 | The spread `NAME...`: where it may appear, and its positional entries | **closed (2026-11-20): no spread.** The author said the `...` of his sketch (`[X...]`) was an ellipsis, a hidden continuation, not an operator. A name is only used as a value (#3) |
| 5 | A field in conflict: the later wins; shallow replace or deep merge | closed with #4: no spread, so nothing to merge. A field written twice stays as in `X.load` (the later wins) |
| 6 | A name that uses an earlier name; the errors (undefined, twice, after the value) | **decided (2026-11-20, #6a)**: a name sees only the names defined before it (so no recursion); errors with the line: `undefined name : A`, `name already defined : A`, and `expected value` when there is no final value |
| 7 | Names given from the program: `X.loadx(s, [TITLE=...])` | **decided (2026-11-20, #7a)**: not now; `X.loadx(s)` takes the text only |
| 8 | The spread in the language too (`[t...]`), with `atmos-args` | closed with #4: nothing for the language |
| 9 | `X.dump` of a value read with names: written expanded | approved with #10 |
| 10 | Approval of the whole design, to implement on the branch `loadx` | **approved (2026-11-20)**; implemented on the branch `loadx` |
| 11 | No limit on the expanded size (`A=[1,1]`, `B=[A,A]`, ...): `X.loadx` is for texts the program trusts, `X.load` for the others | approved with #10 |

## The design, as approved in #10

```
Text  : { ID `=´ Value } Value
Value : (as in X.load) | ID
```

- `X.loadx(s)`: a new function; `X.load` does not change. Every text
  that `X.load` reads, `X.loadx` reads to the same value.
- A definition is `ID = Value` at the top level; the one value with no
  name is the result, and only the end of the text follows it.
- A name is used as a value only (a positional entry, the value of a
  field, the value of a key, the result itself). There is no spread.
- Each use is a deep copy: the result is a tree, equal to what `X.load`
  gives for the text with the names replaced by their values.
- A name sees the names defined before it. Errors, with the line:
  `undefined name : A`, `name already defined : A`, `expected value`.
- `X.dump` does not change: a value read with names is written expanded.
| 12 | Merge of the branch `loadx` into `main` | **approved and done (2026-11-20)**, after the suite passed with ASAN over the current `main` |
| 13 | An `include` in the text of `X.loadx` (asked by the author: "faz sentido um include?") | **decided (2026-11-20, #13b)**: an `include "file"` in the text. Its details are #14 on; nothing is implemented before they are settled |
| 14 | The include: relative to what, since `X.loadx` receives a text and not a file | **decided (2026-11-20)**: first #14a, then the author went back to #14b with a change: the directory of the includes is an argument of `X.loadx`, and it is the FIRST one, `X.loadx(dir, s)`. No function that receives a path |
| 15 | The name of the function that receives the path | closed with #14: no new function |
| 16 | `X.loadx` with one argument, now that the directory comes first | **decided (2026-11-20, #16a with a change)**: `X.loadx(s)` stays valid, and its includes are relative to the current directory, as `io.open` |
| 17 | What an include is: where it may appear, what the included file holds, a file included twice, a nested include | asked, recommended: at the top level before the value; the file holds definitions only; once per file, as in the language; a nested include is relative to the file that has it |

## State (2026-11-20)

- Merged into `main`. The branch `loadx` had three commits: `rt/lload.c` (the loader),
  `tst/cases/loadx-01` (with `inc/slides.atm`), and the manual ("Load
  with Names") with one line in the README.
- `make test` with ASAN passes on the branch: rt, lex 32, ast 90, cases
  422, golden 443, env 1, no failure.
- `X.load` reads and rejects the same texts as before (its test did not
  change), so no measurement on the ports was asked.
- A value nested through names obeys the same limit of 200 nested
  tables (`too many nested tables`).
- `pico-slides` was told the final syntax and semantics, and that there
  is no spread.
