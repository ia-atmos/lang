# Dynamic Values Deserve a Home: Structured Memory with Lexical Pins

Name of the mechanism: **structured memory**. Its construct is the
**lexical pin** (`pin`), which gives a dynamic value a home: a block.

> Status: outline. The rules in Section 4 were declared stable by the
> author on 2026-11-19, as implemented in `main` at `8361f88` and described
> in `doc/manual.md` ("Lexical Memory Management", "Pin", "Set"). The
> collection of unpinned cycles was later removed (atmos-pin#18, on `main`
> at `f999c52`): a cyclic structure must be pinned. History:
> `.claude/plans/261115-pin.md`, `.claude/plans/TODO-pin.md`.

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
that no use of a pinned value outlives its block: a reference may outlive it
only through an explicit `set :out`, and using it then is an error.
A pinned value never leaves its block; a block gives away only values it has
not pinned.
The mechanism needs no static lifetime annotations and no separate region or
arena abstraction: the blocks of the program are the regions.
It bounds the lifetime of a value, but does not collect what becomes garbage
while its block is alive: reference counting does that underneath, and
its counts are part of the semantics: a value that is no longer referred to
is finalized at once, at a deterministic point.
The safety of the pins does not read a count.
With counting, a cyclic structure must be pinned, and is broken at the end
of its block; a tracing collector would also collect the cycles that are
not pinned, which counting leaks.
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
    - an implementation on reference counting: the checks compare block
      depths and never read a count; the counts are part of the semantics,
      and give a value that nobody owns a deterministic end
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
    - protects finalization order and forbids any use of a finalized value
    - invariant: no use outlives the block; a reference may, only by `set :out`
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
    - exception, `set :out place = value`: an outer place refers to a pinned
      value of the current block, with no check and no adoption, and is reset
      to `nil` when the block of the `set` ends (case E); the place may be a
      `pub`, whole or a field (`set :out pub = v`)
- Rule 4, end
    - pinned values finalized in reverse order, interleaved with `defer`
    - a dropped unpinned value is finalized at once
    - table released, native closed, task aborted
    - finalized data is marked: any later use raises
      `invalid access : finalized <type> pinned at f:N`, so a reference that
      outlived its scope is caught instead of reading an empty value
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
    - cycles among unpinned values: not collected; with
      reference counting, they leak, so a cyclic structure must be pinned
      (by its block or by a pinned value that holds it); a function that
      builds one returns it unpinned and the caller pins it
    - a recursive closure through a variable is a cycle: `func` or `pin var`;
      the compiler warns on `var g; set g = func ... g ...`
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
    - the safety of the pins does not depend on counting; but the counts
      are part of the semantics: a tracer would lose the deterministic end of
      a value that nobody owns, such as a dropped task or native
    - pins give what a collector does not: a known point of finalization
    - a collector gives what pins do not: garbage reclaimed inside a live block
    - Atmos drops the collector as a tradeoff: no pauses, no heap scans
    - the price: unreachable data waits for the end of the block, and with
      counting an unpinned cycle leaks
- vs. plain reference counting
    - pinned cycles are reclaimed at the end of their block
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

- Compilation to C; reference counting, and no tracing collector
- What reads a count and what does not: the checks compare depths and
  finalization at block end is driven by the block; only the end of an
  unowned value reads a count (a dropped value, a retired value, a replaced
  native), and there the counts are the semantics: finalized at once, at a
  deterministic point (atmos-pin#29)
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
- Cycles: a pinned cycle is broken at the end of its block, by the pin; an
  unpinned cycle is not collected and leaks (no cycle detection, by the
  author's choice: atmos-pin#18; a local trial deletion was implemented and
  removed, `f999c52` on `main`)
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
  `return`, `break`, `escape` with a literal tag, `until c, v`, `while c, v`

## 9. Evaluation

- Case studies: six ports
    - pico: tables as arguments, `f([x=a])`
    - IUP: tables in await patterns and events, modal dialogs
    - Flare: tables of modules, `pub` of the task itself
    - Lessmilk: variables captured by functions
    - SNKRX: a map body that is a task in a world made by the caller;
      a registry of springs (both by `set :out`)
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
- Case E, a longer-lived place referring to what a shorter block owns:
  experiments with a context owned by the block
    - Flare: 4 of 5 workarounds gone, -32 lines, traces identical
    - Lessmilk: registry gone, 16 of 22; +3% code; the limit reappears one
      level down (entities inside the screen)
    - pico: only its two C globals
    - conclusion: most of case E is per-scope state kept in globals
    - what remains: a task outside the block cannot receive its context; a
      container cannot hold what an inner entity pins; global events carry ids
    - answered by `set :out` (2026-11-19): SNKRX's map body and springs,
      Flare's hero keeping the enemy under the mouse, Lessmilk's registries
    - alternatives the author rejected: a `with` block restoring the old value
      (unsound with `par`); a `:weak` set; a check of who reads; a context
      owned by the block; a field plus `defer` to nil (a reader outside keeps
      the value alive and running, silently)
- Cost of `set :out` and the mark of finalized data: none measured
  (20 million table reads, 0.70 s before and 0.68 s after, `TODO-pin.md`);
  12 ports pass unchanged
- Cost of the checks on a real program (Doom demo1, instructions): 263.7 M
  against 256.3 M with `ATMOS_PIN=off`, 2.8% of the run and about 13% of what
  Atmos adds over the C engine; 0.2% with rendering (doom `doc/bench.md`)
- Cost in code size (env-esp32, board build, program plus runtime): with the
  checks against `ATM_PIN_OFF`, `init` 140,631 against 136,037 bytes (3.3%
  less without), `ultrat` 215,219 against 198,269 bytes (7.9% less without);
  `unchecked` in between; static DRAM -40 bytes; heap and speed not measured
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
    - five variants, and one reference with a JIT:
        - C: the floor (the C versions in `alochaus/dyn-lex-benchmark`)
        - Lua 5.4: the same language model, interpreted, with a tracing GC
        - LuaJIT: the ceiling of a dynamic language; its speed comes from the
          JIT, not from memory, so it is context only; Lua 5.1 dialect, and any
          adaptation of the programs is reported
        - Atmos `ATMOS_PIN=off`: counting only, `pin` only for tasks and pools
        - Atmos `ATMOS_PIN=unchecked`: the same semantics, no checks
        - Atmos with `pin` (default): the whole mechanism
    - the figure of the pin is Atmos with against Atmos without; C, Lua and
      LuaJIT only place it, since they differ in much more than memory
    - measure time and peak memory (RSS): counting, tracing, and pins differ
      in memory more than in time
    - only binary-trees allocates much; Doom stays the main measure of cost
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
- Without a tracing collector, garbage in a pinned cycle lives as long as its block
- A long-lived block (the main loop, a module) may accumulate such garbage
- Cycles, an artifact of reference counting: an unpinned cycle leaks; a
  cyclic structure must be pinned, by its block or by a pinned value that
  holds it (a tracer would remove this limitation)
- `set :out` refers outward with no check: its place is reset only at the end
  of the block, so a reader in between must not keep what it reads
- A task cannot publish a value pinned in its body
- An unpinned value outside its window is an error: `invalid pin : table named at f:N`
- A native replaced in an owned place is closed when its last holder lets it go:
  deterministic, but not a point of the text
- An event is shared: a receiver cannot pin it
- A value that leaves is transferred deep: cost proportional to its size
- Requires a language with structured tasks and abortion

## 11. Related Work

- Prior work by the same author: dyn-lex (2023; checked against its
  repository, last commit 2023-10-24: `doc/manual.md`, `src/main/kotlin/`)
    - every dynamic value attached, with no keyword, to the block of its first
      assignment; tasks and coroutines too; all checks at run time
    - states: FLEET (not assigned yet: the ancestor of unpinned),
      MUTAB (attached, may still be narrowed), IMMUT (captured, cannot move)
    - `drop(v)`: an explicit move, single reference, source set to nil;
      Atmos has `func f (pin x)` in its place
    - `:tmp` names that hold without attaching, for `and`/`or`: the problem
      Atmos solves with `val` without owner and the ceiling
    - its test `ll_05_nest` (`src/test/kotlin/TExec.kt`) expects the error
      "cannot move to deeper scope with pending references": the hole that
      the ceiling closes
    - `drop` applies recursively, only to assignable expressions with a
      single reference (its manual, "Drop")
    - what Atmos changes: an explicit `pin`, the floor and the ceiling,
      reassignment of owned places, the same rules for tasks
- Câmara, undergraduate thesis, UERJ (2024): dyn-lex against Lua 5.4 on 7
  CLBG programs
    - expressiveness: the programs translated with no change of algorithm
    - cost of the checks against a variant without them; reference counting
      doubles each pointer; cycles leak
    - its five rules, for a side-by-side with the Atmos rules
    - known flaw (the author): times include recompilation; cite the method
      and the qualitative findings, not the numbers
    - remark (#12): the published scripts of `alochaus/dyn-lex-benchmark`
      compile once and time only the executable; the flaw is not visible
      there, so it may lie in the runs of the thesis, not in the scripts
    - its repository has 8 programs: the 7 named plus `simple` (a sum loop)
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
- A tracing collector beside reference counting, for unpinned cycles only,
  keeping the counts as the semantics of everything else
- Scoped assignment for module tables and global state: `with m.x = v {...}`

## 13. Conclusion

- One rule for tasks, data, and resources
- The blocks of the program are its memory regions
- Structure makes memory management simple, and memory completes the structure
