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

## What the other branches hook in `rt/task.c`

- `context` (`b0a9489`, told by `atmos-context` on 2026-11-20): hooks
  `trail_resume` (a `ctx_switch` around the resume), `emit_trail`
  (`LAZY = 1` before each `trail_resume`), `trail_off` (`ctx_drop`),
  `a_emit` and `a_spawn`; it does not touch `trail_end`. So the direct
  wake-up of a `par` must resume the waiting trail through
  `trail_resume`, as the late exit already does (`R_LATE`), and not by
  another path; otherwise tell `atmos-context`.
- `lang` (merged in `61979ce`): `a_resumed` records the `(await)` line
  of a trace on `R_EXIT`, on the path of an error leaving a branch of a
  `par`.
- `pool`: changes `a_emit` (told by `atmos-lang`). 
- `grab` (`ad74bc3`, told by `atmos-grab` on 2026-11-20): does not
  touch `trail_end` nor the wake-up of a `par`. It hooks `trail_off`
  (`grab_off`), wraps `emit_trail` (capture conditions `cap_out`,
  `cap_skip` in `emit_kids` and `emit_node`), adds `a_trail_grab`. A
  grab trail is an ordinary transparent block: it ends through
  `trail_end` and its parent awaits it with `a_await_trails(tr, id,
  1)`, so it also depends on the wake-up of the waiting trail. Tell
  `atmos-grab` when the way a transparent trail ends changes; whoever
  merges second rebases.
