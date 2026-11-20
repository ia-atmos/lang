# Dynamic Values Deserve a Home: draft text

> DRAFT, by the session `paper-pin`, to be confirmed by the author (#11).
> Text for the sections of `papers/pin.md` that depend only on the rules,
> which the author declared stable on 2026-11-19 (`main` at `8361f88`,
> `doc/manual.md`: "Lexical Memory Management", "Pin", "Set"). The outline
> is not approved yet (#10): if it changes, these sections move; they are
> not rewritten. Numbers are those reported by the sessions that measured
> them, with their source in brackets; none was measured here.

## 3. The Idea

Structured concurrency gave tasks a home. A task belongs to the lexical block
that spawns it: it cannot outlive that block, and when the block terminates,
normally or by abortion, the task is aborted with it. Nothing comparable
exists for the data and resources that tasks use. A table that a task fills,
a closure it registers, or a file it opens is managed by a different
mechanism, usually a tracing collector, and lives as long as some reference
to it survives, which may be much longer, or much shorter, than the task.

Structured memory applies the same discipline to every dynamic value. Each
table, function, native resource, task, and pool of tasks belongs to a
block. The block finalizes what it owns when it ends, in reverse order and
interleaved with `defer`. A value may be stored only where its block lasts at
least as long as the place that stores it, and this is checked when the
program runs. The construct that gives a value its home is the lexical pin,
`pin`.

## 4. The Rules

**Principle.** Every dynamic value belongs to a block and may only be stored
where its block lasts at least as long as the place that stores it. The
invariant this protects is about use: no use of a value outlives its block. A
reference may outlive it only through an explicit `set :out`, and using such
a reference after the block has ended raises an error.

**Two states.** A value is *pinned* when a block owns it; that block
finalizes it when it ends. Otherwise it is *unpinned*: the result of a
constructor, a call, or a block, not yet owned. An unpinned value has two
bounds. Its *floor* is the innermost block of the pinned values it holds; it
cannot be stored outside its floor. Its *ceiling*, when a name holds it, is
the block of its outermost name; it may only be adopted at its ceiling or
outside. Between the two lies the window in which the value may be given an
owner.

**Rule 1, birth.** A value is born unpinned. Its floor comes from what it is
born with: the fields of a constructor, the operands of `++`, the captures of
a function, the arguments of a task, and, for a native value, what its
binding declares it holds. The floor may rise only while the value's own name
is the only reference to it.

**Rule 2, adoption.** An unpinned value becomes pinned by a declaration
(`pin val x`, `pin var x`, a bare `pin` being `pin val`), by a parameter
declared `pin`, by being stored into a pinned value or a `pub`, or by being
assigned to an owned place (a `pin` variable, a field of a pinned table, the
`pub` of a pinned task). The pin is deep: the unpinned values it holds are
adopted with it, except that a named value is adopted only by a holder that
outlives the name; a holder inside the name's block only refers to it, and
the value's ceiling drops to that holder's block. `val` and `var` name a
value without owning it. The `spawn` statement is the only implicit pin.

**Rule 3, store.** A place accepts a value whose block is the same as or
outer than the block of the place: a field of a table, a `pub`, a variable of
an outer block, a captured variable, a pool. A captured variable belongs to
the block that declares it. `emit` and `throw` follow the same check; an
event that holds a pinned value reaches only targets attached inside that
value's block. The one exception is `set :out place = value`: a place of an
outer block refers to a value of the current block with no check and no
adoption, and is reset to `nil` when the block of the `set` ends, if it still
refers to that value.

**Rule 4, end.** When a block ends, the values it pinned are finalized in
reverse order, interleaved with `defer`: tasks and pools are aborted, tables
and functions release what they hold, even in a cycle, and native values
release their resources. A value replaced in an owned place is retired: it
is finalized when its last reference goes, and at the latest at the end of
the block. An unpinned value that is dropped is finalized at once. Data that
its block finalized is marked, and any later use (an index, a write, a
length, an iteration, a call, a spawn into a finalized pool) raises
`invalid access : finalized <type> pinned at f:N`.

**Rule 5, result.** A pinned value never leaves its block, not even as the
block's result. A block gives away only what it has not pinned: the last
expression, a `return`, a `break`, or an `escape` may carry an unpinned value
out, unless its floor is inside the block. A captured variable leaves with
its closure.

**Ownership of shared values.** An emitted event is owned by the emit, so no
receiver can pin it. A task owns its `pub` and its final value. A native
value declares, through its binding, which values it holds.

## 8. Implementation

Atmos compiles to C. The runtime counts references and has no tracing
collector. Each object header carries its reference count, a state byte, the
depth of its block, the source line of the pin that gave it that depth, and
the ceiling with its line. The state distinguishes unpinned values, unpinned
values with a floor, values pinned by a declaration, values pinned by a
holder, values owned by the runtime, retired values, and finalized data.

The check of rule 3 is one comparison of two depths per store. Adoption
(rule 2) walks the unpinned value deeply, marking visited objects to stop at
cycles, and checks the ceiling before it pins. The ceiling is set when a name
binds a value and cleared when the block of that name ends. Rule 4 runs at
the end of each block: tasks and pools are aborted first, then data is
released. A block may have to wait for a task that is still unwinding, so
finalization always proceeds from the inside out. Rule 5 refuses a pinned
value at the exit of a block and transfers an unpinned one, all or nothing.

Retired values are kept on a list that holds one reference each. A retired
value is released as soon as that is its only reference, and whatever
remains, a cycle, is finalized at the end of the block.

Cycles among unpinned values are broken at the end of the block of the names
that reach them, by a local trial deletion. The candidates are the unpinned
values reachable from the block's names whose ceiling is at that block or
absent. References among the candidates and from the names are subtracted. A
candidate with references left is alive, with everything it reaches; what
counting alone would free is left to counting; and only the edges among what
remains, values in a cycle or reachable only through one, are cut. The pass
runs only if, since the block began, a reference was written into a table, a
box, or a `pub`. Soundness rests on the counts, not on the ceiling: the
escape information that block depths provide only selects the candidates.
The same trial deletion runs on a value with no name when the last reference
to it from outside goes: a temporary, a result that is dropped, an entry of a
value that is freed, or the parameter of a task that ends. For a parameter of
a task, the search gives up on a value that reaches more than 256 others,
which is then left to the counts. No measurable cost was reported for this
step (`.claude/plans/TODO-pin.md`, item 7).

An `emit` that carries a pinned value compares two depths, where the target
is attached and where the value is pinned, and does not walk the tree of
tasks. The compiler also warns at compile time when a pin is given away: in
a constructor that is itself given away, and in a `return`, `break`,
`escape` with a literal tag, `until c, v`, or `while c, v`.

## 9. Costs Measured

All figures below were reported by the sessions that implemented the rules.
None was measured for this paper.

- **The ceiling check.** 2,000,000 `pin`s of two small tables took 0.95 s,
  against 0.75 s without the check; the same loop with `val` took 0.75 s
  (`.claude/plans/261115-pin.md`).
- **The local trial deletion.** No cost on binary-trees, nor when only
  numbers are written. +30 to 40% (about 150 ns per iteration) on a loop that
  does nothing but write a new table into a named one. The suite and a
  6000-frame demo of sdl-pingus, and the echo benchmark of the libuv
  environment, were unchanged (report of `atmos-pin`, 2026-11-18).
- **`set :out` and the mark of finalized data.** No cost: 20 million table
  reads took 0.70 s before and 0.68 s after, and binary-trees and the write
  loop were unchanged (`.claude/plans/TODO-pin.md`).
- **The ports.** On the final rules, 12 ports pass their suites unchanged
  (`atmos-lang#81`, merge `64005fd` into `main`).

The cost of the checks as a whole, the program with checks against the same
program without them, is not measured: it needs the flag that removes the
checks, which the author has not approved yet (#7b, `atmos-pin#3`).
