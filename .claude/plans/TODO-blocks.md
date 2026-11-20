# TODO of the session `atmos-blocks`

The event of termination of a transparent block (`atmos-lang#6`).
Material: `261120-blocks.md`. Implementation: branch `blocks`.

## Index of items

| Item | Title | State |
|---|---|---|
| `#1` | Should the end of a transparent block be an event that programs see? Recommended: no, it ends in silence | asked 2026-11-20, waiting for the author |

## Facts found by the session (2026-11-20)

- The program of the material runs as described on `main` (`70af8bd`):
  an `await :xtask` in a sibling wakes with a made-up task whose `ret`
  is the value of the block.
- An `await true` in a sibling wakes anyway when the block ends because
  of an emit (the emit reaches it too); what changes is the value it
  gets: the made-up task today, the emitted event after.
- The two golden tests that observe the event, found by hiding the
  made-up task from every await but the wait of a `par` (experiment,
  undone; all other 880 cases pass):
  - `golden/0416-return-2`: reads `it.ret` of a sibling block through
    `await(true)`.
  - `golden/0459-par-all-3-return`: a branch of a `par :all` awaits
    `true` and is awoken by the end of the other branch, which ends at
    once; with no event it waits forever and the program prints
    nothing.
- The manual (`doc/manual.md`) does not describe the event.

## Plan, if `#1` is approved

- First part: the wait of a `par` (and of a `toggle`) for its trails
  gets a direct wake-up of the waiting trail, with no emit.
- Second part: `trail_emit` goes; a block that ends drops its value.
- Adapt the two golden tests; tell `atmos-pool`, `atmos-grab` and
  `atmos-context` what changes in `trail_end`; `make test`; ask
  `atmos-lang` to measure the ports; merge only after approval.
