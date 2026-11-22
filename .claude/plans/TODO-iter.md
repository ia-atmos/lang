# TODO of the session `atmos-iter`

Iterators, loops and functions over iterables. Material:
`261120-iter.md` (from `atmos-lang`). Items are `#N` (standing rule of
2026-11-18): those born here are `atmos-iter#N` from other sessions.
The branch of this session is `iter`.

## Index

| Item | Title | State |
|---|---|---|
| `#1` | Where the functions over iterables live: a library `iter`, globals, or the library of each collection | Decided 2026-11-22: `#1a`, a library `iter` (his answer: "A"); on the branch `iter` |
| `#2` | What an iterable is: the library takes exactly what `loop ... in` takes; whether a user value can say how it is walked | Put to him on 2026-11-22, recommended `#2a`; `#2a` on the branch `iter`, by the session, to be confirmed |
| `#3` | Which functions (`find`, `filter`, `map`, `any`, `all`, `count`, `fold`, more) | Open |
| `#4` | The element of a walk: the first variable of `loop` is the key over a table or a pool and the value over a function or a number; the library has `f(v, k)` | Remark put to him on 2026-11-20; decision open |
| `#5` | Eager (a new vector) or lazy (an iterator), under the rules of `pin` | Open |
| `#6` | The forms of `loop`: four spellings of a counter, a range as a value, a loop that builds a list | Open |
| `#7` | Changing a table while it is walked: no rule, an element is skipped in silence | Remark put to him on 2026-11-20; decision open |
| `#8` | `val t = await :tick until find(ts, f)` (`atmos-lang#1`) keeps working, or gets another spelling | Open, follows `#1` |
| `#10` | Whether a loop over a table separates the vector part from the other keys (as Lua's `ipairs` and `pairs`), or tables split into two types | Put to him on 2026-11-22 (his question, before `#2`), recommended `#10a`; waiting |
| `#11` | The syntax of `#10` | Options sketched on 2026-11-22, recommended `#11a`; after `#10` |
| `#9` | The cost of a call per element, measured on `doom` and `snxkx` | Open, before any merge |

## A. Waiting for the author (one at a time)

- `#10` (asked by him before `#2`). Options: `#10a` one type of table,
  and a loop that can say which part it walks (recommended); `#10b`
  two types, vector and dictionary; `#10c` nothing changes. Facts run
  on `main` (2026-11-22): the walk is the vector part, then the other
  keys in the order of insertion (a key removed and set again goes to
  the end), so the order is fixed, unlike Lua's `pairs`; a tagged
  table gives its tag as a key (`:T [10, 20]` walks `0 1 tag`); after
  `set w@1 = nil` in `[1,2,3]`, `#w` is `3` and the walk gives two
  elements; `set u@5` on a vector of two is a key of the other part,
  walked after the vector; with one variable the loop gives the key.
  In the ports (Pingus, SNKRX) the names say the intent: 22 `loop i,
  v` and 15 `loop k, v`; 195 `loop _, v`, mostly over vectors.
- `#11`, after `#10a`: `#11a` two iterators in `iter`, `iter.vector(t)`
  (indexes `0..#t-1`) and `iter.dict(t)` (the other keys, without the
  tag?), native so that they cost no closure (recommended); `#11b` a
  mark in the header of the loop; `#11c` the default walk changes to
  the vector part only (touches the 1490 loops).

- `#2`. Options: `#2a` what `loop ... in` walks but `nil` (a table,
  a pool, a number, a function), and a user value is walked by giving
  a function (recommended); `#2b` tables and pools only; `#2c` as
  `#2a`, and a user value may also say how it is walked (a field).

## Settled by the session, to be confirmed (rule `atmos-lang#95`)

On the branch `iter`, reversible by not merging:

- `#1a` (2026-11-21): the commits of `atmos-pool` on its branch
  `pool-find` (`find`, `filter`, their cases, the manual), taken as
  they were, then moved into a table `iter`: `iter.find(xs, f)`,
  `iter.filter(xs, f)`, still over a table or a pool only, with
  `f(v, k)`. The manual has a section "Iter"; the await over a pool
  reads `await :tick until iter.find(ts, f)`. A case chains them with
  `->`. `make test` with `ATMOS_ASAN=1` passes (`tst/run.sh`
  pass=1015). No port names a variable `iter` (Pingus, SNKRX read).
  Not measured on the ports yet: nothing in them calls it.
- 2026-11-21: `main` at `d19d40f` (the branch `float`) brought into
  `iter` (`535efb0`) with no conflict; `make test` with `ATMOS_ASAN=1`
  passes. The prototype has no word "real" to change.

- `#2a` (2026-11-22): `iter.find` and `iter.filter` also take a number
  (`v` from `0` to `n-1`, `k` nil) and a function (`v`, `k` the first
  two values of each call), through the same `a_next` as `loop`;
  `nil` is left out (a walk with no end). Case `iter-01`; `make test`
  with `ATMOS_ASAN=1` passes; branch at `6bcd61d`.

## D. Decided by the author

- `#1` (2026-11-22): `#1a`, the functions over iterables live in a
  library `iter`, the collection first.

## Received

- `atmos-pool` (2026-11-20): the branch `pool-find` with only `find`
  and `filter`; the branch `pool` no longer has them, so
  `atmos-lang#1` now depends on this session. Its notes: `find` must
  give the member (5 of the 6 cases of Pingus need it); `f(v, k)`
  because predicates ignore the key; the key returned second because
  Lessmilk and SNKRX remove the found entity by key; `filter` drops the
  keys, which loses names in a table; SNKRX, Flare and Lessmilk keep
  entities in tables, not pools; `await :tick until loop _, t in ts {
  until f(t), t }` already works with no library.

## B. What was run (2026-11-20, `main` at `8658dae`)

- Every form of `loop`, a search by hand, lambdas through `->` and
  `-->`: as the manual says.
- `loop v in t` with one variable gives the key (`0 1 2 x y`); over a
  pool, the index; over a function, the value; over a number, the
  number. So `loop k, v in 4` gives `v == nil`, and a library written
  in Atmos over `loop k, v in xs` works for a table and a pool and
  fails for a number and a function (`#4`, `#2`).
- `x -> f(y)` puts `x` first and `f(y) <-- x` puts it last: a library
  with the collection first chains with `->` (`xs -> iter.filter(f) ->
  iter.map(g)` ran, with a table `iter` written in Atmos).
- `vector.remove(t, i)` inside `loop i, v in t`: the element that
  follows is not visited, with no error (`#7`).
- A string is not iterable (`invalid loop : unexpected string`).

## C. What was read in the ports (sdl-pingus, snxkx)

- Loop headers outside `data/`: 389 `loop ... in`; 195 are `loop _, v
  in`, 31 name both variables, 6 are `loop k, _ in`; 157 have one
  variable, 125 of them over a number.
- Searches by hand in Pingus: `game.atm:275` (the first pingu that can
  be a bomber), `zone.atm:11`, `extra.atm:159`, `extra.atm:232`,
  `smasher.atm:9` (any pingu in a zone, with `, true` and `|| false`),
  `options.atm:82` (the index of a choice), `worldmap.atm:55`.
- Filters by hand: `snxkx/src/attack.atm:67` (`enemies_in_circle`),
  `game.atm:152` (the actions of a level), `levels.atm:102`;
  `snxkx/src/info.atm:41` (remove a member by identity, loop and
  `break`).
