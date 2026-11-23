# TODO-ppp: the tip markers `@#` `@+` and `+=` on vectors

Session `atmos-ppp`, opened 2026-11-23. Material: `261123-ppp.md`.
Branch `ppp` (from origin/main `9863fd2`).

| # | Item | State |
|---|---|---|
| #1 | `+=` on vectors: append in place (`set v += x` pushes `x`)? | asked 2026-11-23; recommended: no, keep `@+` as the append |
| #2 | Keep, change or drop `@#` / `@+` | waits for #1 |
| #3 | `++=` on vectors in place (extend, no copy, works on `pin`) | waits for #1 |
| #4 | Pop and peek as values (marker vs `vector.remove`) | waits for #1 |

## Notes

- The golden tests `0058`-`0063` (`ppp-1` to `ppp-6`) are written with
  `t@(#t)` and `t@(#t-1)`, not with the markers; the markers are covered
  by `exs/exp-13-ppp.atm` and the manual examples.
- Any form must keep the `pin` rules: the ceiling on the stored value,
  and `set :out out@+ = o` (manual, "Pin"; TODO-pin #21, #22).
