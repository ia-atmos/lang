# TODO of the session `atmos-tag`

Whether the tag of a table gets a more primitive form (`atmos-iter#17`,
moved here on 2026-11-22). Material: `261122-tag.md` (from
`atmos-lang`). Items are `#N`; those born here are `atmos-tag#N` from
other sessions. The branch of this session is `tag-rebased` (the author, 2026-11-22: the remote `tag` keeps an old base and is not used).

## Index

| Item | Title | State |
|---|---|---|
| `#1` | Where the tag lives: a plain key `"tag"` (today, with the skip of `atmos-iter#14a`) or a field in the header of `ATab`, out of the keys | Decided 2026-11-22: `#1b` (his answer: "B") |
| `#2` | How a program reaches the tag: `t.tag` and `t@"tag"` both reach the header, only `t.tag`, or a new form | Decided 2026-11-22: `#2a` (his answer: "A"); on the branch `tag-rebased` |
| `#3` | What a tag may hold: a string or nil only (the prototype), or any value | Put to him on 2026-11-22, recommended `#3a` |

## A. Waiting for the author (one at a time)

- `#3` (2026-11-22). `#3a` a tag is a string or nil; `set t.tag = 10`
  and `X.load("[tag=[]]")` are errors; `nil` removes the tag (the
  prototype). `#3b` any value, as a key holds today: then the walks
  of pin and of the cycles (`rt/task.c`, nine of them) must also visit
  the header, or a table held only as a tag escapes its block. In the
  ports every tag written is a tag or the tag of another table.

## Settled by the session, to be confirmed

On the branch `tag-rebased`, reversible by not merging:

- `#1b` + `#2a` (2026-11-22): `ATab` has a field `tag`; `a_tab_get`
  and `a_tab_set` send the string `"tag"` to it, so `t.tag`,
  `t@("tag")`, `[tag=...]`, `:T [...]` and `X.load` reach it and no
  walk sees it (`next(t,"tag")` is an unknown key). `??` over a table
  and `toggle_evt` read the field with no lookup. A pattern with a tag
  (`a_is`, `a_gte`) checks it apart. `X.copy` copies it; `a ++ b`
  takes the tag of `b`, else of `a` (as the keys did). `X.dump` writes
  a tag that is not an identifier as the first field
  (`[tag="not a tag!", 1]`, was last; `dump-01.out` changed). Manual:
  a paragraph in "Hierarchical Tags". Cases `tag-01`, `tag-err-01`,
  `tag-err-02`.
- `#3a` in the prototype, pending his answer.

## D. Decided by the author

- `#1` (2026-11-22): `#1b`, the tag is a field in the header of
  `ATab`, never a key.
- `#2` (2026-11-22): `#2a`, the name `tag` is reserved: `t.tag`,
  `t@("tag")`, `set t.tag` and `[tag=...]` all reach the header.

## Later (not yet put to him)

- Whether a tag is its own type at run time (today a string).
- `atmos-iter` told on 2026-11-22 that `#1b` is decided; it drops the
  skip of `#14a` when `tag` merges.
