# TODO of the session `atmos-iter`

Iterators, loops and functions over iterables. Material:
`261120-iter.md` (from `atmos-lang`). Items are `#N` (standing rule of
2026-11-18): those born here are `atmos-iter#N` from other sessions.
The branch of this session is `iter`.

## Index

| Item | Title | State |
|---|---|---|
| `#1` | Where the functions over iterables live: a library `iter`, globals, or the library of each collection | Decided 2026-11-22: `#1a`, a library `iter` (his answer: "A"); on the branch `iter` |
| `#2` | What an iterable is: the library takes exactly what `loop ... in` takes; whether a user value can say how it is walked | Decided 2026-11-22: `#2a`, and ranges at once (his words: "#2a, mas pq nao aceitar intervalos ja?"); on the branch `iter` |
| `#3` | Which functions (`find`, `filter`, `map`, `any`, `all`, `count`, `fold`, more) | He asked for a minimal functional set and a reference (2026-11-22); `#3d` (`find`, `filter`, `map`) recommended; waiting |
| `#4` | The element of a walk: the first variable of `loop` is the key over a table or a pool and the value over a function or a number; the library has `f(v, k)` | Decided 2026-11-22: `#4b`, `loop k, v` stays as it is (his words: "acredito que a forma atual k,v deve ser mantida pq e unica e causa menos confusao") |
| `#5` | Eager (a new vector) or lazy (an iterator), under the rules of `pin` | Decided 2026-11-22: `#5a` (his answer "A"), as on the branch |
| `#6` | The forms of `loop`: four spellings of a counter, a range as a value, a loop that builds a list | Open |
| `#7` | Changing a table while it is walked: no rule, an element is skipped in silence | Decided 2026-11-22: the body must not change the table it walks; the implementation may assume it |
| `#8` | `val t = await :tick until find(ts, f)` (`atmos-lang#1`) keeps working, or gets another spelling | Open, follows `#1` |
| `#10` | Whether a loop over a table separates the vector part from the other keys (as Lua's `ipairs` and `pairs`), or tables split into two types | His idea on 2026-11-22: `#10d`, the kind fixed when the table is made, errors after; recommended `#10d` after a measurement on the ports; waiting |
| `#11` | The syntax of `#10` | Options sketched on 2026-11-22, recommended `#11a`; after `#10` |
| `#12` | Part 1 of `#10d`: how the kind of a table is known, by the content of `[...]`, by a syntax for every constructor, or by the content with a mark for the empty dictionary | Put to him on 2026-11-22 ("vamos por partes"), recommended `#12b`; waiting |
| `#13` | Part 2 of `#10d`: the mark of `#12b`, and the names of the kinds (`:table.vector`, `:table.dict`, his "table.vector, table.xxx") for `type`, `??` and `match` | After `#12` |
| `#14` | A tagged table walks its `tag` as a key (`:T [10, 20]` gives `0 1 tag`) | Decided 2026-11-22: `#14a` (the walk does not visit `tag`); on the branch `iter` |
| `#15` | Defect: a table changed while walked. Appending to its vector while its other keys are walked visits the same key again and again (an endless loop); removing a key and adding others, which compacts the hash part, skips a key that was not removed | Closed 2026-11-22 by `#7`: a walk over a changed table is undefined, not a defect |
| `#16` | `iter.keys(t)` and `iter.values(t)`, iterators over the keys only and the values only | Decided 2026-11-22: `#16a` (his answer: "a"); on the branch `iter` |
| `#17` | A more primitive form of `tag` (his musing, 2026-11-22): out of the keys, in the header of the table | Moved 2026-11-22 to a session `atmos-tag` (his words: "passe pra sessao atmos-lang e mande ela abrir uma sessao atmos-tag. iremos fazer la"); opened by `atmos-lang` as session_011eECogjajEpy59YjN42Max, material `261122-tag.md`. There he decided `atmos-tag#1b` (2026-11-22, his "B", checked in its transcript): the tag becomes a field of the header of a table, never a key. The skip of `#14a` stays on `iter` until the branch `tag` is merged, then is dropped |
| `#9` | The cost of a call per element, measured on `doom` and `snxkx` | Open, before any merge |

## A. Waiting for the author (one at a time)

- `#3` (2026-11-22). Loops by hand in Pingus and SNKRX, classified by
  regular expressions (approximate): about 33 build a list from each
  member (a `map`), 18 a list of some members (`filter`), 16 sum or
  count, 8 tell whether any member passes (`until ..., true` with `||
  false`), 5 search (`find`), 2 keep the least (`closest_enemy` in
  `snxkx/src/attack.atm:78`). `#3a` add `map`, `any`, `count`
  (recommended); `#3b` also `all`, `fold`, `min`/`max` by a function;
  `#3c` nothing more. All with `f(v, k)` and the collection first; `map`
  gives a vector (`#5a`).
  His answer: "acho que teria que ser um conjunto funcional minimo. tem
  alguma referencia de uma linguagem minima?". References given: Lua
  (none: `pairs`, `ipairs`, `table.*`); R7RS small Scheme (`map`,
  `for-each`, `member`; `filter` and `fold` in SRFI 1); Go 1.21
  `slices` (`IndexFunc`, `ContainsFunc`, `DeleteFunc`, `SortFunc`,
  `MinFunc`, no `Map` or `Filter`); Python (`map`, `filter`, `any`,
  `all`, `sum`, `min`, `max`; `reduce` moved out to `functools` in
  Python 3, as loops read better); Smalltalk-80 (`do:`, `select:`,
  `reject:`, `collect:`, `detect:`, `inject:into:`); in theory `fold`
  alone (Hutton, "A tutorial on the universality and expressiveness of
  fold", 1999). New options: `#3d` `find`, `filter`, `map`: what a loop
  writes badly; `any` is `iter.find(xs, f) != nil`, and what
  accumulates (count, sum, the least) stays a loop with a `var`
  (recommended); `#3e` `fold` alone, the others written with it; the
  earlier `#3a` to `#3c` stand.
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
- `#10d` (his idea, 2026-11-22: "o problema e ter que usar
  iter.vetor/dict em vez de nada... seria possivel detectar o subtipo na
  criacao e dar erro depois?"). Internally a table already keeps the two
  parts apart (`arr`/`an` and `ents`, `rt/tab.c`): no flag, no scan;
  `#t` is `an`. The kind would be a flag set by the first key: a
  constructor with positions makes a vector, one with names a
  dictionary, `[]` waits for its first write (`@0` or `@+` makes a
  vector). A vector refuses a key that is not its next index, a nil
  that is not the last, and keeps its tag aside (`:T [10, 20]` stays a
  vector); `loop v in vs` gives the value. Read in Pingus and SNKRX
  (outside `data/`): 1155 constructors with positions only, 1196 with
  names only, 80 tagged with names, 163 empty, none mixed; 2 loops with
  one variable over a table. Not known: tables made empty and then
  given both kinds of keys at run time, and dictionaries with numbers
  as keys from `0`, which the first write would take for a vector;
  that needs a counting runtime over the suites of the ports
  (`atmos-lang`).
- `#4` (2026-11-22). He wants to keep one table, and asked whether its
  inconveniences can be solved: having to write `iter.vector`/`iter.dict`
  instead of nothing, and `i, v` when only `v` is wanted. `#4a`: with
  one variable a loop gives the element (the value of a table, the task
  of a pool; a function and a number as today), with two the key and
  the value (recommended); `#4b` as today. Read in Pingus and SNKRX:
  2 loops with one variable over something that is not a number or a
  function (`extra.atm:373`, `shop.atm:65`, both numbers). With `#4a`
  and `#14`, a vector with no names walks only its elements, so the
  part selectors are needed only for tables that mix both, which no
  constructor of the two ports does. His names (`iter.idxs`,
  `iter.keys`, `iter.vals`) wait for that: with `#4a`, `iter.vals` is
  `loop v in t`, and the selectors would be views that a loop walks as
  a table (not functions, whose first value would be the key).
- `#4`, his "mas e se quiser percorrer as chaves somente? ainda fica
  confuso" (2026-11-22): `#4a` now includes `iter.keys(t)`, an iterator
  over the keys only (the indexes, then the other keys, in the order of
  the walk), as `keys` in Python and JS: `loop k in iter.keys(d)`. A
  function, so `iter.find(iter.keys(d), f)` works too. In Pingus and
  SNKRX: 195 `loop _, v` (values only), 6 `loop k, _` (keys only, e.g.
  the `Tween` of `snxkx/eng/anim.atm:108`, `worldmap.atm:55` and `:72`).
- The walk of a table (`rt/tab.c`, 2026-11-22), as he asked: the
  vector part (`arr`, `an` = `#t`) from `0` to `#t-1`, skipping holes
  (`nil` in the middle); then the other keys (`ents`), in the order of
  insertion; a removed key leaves a tombstone, skipped, and dropped
  when the part grows (`rehash`, which keeps the order); a key set
  again after removal goes to the end; an index past the end lives
  with the other keys until the gap is filled, then moves into the
  vector (`[0,1]`, `@3`, `.x`, `@2` walks `0 1 2 3 x`); a float with
  an integer value is that integer (`d@(1.0)` is `d@1`); a negative
  index is one of the other keys. The position of a walk is one
  counter over both parts (`pos < an` in the vector, `pos - an` in the
  other keys), which is what breaks in `#15` when `an` changes or the
  other keys are compacted while walked.
- `#13`, his syntax `#[]` and `%[]` (`@[]` refused by him, it reads as
  an index): `#[1,2,3]` is today the length of a constructor (`3`), and
  `x %[1]` is a modulo by a table (an error at run time); neither is in
  the code of Pingus and SNKRX (149 matches, all inside strings). A
  call with no parentheses (`f #[1]`) would need a rule, as `f [1]` has.
- `#12` (2026-11-22, his "diferenciar por sintaxe? table.vector,
  table.xxx? vamos por partes"). `#12a` by the content, `[]` decided
  by its first write; `#12b` by the content, `[]` is an empty vector
  and an empty dictionary has a mark (recommended); `#12c` a syntax for
  every constructor (as in dynamic Ceu, `#[...]` and `@[...]`). Read in
  Pingus and SNKRX: of 85 `x = []` given a name, about 53 are filled
  as vectors (`@+`, `vector.insert`) and about 32 as dictionaries,
  most with strings as keys (caches by name), and `ids@(o.id)` in
  `game.atm:180`, numbers as keys, the case that `#12a` would take
  for a vector. `??` does not know the subtypes of numbers today
  (`1 ?? :number.integer` is false).
- `#11`, after `#10a`: `#11a` two iterators in `iter`, `iter.vector(t)`
  (indexes `0..#t-1`) and `iter.dict(t)` (the other keys, without the
  tag?), native so that they cost no closure (recommended); `#11b` a
  mark in the header of the loop; `#11c` the default walk changes to
  the vector part only (touches the 1490 loops).


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

- `#5` (2026-11-22): `#5a`, a function that gives a collection gives a
  vector; `keys`, `values` and the functions of the user stay lazy.

- `#14` (2026-11-22): `#14a`. On the branch: `a_next` skips the key
  `tag` of a table, so neither `loop` nor `iter` visit it; `t.tag`,
  `??`, `++`, `X` still see it (they walk with `a_tab_next`); no case
  of the suite changed; `make test` with `ATMOS_ASAN=1` passes.
- `#7` (2026-11-22), his words: "o usuario nao deve alterar a tabela
  durante a travessia. a implementacao deve assumir isso se for
  vantajoso". So `#15` is no defect. The manual on the branch says the
  body "must not add or remove keys"; reading his rule as allowing to
  assign an existing key (`loop i, v in t { set t@i = v*2 }`, which the
  implementation walks safely) is the session's, to be confirmed. Pools
  keep their rule (abort and spawn while walked).

- `#16` (2026-11-22): `#16a`, `iter.keys(t)` and `iter.values(t)`. On
  the branch: native iterators over a table or a pool (a pool gives
  positions and tasks), in the order of the walk; they hold the
  collection as a closure would, so they do not escape the block of a
  pinned one (`iter-err-02`); `make test` with `ATMOS_ASAN=1` passes.

- `#4` (2026-11-22): `#4b`, the variables of `loop ... in` keep their
  meaning (one variable over a table is the key). He noted that with
  `#4a` an `iter.values` would be needed too.

- `#2` (2026-11-22): `#2a`, `iter` takes what `loop ... in` takes but
  `nil` (a table, a pool, a number, a function), plus a range `(a,
  b)` as two numbers before the function; a user value is walked by
  giving a function. On the branch: a range spreads into two
  arguments, as in any call (`iter.find((3, 9), f)`, `(1, 4) ->
  iter.filter(f)`); a call that gives two values is cut to its first
  when it is not the last argument, as in Lua, so `r() ->
  iter.find(f)` walks `0..r-1`, not the range.

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

## Work known, not done

- (Closed by `#7`.) `#15`, the fix. The position of a walk must stop depending on `an`
  and on the compaction of the other keys. Easy part: a position that
  says which part it is in (a bit), so that a vector that grows does
  not move the walk of the other keys. Hard part: `rehash` drops the
  tombstones and moves the live keys left, and the walk cannot tell by
  how much; resuming from the last key fails when that key was the one
  removed (the case of `#15`). Candidates: count the walks in progress
  on a table and not compact while there are any; or keep the
  tombstones until the part is twice as large. Not started: the
  promise it keeps (keys added are visited, removed are not, the
  others once) is a rule for `#7`.

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
