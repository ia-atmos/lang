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
that no reference to a pinned value outlives its block, and a block may still
give a value away as its result.
Reference counting remains underneath, but the mechanism needs no tracing
collector, no static lifetime annotations, and
no separate region or arena abstraction: the blocks of the program are the
regions.
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
    - an implementation with no tracing collector
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
- A block may give a value away as its result
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
    - fresh: not owned yet, the result of a constructor, call, or block
    - a fresh value has a floor: the innermost block of what it holds
    - it cannot leave the floor; it may be pinned there or deeper
- Rule 1, birth
    - a value is born fresh
    - floor from what it is born with: fields, operands, captures, arguments
    - a native declares what it depends on in its binding
    - the floor never rises after birth
- Rule 2, pin
    - by declaration: `pin`, in the current block
    - by storage: in a pinned value, in the block of the holder
    - deep: every fresh value it holds is pinned with it
- Rule 3, store
    - a place accepts a value of the same block or of an outer one
    - places: table field, `pub`, outer variable, captured variable, pool
    - a fresh place is checked against its floor
    - `emit` and `throw` follow the same check
    - a captured variable belongs to the block that declares it
- Rule 4, end
    - pinned values finalized in reverse order, interleaved with `defer`
    - a dropped fresh value is finalized at once
    - table released, native closed, task aborted
- Rule 5, result
    - the only way to undo a pin: to be the result of the block
    - last expression, `return`, `break`, `escape`
    - the value becomes fresh in the outer block, with what it holds
    - error if it holds something that cannot leave
- Consequences
    - the first declaration is always `pin`; `val` only aliases
    - a parameter may receive a fresh value: pin it, store it, or return it
    - the work block: `pin r = do { pin tmp = ... ; pin x = ... ; x }`
    - a work block as an argument: `spawn @ts H(do { pin c = accept() ; c })`
    - natives: a child holds its parent, by rule 3
- Refinements found in the implementation
    - an emitted event: owned by the emit, at its floor, by no block
    - a task holds its `pub` and its final value
    - a task is pinned from birth by the block that owns it
    - declarations of functions and tasks pin their prototypes
    - parameters and loop variables may hold fresh values
- Semantics sketch: block depth as the lifetime order

## 5. Why It Matters as a Memory Mechanism

- Criteria: determinism, cycles, resources, annotations, pauses, cost, errors
- vs. tracing GC
    - no pauses, no heap scans, no finalizer nondeterminism
    - resources released at a known program point
    - fits embedded and real-time targets
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
- A constructor: a function that gives a pinned value away
- A pool of tasks pinned outside a loop
- A work block: temporaries die, the result leaves
- A fresh value dropped: finalized at once
- A temporary table with a pinned value: `f([x=a])`, `await :close [h=dlg]`
- An escape caught: `invalid escape : pinned value`

## 8. Implementation

- Compilation to C, no tracing collector in the runtime
- What reference counting still does: dropped fresh values, events, early release
- Header of a value: reference count, state, block depth, line of the pin
- States in the header: fresh, fresh with a floor, pinned by declaration,
  pinned by a holder, owned by the runtime
- Rule 1: constructors and `++` raise the floor at birth
- Rule 2: `a_pin`, and the deep pin on a store into a pinned value
- Rule 3: `a_chk_store`, one comparison of depths per store
- Rule 4: `a_pin_end`, `a_pin_clr`; tasks aborted first, then data released
- Rule 5: `a_blk_xfer`, transfer of a result, deep, all or nothing
- A block that waits for a task that is unwinding
- A dropped task that is running: ends with the trail that spawned it
- Tasks held by pinned data: aborted with the block of the holder
- Natives from C: `a_nat_holds(child, parent)`, `a_own`
- Errors: the type of the value and the line of its pin

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
- Cost of the migration
    - test suite: about 230 of 830 programs changed
    - mostly `val`/`var` to `pin`; the rewrites that a keyword does not solve
- What remains strange or needs a workaround, from the reports of the ports
- Quality of errors: what escaped, from where, to where
- Cost
    - time: the store check, the transfer of results
    - space: bits per value
    - comparison with a GC and with plain RC
- Leak freedom: test suite under a memory checker
- Expressiveness: programs that needed restructuring, and how much

## 10. Limitations

- The check is dynamic: errors appear when the program executes
- Shared structures must be owned by a common enclosing block
- No weak references
- Reference counting is still required: not "no GC", but no tracing GC
- A variable does not own: `var x` cannot receive a fresh value later
- A value sometimes fresh and sometimes owned fits neither `val` nor `pin`
- An event is shared: a receiver cannot pin it
- `pin` on every first declaration: more annotations than `val`
- A value that leaves is transferred deep: cost proportional to its size
- Long-lived blocks keep what they pin until the end
- Requires a language with structured tasks and abortion

## 11. Related Work

- Structured concurrency: Trio, Kotlin, Swift, Java Loom, Céu
- Regions: Tofte-Talpin, MLKit, Cyclone, Real-Time Java scoped memory
- Ownership: Rust, ownership types, Vale, Lobster
- Reference counting: Swift ARC, Perceus, cycle collection
- Resource management: RAII, `defer`, `with`, `using`, `try-with-resources`
- Arenas and zones: Zig, Odin, Apache pools
- Escape analysis and stack allocation
- Synchronous languages: Esterel, Céu, finalization on abortion

## 12. Future Work

- Static detection of escapes, for a subset of the checks
- Elision of the dynamic check where the block is known
- Formal semantics and a proof of no dangling references
- Scoped assignment for module tables and global state: `with m.x = v {...}`

## 13. Conclusion

- One rule for tasks, data, and resources
- The blocks of the program are its memory regions
- Structure makes memory management simple, and memory completes the structure
