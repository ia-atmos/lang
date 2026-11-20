# TODO of the session `atmos-iter`

Iterators, loops and functions over iterables. Material:
`261120-iter.md` (from `atmos-lang`). Items are `#N` (standing rule of
2026-11-18): those born here are `atmos-iter#N` from other sessions.
The branch of this session is `iter` (not created yet: nothing is
implemented).

## Index

| Item | Title | State |
|---|---|---|
| `#1` | Where the functions over iterables live: a library `iter`, globals, or the library of each collection | Put to him on 2026-11-20, recommended `#1a` (a library `iter`); waiting |
| `#2` | What an iterable is: the library takes exactly what `loop ... in` takes; whether a user value can say how it is walked | Open, after `#1` |
| `#3` | Which functions (`find`, `filter`, `map`, `any`, `all`, `count`, `fold`, more) | Open |
| `#4` | The element of a walk: the first variable of `loop` is the key over a table or a pool and the value over a function or a number; the library has `f(v, k)` | Remark put to him on 2026-11-20; decision open |
| `#5` | Eager (a new vector) or lazy (an iterator), under the rules of `pin` | Open |
| `#6` | The forms of `loop`: four spellings of a counter, a range as a value, a loop that builds a list | Open |
| `#7` | Changing a table while it is walked: no rule, an element is skipped in silence | Remark put to him on 2026-11-20; decision open |
| `#8` | `val t = await :tick until find(ts, f)` (`atmos-lang#1`) keeps working, or gets another spelling | Open, follows `#1` |
| `#9` | The cost of a call per element, measured on `doom` and `snxkx` | Open, before any merge |

## A. Waiting for the author (one at a time)

- `#1`. Options: `#1a` a library `iter` (`iter.find(xs, f)`,
  recommended); `#1b` globals, as on the branch `pool`; `#1c` the
  library of each collection (`vector.filter`, `tasks.find`).

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
