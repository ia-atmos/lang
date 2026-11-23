# TODO of the session `atmos-blocks`

The event of termination of a transparent block (`atmos-lang#6`).
Material: `261120-blocks.md`. Implementation: branch `blocks`.

## Index of items

| Item | Title | State |
|---|---|---|
| `#1` | Should the end of a transparent block be an event that programs see? | decided 2026-11-21: `#1a`, no, it ends in silence |
| `#2` | Merge of the branch `blocks` into `main` | approved 2026-11-23 (`#2a`); merged as `95946fe` (fast-forward) |
| `#3` | A `par :all` under a block toggled off: today and now, see below | remark, no decision asked |

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

## Plan of `#1a` (done on `blocks`, 2026-11-21)

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

## `#2` What the branch `blocks` does

- `trail_emit` is gone; `trail_end` calls `trail_wake`: if the parent
  trail is suspended in `a_await_trails` over a range that holds the
  ended trail and all of it is off, it is resumed with
  `trail_resume(R_EVENT)`, as a match of an emit would. This is what
  Ceu did (from memory, not checked here): the last branch of a
  `par/and` to end runs the code after it, with no event.
- A task (trail 0) that ends is still an emit of itself, unchanged.
- `make test` with `ATMOS_ASAN=1`: all suites pass (1006 cases).
- Adapted: `golden/0416-return-2` (awaits a `pin t = spawn {...}`),
  `golden/0459-par-all-3-return` (the `emit(:10)` goes after the par).
- Manual: a paragraph under transparent tasks.

## `#3` A `par :all` under a toggled-off block

- The direct wake-up skips a parent that is waiting or doomed, not one
  under a block toggled off. Before, the made-up event went through
  the filter of the toggle and was lost, so such a par would hang;
  now it goes on. A branch can hardly end while its block is off (it
  sees no events), so this is noted, not asked.

## Measurement of `#2` (by `atmos-lang`, 2026-11-21, `atmos-lang#109`)

- `make test` with ASAN on a clean copy: green.
- 2197 `.atm` files of 15 clones: generated C and diagnostics identical.
- No port has `await true`, `await :xtask`, `await \{...}` nor
  `await(\{...})`.
- sdl-pingus: its suite passes (user time 2m41 against 2m55 on main).
- A loop of 300,000 `par :all` of two blocks plus a `spawn {}`, with 30
  idle tasks: 0.043 to 0.045 s against 0.36 to 0.45 s on main.
- After the merge, `pool`, `grab` and `context` rebase over `trail_end`.

## Last check before the merge (by `atmos-lang`, 2026-11-23)

- `8801b76` against main `ae8ca73`: suite green with ASAN, 48
  configurations; C and diagnostics of the 2197 port files identical;
  sdl-pingus and the bomber test block pass; 300,000 `par :all` of two
  branches: 0.118 s on main, 0.059 s on `blocks`.
