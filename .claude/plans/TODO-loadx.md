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
| 1 | One function or two: `X.load` extended, or a new `X.loadx` | asked, recommended: a new `X.loadx`, `X.load` unchanged |
| 2 | The syntax of a definition, and what marks the final value | to ask |
| 3 | A name as a value: a copy at each use, no shared node | to ask |
| 4 | The spread `NAME...`: where it may appear, and its positional entries | to ask |
| 5 | A field in conflict: the later wins; shallow replace or deep merge | to ask |
| 6 | A name that uses an earlier name; the errors (undefined, twice, after the value) | to ask |
| 7 | Names given from the program: `X.loadx(s, [TITLE=...])` | to ask |
| 8 | The spread in the language too (`[t...]`), with `atmos-args` | to ask |
| 9 | `X.dump` of a value read with names: written expanded | to confirm |
