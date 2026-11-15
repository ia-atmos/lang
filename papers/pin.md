# Every Value Has a Block: Lexical Pinning for Structured Concurrent Programs

Proposed name of the mechanism: **lexical pinning** (`pin`).

Alternative titles:

- Pin: Block-Owned Memory without a Garbage Collector
- Structured Memory: Values that Live and Die with their Blocks
- Lexical Memory Management for Structured Concurrency

> Status: outline. The rules in Section 4 follow the redesign under
> discussion ("Pin rules discussion"), not yet approved; the current rules
> are in `doc/manual.md` ("Lexical Memory Management", "Pin").

## Abstract

Structured concurrency ties the lifetime of a task to a lexical block: the
task cannot outlive the block, and is aborted when the block terminates.
We extend this discipline from tasks to all dynamic values.
In Atmos, a `pin` declaration attaches a table, function, native resource,
task, or pool to its enclosing block.
The block finalizes what it pins when it terminates or aborts, in reverse
order and interleaved with `defer`, which reclaims cycles and releases
resources at a known point of the program.
A dynamic check, a block depth per value compared on every store, guarantees
that no reference to a pinned value outlives its block, and a block may still
give a value away as its result.
The mechanism needs no tracing collector, no static lifetime annotations, and
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
- Thesis: one lexical rule for tasks, data, and resources
- Contributions
    - the mechanism: lexical pinning
    - a small set of rules, uniform for all dynamic values
    - a cheap dynamic check: one depth per value, one comparison per store
    - an implementation with no collector
    - an evaluation with six real ports

## 2. Background

- Structured concurrency: blocks own tasks, nurseries, scopes
- Synchronous reactive languages: Esterel, Céu, abortion as a primitive
- Atmos in one page: tasks, `await`, `emit`, `par`, `watching`, `defer`
- Dynamic values: tables, functions, natives, tasks, pools
- Baseline: reference counting, no cycles reclaimed, no known death point

## 3. The Idea

- Every dynamic value belongs to a block
- `pin x = e`: the enclosing block owns the value
- The block finalizes its values when it terminates or aborts
- No reference to a value outlives its block
- A block may give a value away as its result
- Tasks, data, and resources follow the same rule
- Motto: a task, its data, and its block live and die together

## 4. The Rules

- One principle
    - a value may only be stored where its block lasts at least as long
- Two states
    - pinned: owned by a block
    - fresh: not yet owned, with a floor block it cannot leave
- Five rules
    - birth: a new value is fresh, with a floor
    - pin: by declaration, or by being stored in a pinned value (deep)
    - store: checked against the block of the destination
    - finalization: at block end; a dropped fresh value, at once
    - un-pin: only as the result of a block
- Uniformity: tasks and pools obey the same rules
- `val` only aliases; the first declaration is always `pin`
- Channels of escape, all checked
    - assignment, capture, table field, `pub`
    - `emit`, `throw`, pool, argument of a task
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
    - pinning owns tasks and what tasks use
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
- Claim: structured concurrency is necessary, and pinning completes it

## 7. Examples

- A cycle reclaimed at block end
- A native resource: a file closed at block end
- A task and its data aborted together by `watching`
- A constructor: a function that gives a pinned value away
- A pool of tasks pinned outside a loop
- An escape caught: `invalid escape : pinned value`

## 8. Implementation

- Compilation to C, no collector in the runtime
- Header of a value: reference count, pinned bit, block depth
- `a_pin`: pin at the current depth, requires a fresh value
- `a_pdepth`: effective depth of a value, tasks and pools included
- `a_chk_store`, `a_chk_tab`: one comparison per store
- `a_blk_xfer`: transfer of a result, deep, all or nothing
- `a_pin_end`, `a_pin_clr`: abort tasks first, then release data
- A block that waits for a task that is unwinding
- Functions that capture pinned values; tasks that receive them

## 9. Evaluation

- Case studies: six ports
    - pico: tables as arguments, `f([x=a])`
    - IUP: tables in await patterns and events, modal dialogs
    - Flare: tables of modules, `pub` of the task itself
    - Lessmilk: variables captured by functions
    - SNKRX: many short-lived entities and pools
    - libuv: accepted connections pinned in a loop, living in a pool
- Friction found, and how the rules answer each case
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
- Scoped assignment: `with m.x = v {...}`

## 13. Conclusion

- One rule for tasks, data, and resources
- The blocks of the program are its memory regions
- Structure makes memory management simple, and memory completes the structure
