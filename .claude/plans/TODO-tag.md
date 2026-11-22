# TODO of the session `atmos-tag`

Whether the tag of a table gets a more primitive form (`atmos-iter#17`,
moved here on 2026-11-22). Material: `261122-tag.md` (from
`atmos-lang`). Items are `#N`; those born here are `atmos-tag#N` from
other sessions. The branch of this session is `tag`.

## Index

| Item | Title | State |
|---|---|---|
| `#1` | Where the tag lives: a plain key `"tag"` (today, with the skip of `atmos-iter#14a`) or a field in the header of `ATab`, out of the keys | Decided 2026-11-22: `#1b` (his answer: "B") |
| `#2` | How a program reaches the tag: `t.tag` and `t@"tag"` both reach the header, only `t.tag`, or a new form | Put to him on 2026-11-22, recommended `#2a`; waiting |

## A. Waiting for the author (one at a time)

- `#2` (2026-11-22). `#2a` the name `tag` is reserved in a table:
  `t.tag`, `t@"tag"`, `set t.tag = :Q` and `[tag=:T]` all reach the
  header (no change in the ports; `a_index` compares the key with
  `"tag"`). `#2b` only the static `t.tag` reaches it (the compiler
  lowers it); `t@"tag"` is a plain key (0 uses in the ports), so
  `t.tag` and `t@"tag"` differ. `#2c` a new form (`tag(t)`), `t.tag`
  becomes a plain key; about 104 reads and 3 writes to migrate.

## D. Decided by the author

- `#1` (2026-11-22): `#1b`, the tag is a field in the header of
  `ATab`, never a key.

## Later (not yet put to him)

- Whether a tag can be removed (`set t.tag = nil`).
- What `[tag=:T, x=1]` becomes (about 21 in the ports).
- Whether a tag is its own type at run time (today a string).
- `X.load` / `X.dump` with a field `tag`.
- `atmos-iter` told on 2026-11-22 that `#1b` is decided; it drops the
  skip of `#14a` when `tag` merges.
