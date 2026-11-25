# TODO: session `atmos-slices`

Slices of vectors with no copy (a view), on the branch `slices` of this
repository, cut from `origin/main` at `afe7c43`. Material:
`261122-slices.md`. Items are `#N`, in order, never reused.

Last update: 2026-11-24.

Coordination, status and questions for the author go to `atmos`
(session_011eECohhJbc4GMpAj3yyeEo, since 2026-11-24, `atmos-lang#119`);
measurements on the ports and merges into `main` stay with `atmos-lang`
(session_011eECogKVQ7ZzmWCoMST2wK). The author paused new work until
next week's credits (2026-11-23): this session waits for his answer to
#1.

## Index of the items

| # | Title | State |
|---|---|---|
| #1 | What a view is to the program: its own kind of value, taken by the operations that read a vector, or a vector everywhere (same `type`, every table operation checks) | asked, 2026-11-22; recommended #1a, its own kind. Prototyped on `slices` with #1a, by the session, to be confirmed (2026-11-25, under the rule "Work with no input", `#95`, after the pause ended) |
| #2 | Writes: `set v@k = x` writes the base; `set v@+` and a `set` beyond the length are errors | by the session, to be confirmed |
| #3 | A change of the length of the base: the view keeps its positions (`off`, `cap`), and `#v` is what the base still has of them; a hole is skipped by `loop`, as in a table | by the session, to be confirmed |
| #4 | Life: a view holds its base as a closure holds a capture (by its count; it does not leave the block of a pinned base) | by the session, to be confirmed |
| #5 | Form: `vector.view(t [, i [, j]])`, the indexes of `string.sub` (inclusive, negative from the end, clamped); no syntax | by the session, to be confirmed |
| #6 | Vectors only, no views of strings | by the session, to be confirmed |
| #7 | A view of a view is a view of the same base (offsets added, one level) | by the session, to be confirmed |
| #8 | No copy function beside it: `v ++ []` copies | by the session, to be confirmed |

## The prototype (branch `slices`)

- `rt/atmos.h`, `rt/val.c`: the type `A_VIEW` (`type(v)` is `view`), with
  the layout of a function of three captures (the base, `off`, `cap`), so
  that every walk of the rules of `pin` (pin, adopt, transfer, finalize,
  free) treats it as a closure over its base (`rt/task.c`, the cases of
  `A_FUNC` take `A_VIEW`). `a_view`, `a_view_len`, `a_view_tab`.
- `rt/tab.c`: `a_index`, `a_setindex`, `a_len`, `@#`, `@+` on a view;
  `rt/ops.c`: `a_next` (loop), `a_cat` (`++` copies); `rt/std.c`: the
  functions of `iter` take a view; `rt/lvector.c`: `vector.view`, and
  `vector.join` takes a view.
- Tests `view-01` to `03`, `view-err-01` to `03`. Manual: "Types & Values",
  "Vector".
- Not done: `X.dump` of a view is an error (`invalid dump : unexpected
  view`); deep equality and membership of `X` do not look into a view;
  the C API has no accessor besides the macros.
