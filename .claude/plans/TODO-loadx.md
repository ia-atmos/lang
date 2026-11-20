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
| 6 | A name that uses an earlier name; the errors (undefined, twice, after the value) | asked, recommended: a name sees only the names defined before it; undefined, defined twice, and no final value are errors with the line |
| 7 | Names given from the program: `X.loadx(s, [TITLE=...])` | to ask |
| 8 | The spread in the language too (`[t...]`), with `atmos-args` | closed with #4: nothing for the language |
| 9 | `X.dump` of a value read with names: written expanded | to confirm |
