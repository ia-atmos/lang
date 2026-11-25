# TODO of the session `atmos-pool`

Awaits and emits over pools and tasks. Material: `261118-pool.md`
(from `atmos-lang`); cases: `261118-pool-cases.md`. Items are `#N`
(standing rule of 2026-11-18): those born in `atmos-lang` keep its
prefix; those born here are `atmos-pool#N`.

## Index

| Item | Title | State |
|---|---|---|
| `atmos-lang#1` | An await over a pool with a condition | Done: `await :tick until iter.find(ts, f)`, on main with the branch `iter` (7c64f69, 2026-11-22) |
| `atmos-lang#2` | Phases inside a tick | Proposal ready (`261120-pool-proposals.md`), not yet put to him |
| `atmos-lang#4` | An `emit` that names its receiver, instead of `emit @2` | Proposal ready, not yet put to him |
| `atmos-lang#31` | `await x` | Moved to `atmos-args` |
| `atmos-lang#32` | `await :all ts` on an empty pool | Done in `atmos-lang` (`cab49e2`) |
| `atmos-lang#33` | Time emitted to a pool or a task | Proposal ready, not yet put to him |
| `atmos-lang#60` | Where the exception of `emit @t` goes | The author leans to the emitter (2026-11-23: "quem emite é que deve receber o erro. nao é assim com métodos?"); the session agrees and withdrew the merge; waiting for his confirmation to revert it from `pool` and document it |
| `atmos-pool#1` | The behaviours of section 3 of `261118-pool.md` (order of awaking, loop over a pool, `await(n, p)`), decided in `atmos-lang` and not acknowledged | Manual and cases checked; a case added on the branch; still not acknowledged |
| `atmos-pool#2` | A spatial index (grid) as a library | Written and measured on the branch (`lib/grid.atm`); whether it becomes a library is his |
| `atmos-pool#3` | With `atmos-lang#60` on the branch, a holder in the middle of a reaction receives the exception late: who emitted goes on after its `emit @t` | Remark, for his decision on `#60` |
| `atmos-pool#4` | Where libraries written in Atmos live (`lib/` on the branch is a guess) | Question, not yet put to him |

## A. Waiting for the author (one at a time)

Since 2026-11-24 the managing session is `atmos`
(session_011eECohhJbc4GMpAj3yyeEo): status and questions go there too
(checked in `atmos-lang`'s transcript: the author's "Ok" to `#119`).
Items new to him continue `ISSUES.md` from `#120`.

All of it is prepared on the branch `pool` (`261120-pool-work.md`); his
answer to each is a merge or a revert.

- `atmos-lang#60`: merge "to the holder"? See `atmos-pool#3` first.
  Recommended: yes; it departs from the reference.
- `atmos-pool#1`: acknowledge the three behaviours. Recommended: yes.
- `atmos-pool#2`, `#4`: the grid as a library, and where.
  Recommended: keep it out for now (the ports do not need it at their
  sizes), decide `#4` when a second library appears.
- Then `atmos-lang#2`, `#4`, `#33` (`261120-pool-proposals.md`):
  recommended `#2a`, `#4a`, `#33a`, none of which changes the language.

## B. Work known, not done

- `main` (`8c46149`, with the fix of `atm_emit_to` and the changes of
  atmos-pin) is in the branch, `15766e9`, and then `5509fa1` (the chain of holders on the stack), tip `d8a82a9`: the merge took the new body
  of `emit_to1` into `emit_held` with no conflict; `tst/run.sh`
  pass=1018 fail=0 on `d8a82a9`, `make test-rt test-env` pass (ASAN). `atmos-lang`
  reruns the suite and Pingus on it.
- After his approval: bring `main` into the branch again, run the full
  suite, merge. `atmos-lang` then migrates and tells the ports.
- `atmos-lang#2a`: the paragraph of the convention in the manual, if he
  takes it.

## C. Reported, not acknowledged

- `atmos-pool#1`.
- The cases of `atmos-lang#1` (`261118-pool-cases.md`), and that the
  rules of `pin` did not change them.
- The work of 2026-11-20 on the branch (`261120-pool-work.md`), sent to
  `atmos-lang` with what changes in `rt/task.c`, to relay to
  `atmos-grab` and `atmos-context`.

- The measurement of `atmos-lang` on `origin/pool` (`695afa1`), back on
  2026-11-20: the 2152 `.atm` files of 15 clones give the same
  diagnostics, and the same C but for the numbers of the globals; the
  suite of sdl-pingus passes; no cost in a loop of 300,000 emits. Not
  shown by it: a throw that crosses an `emit @t` at run time (only the
  suites of the ports can, and only that of Pingus runs there).

## Settled by the session, to be confirmed (rule `atmos-lang#95`)

All on the branch `pool`, reversible by not merging:

- `atmos-lang#60`: to the holder.
- `atmos-pool#2`: the grid in `lib/grid.atm`, `grid_find` with a plain
  loop.

## D. Decided by the author

- `atmos-lang#1` (2026-11-22, through `atmos-iter#1` = `#1a`): no new
  pattern; the library `iter` (`iter.find`, `iter.filter`, `iter.any`),
  merged into main at 7c64f69. The branch `pool-find` is obsolete.

- `atmos-lang#1` (2026-11-20, in `atmos-lang`): `find`/`filter` belong
  to a library over iterables, in the session `atmos-iter`. Done: the
  branch split, `pool` (75911fa) without them, `pool-find` (0c1a6ec,
  on main) with only them, handed to `atmos-iter` with notes.

- `atmos-lang#32` (decided in `atmos-lang`): `await :all ts` over an
  empty pool is satisfied at once, with `nil, nil, ts`; `:any` still
  waits (`cab49e2`).
- `atmos-lang#31` moved to `atmos-args`, on one rule for arguments
  without parentheses.
