# TODO of the session `pool`

Awaits and emits over pools and tasks. Material: `261118-pool.md`
(from the compiler session); cases: `261118-pool-cases.md`. The item
numbers are those of `TODO.md` of the compiler session.

## A. Waiting for the author (one at a time)

| # | Decision | Recommended |
|---|---|---|
| 1 | An await over a pool with a condition: A (a function of the library, `find(ts, f)`, and the idiom `await :tick until find(...)` in the manual), B (a new pattern `await :some ts until f`), C (re-check on change of `pub`), D (members emit to the pool of zones) | **A**: the cases need the matching task, not a mechanism; C saves nothing since members change every tick; the cost W x P only goes away with a spatial index (a library) |

Next, after item 1: 60 (where the exception of `emit @t` goes), 4, 2,
33, with the recommendations of `261118-pool.md`, section 2. Item 32
was decided in the compiler session, and item 31 moved to the session
`args` (see D).

## B. Work known, not done

- After item 1: the decision and small programs to `ceu-atmos`.
- The ports that keep entities in tables (SNKRX, Flare, Lessmilk) gain
  from item 1 only if they move them to pools, which item 40 (rules of
  `pin`) blocks today.

## C. Reported, not acknowledged

- Section 3 of `261118-pool.md` (order of awaking, loop over a pool,
  `await(n, p)`), from the compiler session.
- The cases of item 1 (`261118-pool-cases.md`), 2026-11-18.

## D. Decided by the author

- Item 32 (reported by `ceu-atmos` on 2026-11-18, decided there):
  `await :all ts` over an empty pool is satisfied at once, with
  `nil, nil, ts`; `:any` still waits (`cab49e2`).
- Item 31 (`await x`) moved to the session `args`, on one rule for
  arguments without parentheses.
