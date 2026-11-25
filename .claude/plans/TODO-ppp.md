# TODO-ppp: the tip markers `@#` `@+` and `+=` on vectors

Session `atmos-ppp`, opened 2026-11-23. Material: `261123-ppp.md`.
Branch `ppp` (from origin/main `9863fd2`; main `31881e0` merged in 2026-11-25).
Since 2026-11-24 coordination goes through `atmos` (session_011eECohhJbc4GMpAj3yyeEo).

| # | Item | State |
|---|---|---|
| #1 | `+=` on vectors: append in place (`set v += x` pushes `x`)? | decided by the author, 2026-11-25 (in `atmos`, checked in its transcript): "atmos-ppp#1: nao fazer como recomendacao dele". No `+=` on vectors; `set v@+ = x` stays the append |
| #2 | Keep, change or drop `@#` / `@+` | by the session, to be confirmed (2026-11-25): keep both as they are. `@+` is the append (359 uses, #1); `@#` is the read side of it (peek, 7 uses) and `set v@# = nil` is the pop; dropping `@#` saves nothing and breaks 7 lines. No code change |
| #3 | `++=` on vectors in place (extend, no copy, works on `pin`) | superseded by #3a (the author leans to in place). Was: no change. 0 uses; `set x OP= e` = `set x = x OP (e)` stays one rule for every operator; in place would make an alias see the change (checked: `var w=[1]; val al=w; set w ++= [2]` gives `#w=2, #al=1` today) and would also have to define in-place merge for tables with keys. No code change |
| #3a | `++=` in place: on a table, append the vector part of `e` after `#x` and copy its keys into `x`, in place (each store checked as `set x@k = v`, so `pin` and the ceiling apply); on a string or number, rebind as today | reopened by the author 2026-11-25 ("O ++= parece fazer sentido modificar inline, o que acha?"); recommended: yes. Asked |
| #4 | Pop and peek as values (marker vs `vector.remove`) | by the session, to be confirmed (2026-11-25): no new form. Peek is `v@#`; pop as a value is `vector.remove(v)`; pop as a statement is `set v@# = nil` (on an empty vector it does nothing: checked). No code change |

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
