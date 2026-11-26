# TODO-ppp: the tip markers `@#` `@+` and `+=` on vectors

Session `atmos-ppp`, opened 2026-11-23. Material: `261123-ppp.md`.
Branch `ppp` (from origin/main `9863fd2`; main `31881e0` merged in 2026-11-25).
Since 2026-11-24 coordination goes through `atmos` (session_011eECohhJbc4GMpAj3yyeEo).

| # | Item | State |
|---|---|---|
| #1 | `+=` on vectors: append in place (`set v += x` pushes `x`)? | decided by the author, 2026-11-25 (in `atmos`, checked in its transcript): "atmos-ppp#1: nao fazer como recomendacao dele". No `+=` on vectors; `set v@+ = x` stays the append |
| #5 | `set :out` at the end of the value: undo (nil) or keep the dead value | superseded by #6 |
| #6 | `set :out` on an index when the value ends: refuse, nil, error, or `:dead` | decided by the author 2026-11-26: nil (option 2), as today |
| #7 | A test of a dead value: type, function, or rule of `??` | moved to `atmos-pin` by the author, 2026-11-26 ("Quem deve assumir esses casos é a sessão atmos-pin. Mande pra lá."), with #5 and #6; sent with the options |
| #2 | Keep, change or drop `@#` / `@+` | confirmed by the author 2026-11-26 ("#2 e #4: ok"): keep both as they are. `@+` is the append (359 uses, #1); `@#` is the read side of it (peek, 7 uses) and `set v@# = nil` is the pop; dropping `@#` saves nothing and breaks 7 lines. No code change |
| #3 | `++=` on vectors in place (extend, no copy, works on `pin`) | superseded by #3a (the author leans to in place). Was: no change. 0 uses; `set x OP= e` = `set x = x OP (e)` stays one rule for every operator; in place would make an alias see the change (checked: `var w=[1]; val al=w; set w ++= [2]` gives `#w=2, #al=1` today) and would also have to define in-place merge for tables with keys. No code change |
| #3a | MERGE APPROVED by the author 2026-11-26 ("#3a: merge"); handed to `atmos-lang`. `++=` in place: on a table, the entries of `e` go into `x` itself, as `++` would put them, each stored as by `set x@k = v` (so `pin` and the ceiling apply); on a string or number, rebind as today | approved by the author 2026-11-25, with no special case for `set v ++= v` ("O caso de copiar v parece muito específico. Acho que não deve tratar especial"). Prototype on `ppp`: `a_cat_set` (rt/ops.c) reads the entries of `e` before storing, which makes `v ++= v` work with no special case; `++=` is accepted on a name that cannot be reassigned (`pin`, `val`) and is a run-time error there if the value is not a table. Tests `cset-05`, `cset-err-05`, `cset-err-06`; manual "Set" and "Concatenation". Ports: 0 uses of `++=`, nothing breaks |
| #4 | Pop and peek as values (marker vs `vector.remove`) | confirmed by the author 2026-11-26 ("#2 e #4: ok"): no new form. Peek is `v@#`; pop as a value is `vector.remove(v)`; pop as a statement is `set v@# = nil` (on an empty vector it does nothing: checked). No code change |

## :out and `@+` (atmos-pin#21)

The author asked (2026-11-25) what happens to `set :out v@+ = o` when
`o` ends. Today the runtime records each `set :out` place (table and key)
with its value and writes `nil` there when the value ends; names that
kept the value see a dead task, or a finalized table whose use is an
error. For `v@+` the place is the index `#v` at the time of the set.
Options given: (a) nil at that index (a hole), (b) remove it and shift
the others, (c) leave the dead value there. Recommended (b); the rule is
`atmos-pin`'s.

#5 (asked 2026-11-25): at the end of the value, undo the `set :out`
(today: nil) or keep the dead value in the place? Recommended: undo.
Checked on `ppp`: there is no `:dead` type; a task that ended keeps
`type` `xtask` and `atmos.status(t)` gives `dead`; a finalized table
keeps `type` `table`, `==` works, and any index, length or iteration
raises `invalid access : finalized table pinned at ...`, with no way to
test it before use. Keeping it would break the stable rule "a place
never refers to a finalized value" and every `if place` test in the
ports that use `set :out` (count to be asked of `atmos-lang`).

2026-11-26, the author: "Outra opção seria recusar o :out. Temos usos?"
Read as: keep refusing `set :out` on `@+` (today's compile error), and
maybe on vector indexes. Asked `atmos-lang` to count `set :out` in the
ports by form of the place (field, index, `@#`, `@+`, variable, `pub`).
Recommended so far: refuse `@+` (no change, 0 uses, no hole/shift rule
needed); refusing indexes in general waits for the count.

Count by `atmos-lang` (2026-11-26): 63 `set :out` in 4 repositories:
field 55 (lessmilk 26, sdl-snxkx 19, sdl-flare 10), index 4, `@#` 0,
`@+` 0, outer variable 4 (env-ray), `pub` 0. The 4 indexes: sdl-flare
`loot.dropping@(#loot.dropping) = e` (an append by hand, walked by
`loop`, reset to `[]` each frame: the only vector use); sdl-snxkx
`owner.pub.springs@(id)` x2 and `shop.pub.items@(i)`, keyed, never
walked. Recommended (#6): refuse `@+` (no change) and keep `set :out` on
indexes (refusing them breaks 4 lines in 2 ports).

## Notes

- The golden tests `0058`-`0063` (`ppp-1` to `ppp-6`) are written with
  `t@(#t)` and `t@(#t-1)`, not with the markers; the markers are covered
  by `exs/exp-13-ppp.atm` and the manual examples.
- Any form must keep the `pin` rules: the ceiling on the stored value,
  and `set :out` (TODO-pin #21, #22).
- Checked on main `31881e0`: `set :out v@+ = o` is refused at compile time
  (`invalid set :out : expected field or index`), which is TODO-pin #21
  (open, `atmos-pin`). The manual's example at "Pin" is a plain
  `set out@+ = o`, not `:out`. Not this session's to change.

## The 4 `set :out` on an index (read 2026-11-26, clones of sdl-flare and sdl-snkrx)

| Where | Line | Use of the table | A nil when the value ends |
|---|---|---|---|
| sdl-flare src/loot.atm:259 | `set :out loot.dropping@(#loot.dropping) = e` (an enemy that died this frame) | walked by `loop` (loot.atm, play.atm:453), reset to `[]` each frame | a hole, only if the enemy's table ends before `loot.logic` of the same frame |
| sdl-snkrx src/button.atm:20 | `set :out owner.pub.springs@(id) = s` | read by key `springs@(e.id)`; ids are tags, or numbers in credits.atm (sparse already) | correct: "no spring" |
| sdl-snkrx src/shop.atm:54 | same, `ShopButton` | same | correct |
| sdl-snkrx src/shop.atm:551 | `set :out shop.pub.items@(i) = s` | read by `items@(e.i)` | correct |

The 19 `A.w.objs@(o.b).task` of sdl-snkrx are a field of an element
(counted as field): no hole.

Alternatives in code, with no `set :out` on an index:
- flare: (a) do the work at the death (xp and loot in `loot_dropping`):
  changes the order of the random draws against the original; (b) append
  a copy of the data it needs (`xp`, `loot`, `loot_count`, `x`, `y`) with a
  plain `@+`: needs `e.loot` not pinned, and `set en.loot = []` would no
  longer reset the enemy; (c) a task reference instead of the table (a
  dead task stays valid), if the data were in its `pub`.
- snkrx: invert: the owner emits to the button, which pulls its own
  spring (`emit @btn :Button.pull`); for `ItemCard`, the shop answers the
  card. More code; the current form is not wrong.

## #7 A test of a dead value (asked by the author 2026-11-26)

Today: a task that ended has `type` `xtask` and `atmos.status(t)` is
`dead`; a finalized table or pool keeps its `type`, and any use but `==`
raises `invalid access : finalized ...`; there is no test.

| Option | Form | `??` | Breaks |
|---|---|---|---|
| A. a type | `type(x)` gives `dead` once it ends, so `x ?? :dead` by the type rule | yes, for free | a dead task is no longer `?? :xtask` (`await :xtask`, matches); a value changes type over time |
| B. a function (recommended) | `atmos.status(x)` also takes tables and pools: `dead` when finalized (tasks as today) | none; `x ?? :table` stays true | nothing |
| C. a rule of `??` | `x ?? :dead` true for a dead task or a finalized value, type kept | yes | clashes with a tag `:dead` of the user (`:dead [] ?? :dead` is already true by the tag rule) |

## #6 reformulated by the author (2026-11-26): what `set :out` on an index does when the value ends

| Option | Effect | Ports (the 4 index uses) |
|---|---|---|
| 1. refuse on any index | compile error | 4 lines break (flare 1, snkrx 3) |
| 2. nil (today) | a hole in a walked vector | 0 break; latent hole in flare only |
| 3. error if the value is still there | the program must clear the place itself before the value ends (a loan that must be returned); the runtime never edits the table | snkrx breaks at run time: its buttons end (pool re-made) while `owner.pub.springs` still refers to them, so each needs a `defer` that clears; flare errs only if an enemy ends before the reset of the frame |
| 4. a `:dead` marker in the place | no hole, `#v` unchanged; every reader must test it (depends on #7) | flare's loop and snkrx's `spring_pull` must test |

Recommended: 2 (consistent with fields, no breakage; the hole is #22).

#6 decided by the author, 2026-11-26: "#6: fica a 2". `set :out` keeps
writing nil in the place when the value ends, for every form of place
(no change). Corrected by the author the same day ("O @+ com :out fica
aceito, mas deve ser implementado lá"): `set :out v@+ = o` is accepted,
the place being the index `#v` at the time of the set, which goes back
to nil when `o` ends (a hole if it is not the last). To be implemented
by `atmos-pin` (its #21); #22 (the hole) accepted.
