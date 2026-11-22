# TODO of the session `atmos-tag`

Whether the tag of a table gets a more primitive form (`atmos-iter#17`,
moved here on 2026-11-22). Material: `261122-tag.md` (from
`atmos-lang`). Items are `#N`; those born here are `atmos-tag#N` from
other sessions. The branch of this session is `tag`.

## Index

| Item | Title | State |
|---|---|---|
| `#1` | Where the tag lives: a plain key `"tag"` (today, with the skip of `atmos-iter#14a`) or a field in the header of `ATab`, out of the keys | Put to him on 2026-11-22, recommended `#1b`; waiting |

## A. Waiting for the author (one at a time)

- `#1` (2026-11-22). `#1a` the tag stays the key `"tag"`; the walk
  skips it (`atmos-iter#14a`), `#`, `++`, `X.copy`, `??` and `match`
  see it as a key. `#1b` a field `tag` in the header of `ATab`
  (`rt/atmos.h:541`), never a key: no walk, copy or length sees it,
  `??` and `match` read it with no lookup, a tagged vector stays a
  vector (`atmos-iter#12`). Cost of `#1b`: one `AVal` per table; the
  six `a_strz("tag")` of the runtime change (`tab.c:436`, `ops.c:66`,
  `std.c:381`, `lload.c:508, 949`, `task.c:1582`). How a program
  reaches it (`t.tag`, a new form) is the next item, not this one.

## Later (not yet put to him)

- How a program reads and writes it (`t.tag` kept, `tag(t)`, both;
  `set t.tag`; removal).
- What `[tag=:T, x=1]` becomes (about 21 in the ports).
- Whether a tag is its own type at run time (today a string).
- `X.load` / `X.dump` with a field `tag`.
- Tell `atmos-iter` to drop the skip of `#14a` if `#1b`.
