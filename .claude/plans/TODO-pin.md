# TODO: session `atmos-pin`

The session that discusses, decides, and implements `pin`, on the
branch `pin` of this repository (since 2026-11-18; before, the rules were
decided here and implemented by `atmos-lang`: `261115-pin.md`,
`261118-pin-handoff.md`). This session cannot push to `main`:
`atmos-lang` fast-forwards `main` to `pin` after each approved change.

Last update: 2026-11-20.

## Index of the items (`atmos-pin#N`)

Identifiers used before the rule of 2026-11-18 (decisions 1 to 6, cases
A to H, E1 to E4, "item 2") stay valid for what was written with them.

| # | Title | State |
|---|---|---|
| #1 | An outer place that holds a value of an inner block (case E) | decided 2026-11-19: `set :out place = value` (the place goes back to `nil` at the end of the block of the `set`), plus the state "finalized" of data, whose use is an error (`invalid access : finalized table pinned at f:N`). Implemented and merged in `main` (`8361f88`, 2026-11-19), after 12 ports passed with no `invalid access : finalized`. See below |
| #2 | env-pico: the finalizers of a layer and of a font reset the current layer and the pencil's font, instead of the binding's own count | done by `env-pico` (`9df0db43`, 2026-11-19): 67 of 67 tests, ASAN clean, the three games run, screenshots unchanged. Kept by design: a layer or a font that nobody pinned lives while it is the current one (the binding holds it as a name would). To recheck if the runtime changes how it closes a pinned value: the finalizers drop the binding's reference to the box being finalized |
| #3 | A compile flag that removes the checks (the end-of-block finalization stays, it is semantics) | open: "not yet" (2026-11-17). Needed by the experiment of the paper (CLBG, with and without the checks) |
| #4 | Choices of the implementation: a task owns its `pub` and its final value; an emitted event is owned by the emit, and a receiver cannot pin it; a native declares what it holds (`a_nat_holds`) | confirmed by the author, 2026-11-19 |
| #5 | The `spawn` statement as the only implicit pin | confirmed by the author, 2026-11-19: it stays |
| #6 | sdl-snxkx: the second migration (pins given away) and the contacts sent only to the two tasks | waiting for its session. The experiment of the context is no longer needed: with `set :out w.tasks@(b) = task` in the body of each task, the world made by the caller keeps the map (answered with a tested program, 2026-11-19); `SHARED_SPRINGS` has the same direct form |
| #7 | The two leaks of the collection of cycles: a cyclic value that a call returns and that is dropped without a name; a cyclic value given to a task of a pool of an outer block, when the task ends | fixed on `pin`, 2026-11-20 (by the session, under the rule "work with no input"; a defect, no rule changes): a reference that goes and is not a name (a temporary, a result that is dropped, an entry of a value that is freed) runs the same trial deletion on a value with no name that something else still holds; the parameters of a task do it when the task ends, whatever the ceiling, giving up beyond 256 values. No measurable cost. Test `cycle-02`. Waits for the measurement on the ports and the merge by `atmos-lang` |
| #8 | Remarks of the ports for the author: env-uv (a function that creates a handle and gives it away declares it with `val`, the weaker word; on a throw in the middle the handle is closed by the count, not by the block); Doom and env-uv (the same lines changed three times in three days: a change that breaks programs should say what migrates) | relayed on 2026-11-18, not answered |
| #9 | Declare the rules stable again, and tell `paper-pin` and the ports | done: the author declared the rules of `pin` stable on 2026-11-19, as they are in `main` `8361f88` (see "Stable" below). Told to `paper-pin` and, through `atmos-lang`, to the ports. The flag (#3) is outside: it changes no rule |
| #10 | The ports on `8c9453c` (refined adoption, the ceiling of an argument of a task, the wider warning): look for `invalid pin : ... named at <a task's line>` | closed, 2026-11-20: no anomaly in sdl-pingus, env-uv, ray-lessmilk, env-ray (reported here), and in sdl-flare, doom, sdl-tux, env-iup, env-sdl, env-pico, pico-frescogo, pico-1oom (their suites on `4199167` or later, reported to `atmos-lang` for `atmos-lang#81`). sdl-snxkx is #6 |
| #11 | The warning of a pin given away missed `until c, v` and `while c, v` (found by sdl-pingus: `pin r = await Game(path)` ... `until !again, r`, a live defect that only the replay of the demos reached) | done on `pin` (2026-11-18): the values after the condition are checked against the pins of the loop; test `warn-pin-02` |
| #12 | `atmos-lang#63`, here since pin moved: a `pin` of a task that is the last statement of its block (`pin t = if c { spawn T() }` at the end of a loop body) dies at once and was not flagged | done on `pin`, 2026-11-20 (by the session, under "work with no input"): a compile-time WARNING, `task aborted at end of block (a pin that ends its block)`, for a `pin` whose value is a spawn (directly, or the last expression of the branches of an `if`, of a `do`) of a task that awaits, when the declaration is the last statement of a block whose value is not used. The spawn statement in that position stays an error. Test `warn-spawn-01`; `task-06` gains the warning. Waits for the measurement and the merge by `atmos-lang` |

## Stable (2026-11-19)

The author declared the rules of `pin` stable, as implemented in `main`
at `8361f88` and described in `doc/manual.md` ("Lexical Memory
Management", "Pin", "Set"). No project needs to expect another migration
of the same lines. A later change that breaks programs says what
migrates (the remark of Doom and env-uv, #8).

- Two states: pinned (owned by a block) and unpinned, with a floor (what
  it holds) and a ceiling (who names it). `pin` is optional: `val` and
  `var` hold unpinned values, and `pin val` / `pin var` / a parameter
  `pin x` / `pin func` own. A bare `pin` is `pin val`.
- A store only goes where the block of the value lasts at least as long
  as the place. A pinned value never leaves its block, not even as its
  result. The pin is deep; a named value is adopted by a holder at the
  block of its name or outside, and only referenced by one inside.
- Reassigning an owned place retires the old value. The floor of an
  unpinned value may rise while only its name refers to it. An
  argument of a task that outlives its spawner has its ceiling at the
  block that ends the task.
- A cycle of unpinned values is broken at the end of the block of its
  names, and when the last reference from outside it goes (#7).
- An event with a pinned value only reaches a target attached inside
  the block of the pin. An emitted event is owned by the emit. A task
  owns its `pub` and its final value. A native declares what it holds.
  The `spawn` statement is the only implicit pin.
- `set :out place = value`: a place outside refers to a value until the
  block of the `set` ends. Finalized data may not be used (`invalid
  access : finalized ...`).
- Open, and no rule: the flag that removes the checks (#3).

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
