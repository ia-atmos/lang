# TODO of the session `atmos-pool`

Awaits and emits over pools and tasks. Material: `261118-pool.md`
(from `atmos-lang`); cases: `261118-pool-cases.md`. Items are `#N`
(standing rule of 2026-11-18): those born in `atmos-lang` keep its
prefix; those born here are `atmos-pool#N`.

## Index

| Item | Title | State |
|---|---|---|
| `atmos-lang#1` | An await over a pool with a condition | Waiting for the author (options a-d below) |
| `atmos-lang#2` | Phases inside a tick | Open, not yet put to him |
| `atmos-lang#4` | An `emit` that names its receiver, instead of `emit @2` | Open, not yet put to him |
| `atmos-lang#31` | `await x` | Moved to `atmos-args` |
| `atmos-lang#32` | `await :all ts` on an empty pool | Done in `atmos-lang` (`cab49e2`) |
| `atmos-lang#33` | Time emitted to a pool or a task | Open, not yet put to him |
| `atmos-lang#60` | Where the exception of `emit @t` goes | Open, not yet put to him |
| `atmos-pool#1` | The behaviors of section 3 of `261118-pool.md` (order of awaking, loop over a pool, `await(n, p)`), decided in `atmos-lang` and not acknowledged | Reported, not acknowledged |
| `atmos-pool#2` | A spatial index (grid) as a library, the only thing that removes the cost W x P of the cases | Remark, not asked |

## A. Waiting for the author (one at a time)

`atmos-lang#1`, an await over a pool with a condition. Options:

- `#1a`: a function of the library, `find(xs, f)`, over anything that
  `loop ... in` walks (a pool or a table: SNKRX, Flare and Lessmilk
  keep their entities in tables, also after the migrations of `pin`),
  giving the first member whose `f` is truthy, and the idiom `val t =
  await :tick until find(ts, f)` in the manual (an `await ... until c`
  evaluates to the result of `c`; checked on the compiler of
  2026-11-18 with `find` written in Atmos). A `filter` companion for
  the cases that act on all that match.
- `#1b`: a new pattern, `await :some ts until f` (`:any ts until c`
  already means "a task ended, and `c`"). Pools only.
- `#1c`: re-check on change of `pub`.
- `#1d`: members emit to the pool of zones.

Recommended: **`#1a`**. The cases need the matching task, not a
mechanism; `#1c` saves nothing since members change every tick; the
cost W x P only goes away with `atmos-pool#2`.

Next: `atmos-lang#60`, `#4`, `#2`, `#33`, with the recommendations of
`261118-pool.md`, section 2.

## B. Work known, not done

- After `atmos-lang#1`: implement it here, on the branch `pool` (rule
  of the author, 2026-11-18, `TODO.md` "Standing rules", checked in
  the transcript of `atmos-lang`): bring `main` in, run `tst/run.sh`,
  `make test-rt test-env` with `ATMOS_ASAN=1`, ask `atmos-lang` to
  measure the ports if existing programs change, merge after the
  author approves. Tell `atmos-grab` and `atmos-context` what changes
  in `rt/task.c`.

## C. Reported, not acknowledged

- `atmos-pool#1`.
- The cases of `atmos-lang#1` (`261118-pool-cases.md`), 2026-11-18,
  and that the rules of `pin` did not change them (checked again on
  2026-11-18: the survey was already after both migrations).

## D. Decided by the author

- `atmos-lang#32` (decided in `atmos-lang`): `await :all ts` over an
  empty pool is satisfied at once, with `nil, nil, ts`; `:any` still
  waits (`cab49e2`).
- `atmos-lang#31` moved to `atmos-args`, on one rule for arguments
  without parentheses.
