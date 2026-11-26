# TODO-ppp: the tip markers `@#` `@+` and `+=` on vectors

Session `atmos-ppp`, opened 2026-11-23. Material: `261123-ppp.md`.
Branch `ppp` (from origin/main `9863fd2`; main `31881e0` merged in 2026-11-25).
Since 2026-11-24 coordination goes through `atmos` (session_011eECohhJbc4GMpAj3yyeEo).

| # | Item | State |
|---|---|---|
| #1 | `+=` on vectors: append in place (`set v += x` pushes `x`)? | decided by the author, 2026-11-25 (in `atmos`, checked in its transcript): "atmos-ppp#1: nao fazer como recomendacao dele". No `+=` on vectors; `set v@+ = x` stays the append |
| #2 | Keep, change or drop `@#` / `@+` | by the session, to be confirmed (2026-11-25): keep both as they are. `@+` is the append (359 uses, #1); `@#` is the read side of it (peek, 7 uses) and `set v@# = nil` is the pop; dropping `@#` saves nothing and breaks 7 lines. No code change |
| #3 | `++=` on vectors in place (extend, no copy, works on `pin`) | superseded by #3a (the author leans to in place). Was: no change. 0 uses; `set x OP= e` = `set x = x OP (e)` stays one rule for every operator; in place would make an alias see the change (checked: `var w=[1]; val al=w; set w ++= [2]` gives `#w=2, #al=1` today) and would also have to define in-place merge for tables with keys. No code change |
| #3a | `++=` in place: on a table, the entries of `e` go into `x` itself, as `++` would put them, each stored as by `set x@k = v` (so `pin` and the ceiling apply); on a string or number, rebind as today | approved by the author 2026-11-25, with no special case for `set v ++= v` ("O caso de copiar v parece muito específico. Acho que não deve tratar especial"). Prototype on `ppp`: `a_cat_set` (rt/ops.c) reads the entries of `e` before storing, which makes `v ++= v` work with no special case; `++=` is accepted on a name that cannot be reassigned (`pin`, `val`) and is a run-time error there if the value is not a table. Tests `cset-05`, `cset-err-05`, `cset-err-06`; manual "Set" and "Concatenation". Ports: 0 uses of `++=`, nothing breaks |
| #4 | Pop and peek as values (marker vs `vector.remove`) | by the session, to be confirmed (2026-11-25): no new form. Peek is `v@#`; pop as a value is `vector.remove(v)`; pop as a statement is `set v@# = nil` (on an empty vector it does nothing: checked). No code change |

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
