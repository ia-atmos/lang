# TODO: session `atmos-slices`

Slices of vectors with no copy (a view), on the branch `slices` of this
repository, cut from `origin/main` at `afe7c43`. Material:
`261122-slices.md`. Items are `#N`, in order, never reused.

Last update: 2026-11-22.

## Index of the items

| # | Title | State |
|---|---|---|
| #1 | What a view is to the program: its own kind of value, taken by the operations that read a vector, or a vector everywhere (same `type`, every table operation checks) | asked, 2026-11-22; recommended: its own kind |

## Choices still ahead (not yet asked)

- Read-only or writable (writes go to the base); append to a view.
- A change of the length of the base (append, insert, remove, clear,
  sort): the view keeps `(base, off, len)` and reads what is there now,
  or is invalidated, or the base is frozen while a view lives.
- Life: the view holds its base by a count; under `pin`, it is a
  reference into the base and does not leave the block of a pinned
  base (as a closure or an iterator).
- Form: `vector.view(t, i, j)` and/or a syntax; inclusive bounds as
  `string.sub` and `loop`; negative indexes.
- Strings too, or vectors only.
- A view of a view (one level, offsets added).
- A copy beside the view (`vector.sub`, `atmos-lang#116a`).
