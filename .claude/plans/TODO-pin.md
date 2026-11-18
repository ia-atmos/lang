# TODO: session `atmos-pin`

The session that discusses, decides, and implements `pin`, on the
branch `pin` of this repository (since 2026-11-18; before, the rules were
decided here and implemented by `atmos-lang`: `261115-pin.md`,
`261118-pin-handoff.md`). This session cannot push to `main`:
`atmos-lang` fast-forwards `main` to `pin` after each approved change.

Last update: 2026-11-18.

## Index of the items (`atmos-pin#N`)

Identifiers used before the rule of 2026-11-18 (decisions 1 to 6, cases
A to H, E1 to E4, "item 2") stay valid for what was written with them.

| # | Title | State |
|---|---|---|
| #1 | An outer place that holds a value of an inner block (case E): nothing new, a block `with x.k = v {...}` that resets to `nil`, or a `set :weak` | open, with the author. Recommended: nothing new. See below |
| #2 | env-pico: the finalizers of a layer and of a font reset the current layer and the pencil's font, instead of the binding's own count | open, with the author (not asked to env-pico yet) |
| #3 | A compile flag that removes the checks (the end-of-block finalization stays, it is semantics) | open: "not yet" (2026-11-17). Needed by the experiment of the paper (CLBG, with and without the checks) |
| #4 | Choices of the implementation that the author did not confirm: a task owns its `pub` and its final value; an emitted event is owned by the emit, and a receiver cannot pin it; a native declares what it holds (`a_nat_holds`) | open, with the author |
| #5 | The `spawn` statement as the only implicit pin | open, with the author. Recommended: keep |
| #6 | sdl-snxkx: the second migration (35 pins given away, by the compile pass of `atmos-lang`) and the experiment of the context owned by the arena (contacts sent only to the two tasks) | waiting for its session |
| #7 | Known limits of the collection of cycles (leaks): a cyclic value that a call returns and that is dropped without a name; a cyclic value given to a task of a pool of an outer block, when the task ends | open, documented in the manual; no plan |
| #8 | Remarks of the ports for the author: env-uv (a function that creates a handle and gives it away declares it with `val`, the weaker word; on a throw in the middle the handle is closed by the count, not by the block); Doom and env-uv (the same lines changed three times in three days: a change that breaks programs should say what migrates) | relayed on 2026-11-18, not answered |
| #9 | Declare the rules stable again, and tell `paper-pin` and the ports | waits for #1, #3, #4, #5 |
| #10 | The ports on `8c9453c` (refined adoption, the ceiling of an argument of a task, the wider warning): look for `invalid pin : ... named at <a task's line>` | confirmed with no anomaly: sdl-pingus, env-uv, ray-lessmilk, env-ray; the others have not reported |

## #1, as it stands

- The cases left after the experiments of the contexts (sdl-flare
  `exp-map-context`, ray-lessmilk `exp-screen-context`, env-pico): a task
  outside the block cannot receive its context (the hero of Flare); a
  container cannot hold what an inner entity pins (Lessmilk); a global
  event carries an id instead of a value; the two C globals of pico (#2).
- #1a, nothing new: ids, requests, and contexts, as today.
- #1b, `with x.k = v { ... }`: a `set` checked against the block of the
  `with`, plus a reset to `nil` at its end if the place still holds `v`.
  Sound for the place, also with `par` (the form that restored the
  previous value was not). No list in the runtime.
- #1c, `set :weak x.k = v`: the place becomes `nil` when `v` is
  finalized. Needs a list of places per value.
- Both #1b and #1c let code outside the block name the value (`val es =
  play.enemies`, then an await, then the block ends). To refuse that
  name, the runtime must know whether who reads is inside the block of
  the value, and depths do not say it (two sibling tasks use the same
  numbers): each pinned value would record its owner, and a read would
  walk the tasks.
- The author (2026-11-18): an event with a pinned value makes sense
  for the children, not outwards; the hierarchy should not be broken.

## Done on the branch `pin` (all in `main`)

- `b42c853`, `f83b46e`: the collection of cycles of unpinned values at
  the end of the block of their names (trial deletion; only the edges
  of what counting cannot free are cut; runs only after a write of a
  reference).
- `5125c9c`: `emit` with a pinned value checked by two depths, with no
  walk up the tree of tasks.
- `d9d7217`: a named value is adopted by a holder at the block of its
  name or outside; the ceiling of an argument of a task of an outer
  pool (a soundness hole).
- `8c9453c`: the warning of a pin given away sees constructors, `break`,
  `escape`, and each value of a `return`; the manual on `pin var`.
