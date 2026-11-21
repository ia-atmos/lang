# TODO: session `atmos-pin`

The session that discusses, decides, and implements `pin`, on the
branch `pin` of this repository (since 2026-11-18; before, the rules were
decided here and implemented by `atmos-lang`: `261115-pin.md`,
`261118-pin-handoff.md`). This session cannot push to `main`:
`atmos-lang` fast-forwards `main` to `pin` after each approved change.

Last update: 2026-11-21.

## Index of the items (`atmos-pin#N`)

Identifiers used before the rule of 2026-11-18 (decisions 1 to 6, cases
A to H, E1 to E4, "item 2") stay valid for what was written with them.

| # | Title | State |
|---|---|---|
| #1 | An outer place that holds a value of an inner block (case E) | decided 2026-11-19: `set :out place = value` (the place goes back to `nil` at the end of the block of the `set`), plus the state "finalized" of data, whose use is an error (`invalid access : finalized table pinned at f:N`). Implemented and merged in `main` (`8361f88`, 2026-11-19), after 12 ports passed with no `invalid access : finalized`. See below |
| #2 | env-pico: the finalizers of a layer and of a font reset the current layer and the pencil's font, instead of the binding's own count | done by `env-pico` (`9df0db43`, 2026-11-19): 67 of 67 tests, ASAN clean, the three games run, screenshots unchanged. Kept by design: a layer or a font that nobody pinned lives while it is the current one (the binding holds it as a name would). To recheck if the runtime changes how it closes a pinned value: the finalizers drop the binding's reference to the box being finalized |
| #3 | A compile flag that removes the costs of pin | approved by the author with both levels (2026-11-21), merged into `pin`; waits for the measurement and the merge by `atmos-lang`. Two levels: `ATMOS_PIN=unchecked` (no checks, no ceiling or floor; the semantics stays: finalization at the end of a block, deep pin, cycles collected) and `ATMOS_PIN=off` (also, `pin` only for tasks and pools, as the author asked; no collection of cycles). Measured with #17 (min of 5, s; main / normal with #17 / unchecked / off): binary-trees 0.74 / 0.29 / 0.24 / 0.23; a loop that writes numbers 0.86 / 0.78 / 0.70 / 0.68; one that writes tables 1.52 / 1.27 / 1.26 / 0.78; 20M table reads 0.61 / 0.58 / 0.51 / 0.50; 2M pinned tables 0.81 / 0.84 / 0.80 / 0.45. So the checks cost 5 to 15%, the semantics the rest: the finalization of pinned data (2M pinned tables) and the collection of cycles (the loop that writes tables) |
| #4 | Choices of the implementation: a task owns its `pub` and its final value; an emitted event is owned by the emit, and a receiver cannot pin it; a native declares what it holds (`a_nat_holds`) | confirmed by the author, 2026-11-19 |
| #5 | The `spawn` statement as the only implicit pin | confirmed by the author, 2026-11-19: it stays |
| #6 | sdl-snxkx: the second migration and the contacts sent only to the two tasks | closed, 2026-11-21: migrated (36 pins to `val`); its main (`ad37d31`) now has `contacts-pinout`: contacts to their two tasks, springs lent with a conditional `set :out`, the program-wide registry gone; 40 tests identical on `lang` after #17 and #3, no `invalid pin : shared` |
| #7 | (superseded by #18) The two leaks of the collection of cycles: a cyclic value that a call returns and that is dropped without a name; a cyclic value given to a task of a pool of an outer block, when the task ends | fixed on `pin`, 2026-11-20 (by the session, under the rule "work with no input"; a defect, no rule changes): a reference that goes and is not a name (a temporary, a result that is dropped, an entry of a value that is freed) runs the same trial deletion on a value with no name that something else still holds; the parameters of a task do it when the task ends, whatever the ceiling, giving up beyond 256 values. No measurable cost. Test `cycle-02`. Waits for the measurement on the ports and the merge by `atmos-lang` |
| #8 | Remarks of the ports: env-uv (a function that creates a handle and gives it away declares it with `val`, the weaker word; on a throw in the middle the handle is closed by the count, not by the block); Doom and env-uv (the same lines changed three times in three days) | closed, 2026-11-21 (reassessed with the author): on a throw, the block of the function unwinds and its `val`, the only reference, goes there, so the handle closes at the same point as with a `pin`, and the function is not the owner (who receives it decides); the churn was answered by the declaration of stability of 2026-11-19 and the lists of migration since. Answered to env-uv |
| #9 | Declare the rules stable again, and tell `paper-pin` and the ports | done: the author declared the rules of `pin` stable on 2026-11-19, as they are in `main` `8361f88` (see "Stable" below). Told to `paper-pin` and, through `atmos-lang`, to the ports. The flag (#3) is outside: it changes no rule |
| #10 | The ports on `8c9453c` (refined adoption, the ceiling of an argument of a task, the wider warning): look for `invalid pin : ... named at <a task's line>` | closed, 2026-11-20: no anomaly in sdl-pingus, env-uv, ray-lessmilk, env-ray (reported here), and in sdl-flare, doom, sdl-tux, env-iup, env-sdl, env-pico, pico-frescogo, pico-1oom (their suites on `4199167` or later, reported to `atmos-lang` for `atmos-lang#81`). sdl-snxkx is #6 |
| #11 | The warning of a pin given away missed `until c, v` and `while c, v` (found by sdl-pingus: `pin r = await Game(path)` ... `until !again, r`, a live defect that only the replay of the demos reached) | done on `pin` (2026-11-18): the values after the condition are checked against the pins of the loop; test `warn-pin-02` |
| #12 | `atmos-lang#63`, here since pin moved: a `pin` of a task that is the last statement of its block (`pin t = if c { spawn T() }` at the end of a loop body) dies at once and was not flagged | done on `pin`, 2026-11-20 (by the session, under "work with no input"): a compile-time WARNING, `task aborted at end of block (a pin that ends its block)`, for a `pin` whose value is a spawn (directly, or the last expression of the branches of an `if`, of a `do`) of a task that awaits, when the declaration is the last statement of a block whose value is not used. The spawn statement in that position stays an error. Test `warn-spawn-01`; `task-06` gains the warning. In `main` (`daab0b7`); the author confirmed that the warning stays (2026-11-20, in `atmos-lang`, its `#100a`) |
| #13 | The warning of a pin given away pointed only at the line that gives the value away, not at the declaration to change (sdl-snxkx, 36 sites) | done on `pin`, 2026-11-20 (by the session): the message ends with `(expected 'val' at line N)`, the line of the declaration |
| #14 | sdl-snxkx: the logic of its frame loop is about 12% slower between its old tree on `3a6ffe9` and its new tree on `c4e99c4` | fixed on `pin`, 2026-11-20 (by the session; a defect of performance, no rule changes). Diagnosed by sdl-snxkx with a standalone program: the compilers from `8c9453c` on do not differ; the time is `a_unname_x` at the end of each step of a loop, which called `ceil_up` for each of the 23 slots of the block (names and temporaries), almost all numbers. Now `a_unname` tests the slot inline and only calls for an unpinned value with a ceiling at the block: its program goes from 134 to 108 ns per step. Waits for its measurement in the game, and the merge by `atmos-lang` |
| #15 | Two escapes that no warning sees (sdl-snxkx) | closed, 2026-11-21 (reassessed with the author): the first is the ceiling doing its job; a warning for the second (a field of a pinned value put into a value that leaves) needs to follow values across variables, with false positives, and the error at run time already names the line of the pin. Not done |
| #16 | `set :out` written in a function or in an `if` lasted only that block (sdl-flare, sdl-snxkx) | decided by the author, 2026-11-21: #16b, the place lasts until the VALUE ends (pinned data: when its block finalizes it; a task: when it terminates; a pool: when aborted), wherever the `set :out` is written; unpinned data still leaves at the end of the block of the `set`. The branch `pin-out` is merged into `pin`; waits for the measurement and the merge by `atmos-lang`. Evidence: sdl-flare (five places, traces identical, no slowdown), sdl-snxkx (contacts and springs, 40 tests identical, no slowdown). It only extends the life of a place (see the note in the history of this item) |
| #17 | The ceiling cost two walks of a whole value per name (binary-trees, which pins nothing, 0.74 s against 0.23 s with no pin) | approved by the author (2026-11-21), merged into `pin`: the ceiling is set and reset on the value only; when a pin adopts, a visit counts the references among the values it would adopt, and a value held elsewhere is only referenced, with what it holds; pinning explicitly a value held elsewhere is `invalid pin : shared table` (it was `table named at`, for a value inside a named one). Also: no search for cycles from a released value before the first write of a reference, and that search gives up beyond 256 values. Binary-trees 0.74 -> 0.29 s. Full suite passes (pin-err-06 says `shared table`). Waits for the measurement on the ports and the merge by `atmos-lang` |
| #18 | Remove the collection of unpinned cycles | decided by the author, 2026-11-21 ("Tire a checagem"; paper-pin had relayed "não checar"): back to the rule "a cyclic structure must be pinned; without a pin, reference counting applies and an unpinned cycle leaks". Removed: the trial deletion at the end of a block, the orphan path (temporaries, entries, task arguments), the write counter and its snapshots, the collection of the globals. `cycle-02` removed, `cycle-01` now pins its cycles. Found and fixed with it: `pin var x` with no initial value did not register the pin, so what it got later was not finalized (a recursive closure through `pin var g` leaked). Supersedes #7. Measured by `atmos-lang` (no anomaly); merge approved by the author, 2026-11-21 ("#18. Sim, main"; "Checagem sai de todo modo": from every mode) |
| #19 | A forgotten pin on a cycle leaks silently (only ASAN sees it); the common case is a recursive closure through `var g; set g = func ... g ...` | done on `pin`, 2026-11-21 (approved by the author: "Ok"): a warning at compile time when a function that refers to a `var` is assigned to that same `var` (`unpinned cycle : the function refers to 'g', which holds it (expected 'func g' or 'pin var g')`). Other cycles still leak silently. Test `warn-cycle-01`. Waits for the measurement and the merge by `atmos-lang` |
| #20 | A global event cannot carry a value of an inner block (programs send ids instead) | open, bad case, by design (an event with a pinned value only reaches targets inside its block) |
| #21 | `set :out` does not take `@+` | open, bad case. Moderate: the place is the new index, removed (set to nil) when the value ends |
| #22 | `set :out` leaves nil holes in a vector when the value ends | open, bad case, inherent to clearing a place |
| #23 | `set :out pub = v` (the whole `pub`) is not accepted | done on `pin`, 2026-11-21 (approved by the author: "Ok"): the pub of the running task, held by the entry of the registry with the task; it goes back to nil when the value ends, or, for unpinned data, when the block of the `set` ends. Test `out-pub-01`. Waits for the measurement and the merge by `atmos-lang` |
| #24 | The errors of pin are only found at run time; the warnings cover part of them | open, bad case, by design |
| #25 | An explicit pin of a value held elsewhere is `invalid pin : shared table` | open, bad case, inherent to a single owner |
| #26 | A C global that holds Atmos values needs a finalizer written by hand | open, bad case. Moderate: a registration call for C roots |
| #27 | The `pub` of a task that ended stays until its pool ends | open, bad case. Moderate: release it when the task ends and no one refers to the task |
| #28 | The receiver of an event cannot pin its value | open, bad case, by design (the emit owns it) |
| #29 | Rule 4 and the finalization of retired values are worded by reference counting ("at once"); paper-pin proposed wording by block | decided by the author, 2026-11-21: deterministic reference counting is part of the semantics. The manual keeps "at once" and says so |

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
- Without a pin, reference counting applies, and an unpinned cycle
  leaks: a cyclic structure must be pinned (#18, 2026-11-21; from
  2026-11-18 to #18, unpinned cycles were collected).
- An event with a pinned value only reaches a target attached inside
  the block of the pin. An emitted event is owned by the emit. A task
  owns its `pub` and its final value. A native declares what it holds.
  The `spawn` statement is the only implicit pin.
- `set :out place = value`: a place outside refers to a value until the
  value ends (#16b, 2026-11-21; before, until the block of the `set`
  ended). Finalized data may not be used (`invalid access : finalized
  ...`).
- `ATMOS_PIN=unchecked` (no checks, same semantics) and `ATMOS_PIN=off`
  (`pin` only for tasks and pools) (#3).

### Cases of the suite for each sentence (audit of 2026-11-20)

| Sentence | Cases (`tst/cases/`) |
|---|---|
| Pinned and unpinned, floor and ceiling | `floor-01`, `floor-02`, `ceil-01`, `pin-err-01`, `pin-err-04` to `06` |
| `pin` is optional; `val` and `var` hold unpinned values | `pin-val-01`, `pin-val-02`, `ceil-01` |
| `pin val`, `pin var`, a parameter `pin x`, `pin func` | `pin-val-01`, `par-mod-01`, `par-mod-02`, `pool-val-01` |
| A bare `pin` is `pin val` | `stable-01` |
| A store only goes where the block of the value lasts | `pin-01` to `03`, `pin-err-02`, `pin-err-03` |
| A pinned value never leaves its block | `xfer-01`, `xfer-02`, `warn-pin-01`, `warn-pin-02` |
| The pin is deep | `deep-01` to `05` |
| A named value: adopted at the block of its name or outside, referenced inside | `deep-06`, `deep-07` |
| Reassigning an owned place retires the old value | `repin-01` to `04` |
| The floor rises while only the name refers to the value | `floor-up-01` |
| An argument of a task that outlives its spawner | `deep-07` |
| A cyclic structure is pinned (an unpinned one leaks) | `cycle-01` |
| An event with a pinned value reaches a target inside the block of the pin | `emit-pin-01` |
| An emitted event is owned by the emit | `stable-01` |
| A task owns its `pub` and its final value | `stable-01`, `pin-await-01`, `deep-04` |
| A native declares what it holds | none here (it needs C): covered by `env-pico` (`tst/life`, `tst/life-loop`) |
| The `spawn` statement is the only implicit pin | `stable-01`, `task-06`, `warn-spawn-01` |
| `set :out` | `out-01`, `out-02`, `out-err-01` |
| Finalized data may not be used | `out-02` |
| An unpinned pool held by a `val` | `pool-val-01` |

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
