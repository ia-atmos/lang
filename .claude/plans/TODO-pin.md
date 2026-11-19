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
| #1 | An outer place that holds a value of an inner block (case E) | decided 2026-11-19: `set :out place = value` (the place goes back to `nil` at the end of the block of the `set`), plus the state "finalized" of data, whose use is an error (`invalid access : finalized table pinned at f:N`). Implemented and merged in `main` (`8361f88`, 2026-11-19), after 12 ports passed with no `invalid access : finalized`. See below |
| #2 | env-pico: the finalizers of a layer and of a font reset the current layer and the pencil's font, instead of the binding's own count | open, with the author (not asked to env-pico yet) |
| #3 | A compile flag that removes the checks (the end-of-block finalization stays, it is semantics) | open: "not yet" (2026-11-17). Needed by the experiment of the paper (CLBG, with and without the checks) |
| #4 | Choices of the implementation that the author did not confirm: a task owns its `pub` and its final value; an emitted event is owned by the emit, and a receiver cannot pin it; a native declares what it holds (`a_nat_holds`) | open, with the author |
| #5 | The `spawn` statement as the only implicit pin | open, with the author. Recommended: keep |
| #6 | sdl-snxkx: the second migration (pins given away) and the contacts sent only to the two tasks | waiting for its session. The experiment of the context is no longer needed: with `set :out w.tasks@(b) = task` in the body of each task, the world made by the caller keeps the map (answered with a tested program, 2026-11-19); `SHARED_SPRINGS` has the same direct form |
| #7 | Known limits of the collection of cycles (leaks): a cyclic value that a call returns and that is dropped without a name; a cyclic value given to a task of a pool of an outer block, when the task ends | open, documented in the manual; no plan |
| #8 | Remarks of the ports for the author: env-uv (a function that creates a handle and gives it away declares it with `val`, the weaker word; on a throw in the middle the handle is closed by the count, not by the block); Doom and env-uv (the same lines changed three times in three days: a change that breaks programs should say what migrates) | relayed on 2026-11-18, not answered |
| #9 | Declare the rules stable again, and tell `paper-pin` and the ports | waits for #1, #3, #4, #5 |
| #10 | The ports on `8c9453c` (refined adoption, the ceiling of an argument of a task, the wider warning): look for `invalid pin : ... named at <a task's line>` | confirmed with no anomaly: sdl-pingus, env-uv, ray-lessmilk, env-ray; the others have not reported |
| #11 | The warning of a pin given away missed `until c, v` and `while c, v` (found by sdl-pingus: `pin r = await Game(path)` ... `until !again, r`, a live defect that only the replay of the demos reached) | done on `pin` (2026-11-18): the values after the condition are checked against the pins of the loop; test `warn-pin-02` |

## #1, as decided

- The path: the block form that restored the previous value was unsound
  with `par`; a `:weak` set needed a list of places per value; a check
  of who reads needed the owner in each pinned value; a context
  (`atmos-context`) keeps the hierarchy but does not serve a reader
  outside (the hero of Flare); the author's own form (the value born in
  the field of the outer table, plus a `defer` that sets `nil`) works
  today with no mechanism, but a reader outside keeps the value alive
  and running, silently.
- Decided: `set :out place = value` (the author chose `:out`; the other
  names were `:weak`, `:lend`, `:export`, `with`, `val place = value`).
  The place is a field or index of a table, or a variable of an outer
  block (not a pin, not a whole `pub`). No check of an escape and no
  adoption. At the end of the block of the `set`, the place goes back to
  `nil` if it still refers to the value.
- A name outside that outlives the block refers to a finalized value.
  Data that its block finalized is now marked (`A_FIN`, one more value
  of the state byte of the header, no new field): an index, a write, a
  length, an iteration, a call, and a spawn into a finalized pool raise
  `invalid access : finalized <type> pinned at f:N`. A dead task stays
  as it was.
- Measured: no cost (20 million table reads: 0.70 s before, 0.68 s
  after; binary-trees and the write loop unchanged). Tests: `out-01`,
  `out-02`, `out-err-01`.
- Not done: `set :out pub = v` (the whole `pub`); #1d (the context) is
  with `atmos-context`.

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
