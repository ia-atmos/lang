# Dynamic Values Deserve a Home: Structured Memory with Lexical Pins

Name of the mechanism: **structured memory**. Its construct is the
**lexical pin** (`pin`), which gives a dynamic value a home: a block.

> Status: outline. The rules in Section 4 were approved by the author on
> 2026-11-15 and are implemented in `main` since `98ad03a` (compiler,
> runtime, tests, manual). See `.claude/plans/261115-pin.md` and
> `.claude/plans/261115-pin-migration.md`.

## Abstract

Structured concurrency ties the lifetime of a task to a lexical block: the
task cannot outlive the block, and is aborted when the block terminates.
Tasks have a home; the data and resources they use do not.
We extend this discipline from tasks to all dynamic values, and call it
structured memory.
In Atmos, a lexical pin (`pin`) gives a table, function, native resource,
task, or pool a home: its enclosing block.
The block finalizes what it pins when it terminates or aborts, in reverse
order and interleaved with `defer`, which reclaims cycles and releases
resources at a known point of the program.
A dynamic check, a block depth per value compared on every store, guarantees
that no reference to a pinned value outlives its block.
A pinned value never leaves its block; a block gives away only values it has
not pinned.
The mechanism needs no static lifetime annotations and no separate region or
arena abstraction: the blocks of the program are the regions.
It bounds the lifetime of a value, but does not collect what becomes garbage
while its block is alive: reference counting does that underneath.
Cycles are broken at the end of a block: by the pin, for pinned values, and
by a local trial deletion among the unpinned values that the block's names
reach, which replaces a cycle collector.
A tracing collector would still help inside a long-lived block; Atmos leaves
it out as a design choice, trading that memory for determinism.
We argue that it is only sound and useful under structured programming, in
which blocks own tasks, abortion is a language primitive, and finalization
order is deterministic.
We present the rules, the implementation on top of reference counting, and
case studies from six ports of libraries and games to Atmos.

## 1. Introduction

- Problem: memory and resources in reactive, long-running, concurrent programs
- Tasks hold data, data holds tasks: lifetimes are entangled
- Structured concurrency solved it for tasks only
- Gap: data and resources still depend on GC, RC, or manual discipline
- Thesis: one lexical rule for tasks, data, and resources: every value has a home
- Contributions
    - the mechanism: structured memory, with lexical pins
    - a small set of rules, uniform for all dynamic values
    - a cheap dynamic check: one depth per value, one comparison per store
    - an implementation on reference counting, with no tracing collector by choice
    - an evaluation with six real ports

## 2. Background

- Structured concurrency: blocks own tasks, nurseries, scopes
- Synchronous reactive languages: Esterel, Céu, abortion as a primitive
- Atmos in one page: tasks, `await`, `emit`, `par`, `watching`, `defer`
- Dynamic values: tables, functions, natives, tasks, pools
- Baseline: reference counting, no cycles reclaimed, no known death point

## 3. The Idea

- Every dynamic value has a home: a block
- `pin x = e`: the enclosing block owns the value
- The block finalizes its values when it terminates or aborts
- No reference to a value outlives its block
- A block gives away only what it has not pinned
- Tasks, data, and resources follow the same rule
- Motto: a task, its data, and its block live and die together

## 4. The Rules

- Principle
    - every dynamic value belongs to a block
    - it may only be stored where its block lasts at least as long
    - protects finalization order and forbids references to finalized values
    - same rules for tables, functions, natives, tasks, and pools
- Two states
    - pinned: owned by a block, which finalizes it at its end
    - unpinned: not owned yet, the result of a constructor, call, or block
    - an unpinned value has a floor: the innermost block of what it holds
    - an unpinned value held by a name has a ceiling: the block of its outermost name
    - a window: it may only be adopted between its floor and its ceiling
- Rule 1, birth
    - a value is born unpinned
    - floor from what it is born with: fields, operands, captures, arguments
    - a native declares what it depends on in its binding
    - the floor may rise only while just its name refers to it
      (`val z = [] ; set z.k = k`)
- Rule 2, pin (adoption)
    - by declaration: `pin val x` or `pin var x` (bare `pin` is `pin val`),
      in the current block
    - by parameter: parameters take the modifiers of declarations,
      `func f (var x, val y, pin a, pin var z, b)`; `pin` makes the body own it
    - by storage: in a pinned value or a `pub`, in the block of the holder
    - by assignment to an owned place: a `pin` variable, a field of a
      pinned table, the `pub` of a pinned task
    - deep: an unpinned value it holds is pinned with it when the holder
      outlives the value's name (holder in the name's block or outer);
      otherwise it is only referenced, and its ceiling drops to the holder
    - only at the ceiling or outside: a name never sees its value finalized
- Rule 3, store
    - a place accepts a value of the same block or of an outer one
    - places: table field, `pub`, outer variable, captured variable, pool
    - an unpinned place is checked against its floor
    - `emit` and `throw` follow the same check; an event with a pinned value
      reaches only targets attached inside the block of the pin, checked by
      comparing two depths, with no walk of the tree of tasks
    - a captured variable belongs to the block that declares it
- Rule 4, end
    - pinned values finalized in reverse order, interleaved with `defer`
    - a dropped unpinned value is finalized at once
    - table released, native closed, task aborted
    - a value replaced in an owned place is retired: finalized when its last
      reference goes, at the latest at the end of the block (cycles broken)
- Rule 5, result
    - a pinned value never leaves its block: no implicit give-away
    - a block gives away an unpinned value (a `val`): last expression,
      `return`, `break`, `escape`
    - it leaves unless its floor is in the block
    - a captured variable leaves with its closure
- Consequences
    - `val`, `var`, parameters, loop variables, `where`: name, do not own
    - `pin` adopts at once; recommended for natives, tasks, and pools
    - a value that nobody adopts is reclaimed when it is dropped
    - a parameter may receive an unpinned value: store it outside, or return it
    - a constructor builds with `val` and returns it; the caller pins it
    - a pool held by a `val` takes its floor from its tasks, checked at its ceiling
    - cycles among unpinned values: broken at the end of the block of the names
      that reach them; `pin` chooses the moment of release
    - a named argument to a task that outlives its spawner (a task of an outer
      pool) gets its ceiling at the block that ends the task
    - `pin var` for values the block creates; `var` for those it refers to
    - natives: a child holds its parent, by rule 3
- Refinements found in the implementation
    - an emitted event: owned by the emit, at its floor, by no block
    - a task holds its `pub` and its final value
    - a task is pinned from birth by the block that owns it
    - declarations of functions and tasks name their prototypes (unpinned),
      so a local function, recursive or not, may be returned; `pin func f` owns it
- Semantics sketch: block depth as the lifetime order

## 5. Why It Matters as a Memory Mechanism

- Criteria: determinism, cycles, resources, annotations, pauses, cost, errors
- vs. tracing GC
    - not a replacement: orthogonal, and the two could be combined
    - pins give what a collector does not: a known point of finalization
    - a collector gives what pins do not: garbage reclaimed inside a live block
    - Atmos drops the collector as a tradeoff: no pauses, no heap scans
    - the price: cycles and unreachable data wait for the end of the block
- vs. plain reference counting
    - cycles are reclaimed
    - death point is lexical, not "when the last reference goes"
    - aborts a task, not only frees memory
- vs. ownership and borrowing (Rust)
    - no lifetime annotations, no borrow checker
    - aliasing is free inside the block
    - cyclic and shared structures need no `Rc`/`Weak`/`unsafe`
    - cost: errors are dynamic, not static
- vs. regions (Tofte-Talpin, Cyclone)
    - no region variables, no region polymorphism, no inference
    - blocks are the regions; values move out as results
    - finalization runs code, not only frees memory
- vs. RAII and `defer`
    - same death point, plus a guarantee of no dangling reference
    - one finalization order for `pin` and `defer`
    - survives abortion from outside, not only normal exit
- vs. arenas
    - no arena to thread through the code
    - per value, not per allocator; values may leave
- vs. nurseries and scopes (Trio, Kotlin, Swift, Java)
    - they own tasks only; data still relies on GC
    - structured memory owns tasks and what tasks use
- Summary table: mechanism x criterion

## 6. Why It Requires Structured Programming

- Every value has a block: needs a block for every execution point
- Blocks own tasks: a task cannot hold a value past its own block
- Abortion as a primitive: a block can always be terminated, with its content
- Deterministic finalization: reverse order, inside out, interleaved with `defer`
- Synchronous reactions: no preemption in the middle of a store check
- Lexical nesting gives a total order of lifetimes: one integer per value
- Counterexamples without structure
    - detached threads and `go` statements
    - callbacks and promises that outlive the caller
    - global registries, unstructured `goto` and early exits
- Claim: structured concurrency is necessary, and structured memory completes it

## 7. Examples

- A cycle reclaimed at block end
- A native resource: a file closed at block end
- A task and its data aborted together by `watching`
- A constructor: a function that builds with `val` and returns; the caller pins
- A pool of tasks pinned outside a loop
- A work block: pinned temporaries die, an unpinned result leaves
- An unpinned value dropped: finalized at once
- A temporary table with a pinned value: `f([x=a])`, `await :close [h=dlg]`
- An escape caught: `invalid escape : pinned value`

## 8. Implementation

- Compilation to C; reference counting, and no tracing collector by choice
- What reference counting still does
    - a dropped unpinned value: data at count zero; a task at count one,
      aborted at the end of the statement, since an abort runs code
    - an emitted event: owned by the runtime, released by its count
    - a value removed from a pinned holder: freed before the block ends
    - values with no block at all, such as strings
- Header of a value: reference count, state, block depth, line of the pin,
  ceiling and its line
- States in the header: unpinned, unpinned with a floor, pinned by declaration,
  pinned by a holder, owned by the runtime
- Rule 1: constructors and `++` raise the floor at birth
- Rule 2: `a_pin`, and the deep pin on a store into a pinned value
- Rule 3: `a_chk_store`, one comparison of depths per store
- Rule 4: `a_pin_end`, `a_pin_clr`; tasks aborted first, then data released
- Rule 5: `a_blk_xfer`, refuses a pinned value that would leave; transfers an unpinned one
- Cycles at block end: local trial deletion
    - candidates: unpinned values reachable from the block's names
      (ceiling at that block, or none)
    - subtract references among candidates and from the names; what keeps a
      count is alive, with all it reaches; what counting would free is left to it
    - cut only the edges among the rest (in a cycle, or reached only through one)
    - runs only if a reference was written into a table, box, or `pub` in the block
    - soundness rests on the counts, not on the ceiling
    - escape information by block depth selects the candidates
- Ceiling: `a_name` at each binding, deep; `a_unname` at the end of a block;
  the check before an adoption walks the unpinned value, with marks for cycles
- Retired values: a list that keeps one reference; released at count one,
  the rest finalized at the end of the block (`a_retire`, `a_retired_end`)
- Approximations: a ceiling kept after a `var` is reassigned; parallel trails
  at the same depth compare depths, not blocks
- A block that waits for a task that is unwinding
- A dropped task that is running: ends with the trail that spawned it
- Tasks held by pinned data: aborted with the block of the holder
- Natives from C: `a_nat_holds(child, parent)`, `a_own`
- Errors: the type of the value and the line of its pin
- Compile-time warning of a pin given away: in a given constructor, and in
  `return`, `break`, `escape` with a literal tag

## 9. Evaluation

- Case studies: six ports
    - pico: tables as arguments, `f([x=a])`
    - IUP: tables in await patterns and events, modal dialogs
    - Flare: tables of modules, `pub` of the task itself
    - Lessmilk: variables captured by functions
    - SNKRX: many short-lived entities and pools
    - libuv: accepted connections pinned in a loop, living in a pool
- Friction found under the first rules, and how the new rules answer it
- Workarounds removed in each port after the migration
- Cost of the migration (under the first rules, before the ceiling)
    - test suite: about 230 of 830 programs changed
    - after the five decisions of 2026-11-17: 18 more tests; in the ports,
      pins given away by a block (a lower bound): SNKRX 36, Lessmilk 16,
      Flare 13, Pingus 8, pico-1oom 4, others 0 to 3
    - mostly `val`/`var` to `pin`; the rewrites that a keyword does not solve
    - to measure again: names may hold unpinned values, and owned places may be
      reassigned, so many of those rewrites are no longer needed
- Cost of the local trial deletion: none on binary-trees or when only numbers
  are written; +30 to 40% (~150 ns per iteration) on a loop that only writes
  a new table into a named one; sdl-pingus (suite, 6000-frame demo) and the
  libuv echo benchmark unchanged
- Case E, a longer-lived place referring to what a shorter block owns:
  experiments with a context owned by the block
    - Flare: 4 of 5 workarounds gone, -32 lines, traces identical
    - Lessmilk: registry gone, 16 of 22; +3% code; the limit reappears one
      level down (entities inside the screen)
    - pico: only its two C globals
    - conclusion: most of case E is per-scope state kept in globals
    - what remains: a task outside the block cannot receive its context; a
      container cannot hold what an inner entity pins; global events carry ids
- Cost of the ceiling: 2,000,000 `pin` of two small tables,
  0.95 s against 0.75 s without the check; the same loop with `val`, 0.75 s
- Reports of the migration, so far (`261115-pin.md`)
    - sdl-tux: 9 lines to `pin`, 1 rewrite
    - doom: 8 lines to `pin`, 2 rewrites
    - SDL environment: 16 lines to `pin`, 4 rewrites
    - libuv environment: 25 lines to `pin`, 13 rewrites; workaround gone in 10 places
    - IUP environment: 66 lines to `pin`, 4 rewrites; workaround gone in 12 places
    - outputs of the tests identical in all five
    - pending: pico, Flare, Lessmilk, SNKRX
- What remains strange, from the reports
    - a record filled and then given away: created in its place first
    - two resources opened in parallel need a table to hold them
    - `pin ok, e = catch`: a `pin` on names that are not dynamic
    - a helper that returns a small table: a `pin` for a value read once
    - errors only show on the paths that execute
- Quality of errors: what escaped, from where, to where
- Cost
    - time: the store check, the transfer of unpinned results
    - space: bits per value
    - comparison with a GC and with plain RC
- Planned experiment: the Computer Language Benchmark Game (CLBG)
    - the 7 programs of Câmara (2024): binary-trees, fannkuch-redux, fasta,
      fibonacci, mandelbrot, n-body, spectral-norm
    - Atmos against Lua 5.4, and against Atmos with the checks disabled
      (a compile flag: removes only the checks, keeps end-of-block finalization)
    - methodology fix: compile once, time only the run (the earlier study
      included recompilation in every run)
    - questions: overhead of the checks; where it concentrates (binary-trees)
- Two ends of expressiveness
    - CLBG: algorithms translated from Lua with no change (dyn-lex, 2024)
    - the ports: games with tasks, registries, module state; about 240 rewrites
- Leak freedom: test suite under a memory checker
- Expressiveness: programs that needed restructuring, and how much

## 10. Limitations

- The check is dynamic: errors appear when the program executes
- Shared structures must be owned by a common enclosing block
- No weak references
- Collection is still needed during the scope: pins do not replace a GC
- Without a tracing collector, garbage in cycles lives as long as its block
- A long-lived block (the main loop, a module) may accumulate such garbage
- Cycles that leak: a cyclic value returned by a call and dropped without a
  name; a cyclic value given to a task of an outer pool, when that task ends
- No way for an outer place to hold an inner value (case E); proposals
  (`with x.k = v {...}`, a `:weak` set) are under discussion, not decided
- A task cannot publish a value pinned in its body
- An unpinned value outside its window is an error: `invalid pin : table named at f:N`
- A native replaced in an owned place is closed when its last holder lets it go:
  deterministic, but not a point of the text
- An event is shared: a receiver cannot pin it
- A value that leaves is transferred deep: cost proportional to its size
- Requires a language with structured tasks and abortion

## 11. Related Work

- Prior work by the same author: dyn-lex (2023)
    - every dynamic value attached, with no keyword, to the block of its first
      assignment; tasks and coroutines too; all checks at run time
    - states: FLEET (not assigned yet: the ancestor of unpinned),
      MUTAB (attached, may still be narrowed), IMMUT (captured, cannot move)
    - `drop(v)`: an explicit move, single reference, source set to nil;
      Atmos has `func f (pin x)` in its place
    - `:tmp` names that hold without attaching, for `and`/`or`: the problem
      Atmos solves with `val` without owner and the ceiling
    - its test `ll_05_nest` fails on the hole that the ceiling closes
    - what Atmos changes: an explicit `pin`, the floor and the ceiling,
      reassignment of owned places, the same rules for tasks
- Câmara, undergraduate thesis, UERJ (2024): dyn-lex against Lua 5.4 on 7
  CLBG programs
    - expressiveness: the programs translated with no change of algorithm
    - cost of the checks against a variant without them; reference counting
      doubles each pointer; cycles leak
    - its five rules, for a side-by-side with the Atmos rules
    - known flaw: times include recompilation; cite the method and the
      qualitative findings, not the numbers
- Structured concurrency: Trio, Kotlin, Swift, Java Loom, Céu
- Regions: Tofte-Talpin, MLKit, Cyclone, Real-Time Java scoped memory
- Ownership: Rust, ownership types, Vale, Lobster
- Reference counting: Swift ARC, Perceus, cycle collection, trial deletion
- Resource management: RAII, `defer`, `with`, `using`, `try-with-resources`
- Arenas and zones: Zig, Odin, Apache pools
- Escape analysis and stack allocation
- Synchronous languages: Esterel, Céu, finalization on abortion

## 12. Future Work

- Static detection of escapes, for a subset of the checks
- Elision of the dynamic check where the block is known
- Formal semantics and a proof of no dangling references
- An optional tracing collector, scoped to a block
- Scoped assignment for module tables and global state: `with m.x = v {...}`

## 13. Conclusion

- One rule for tasks, data, and resources
- The blocks of the program are its memory regions
- Structure makes memory management simple, and memory completes the structure
