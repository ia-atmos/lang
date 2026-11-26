# Dynamic Values Deserve a Home: draft text

> DRAFT, by the session `paper-pin`, to be confirmed by the author (#11).
> Text for every section of `papers/pin.md`, which the author let go
> ahead on 2026-11-25 ("desbloqueado, vá em frente"). The rules are those
> of `doc/manual.md` on `main` ("Lexical Memory Management", "Pin",
> "Set"). Numbers come either from the sessions that measured them, with
> their source in brackets, or from `papers/pin-bench/`, measured by this
> session (Section 9, CLBG).

## 1. Introduction

Reactive programs run for a long time, react to many sources of events, and
keep many activities alive at once: a game keeps its entities, a server its
connections, an interface its dialogs. Each activity owns data and
resources, and the data in turn refers to activities: an entity holds the
task that animates it, a registry holds the tasks that listen to it, a task
holds the file that it writes. Their lifetimes are entangled, and most of
the defects of such programs live in that entanglement: a task that keeps
running after its entity is gone, a callback that fires on a closed
dialog, a file that stays open because something still refers to it.

Structured concurrency solved half of the problem. A task belongs to the
block that starts it, cannot outlive it, and is aborted with it, so the
lifetime of every task can be read in the text of the program. The data
and the resources that tasks use were left outside that discipline. They
are managed by a tracing collector, by reference counting, or by hand, and
they live as long as some reference to them survives, which the text of
the program does not show.

This paper extends the discipline of structured concurrency to every
dynamic value, and calls it *structured memory*. In Atmos, a structured
reactive language, a table, a function, a native resource, a task, and a
pool of tasks each belong to a block. The construct that attaches a value to
its block is the *lexical pin*, `pin`. The block finalizes what it pins when
it ends, normally or by abortion, in reverse order and interleaved with
`defer`. A check at each store, which compares two block depths, guarantees
that no use of a value outlives its block.

The contributions are:

- the mechanism, structured memory, with a single construct, the lexical
  pin, for tasks, data, and resources alike;
- a small set of rules, uniform for all dynamic values (Section 4);
- a cheap dynamic check: one depth per value, one comparison per store;
- an implementation on reference counting (Section 8), in which the checks
  never read a count, while the counts give a value that nobody owns a
  deterministic end, at the moment it is dropped;
- an evaluation on ports of real programs, games and environments, and on
  the benchmarks of the Computer Language Benchmarks Game (Section 9).

## 2. Background

**Structured concurrency.** In structured concurrency, a block that starts
tasks waits for them, or cancels them, before it ends: the nurseries of
Trio, the scopes of Kotlin and Swift, the structured task scopes of Java.
The lifetime of a task is nested in the lifetime of the block that starts
it, as the lifetime of a local variable is nested in its block.

**Synchronous reactive languages.** Esterel and Céu go further: a block
may be aborted from the outside, at any of its points of suspension, and
abortion is a primitive of the language, with a deterministic order of
finalization. Execution proceeds in reactions to events, each of which runs
to completion with no preemption.

**Atmos.** Atmos is a dynamic language in the family of Lua and Céu. Its
tasks await events (`await`), signal them (`emit`), run in parallel in the
same block (`par`, `par :any`, `par :all`), and are aborted when an event
occurs (`watching`). A `defer` block runs when its enclosing block ends,
for any reason. Tasks may live in pools, which a block owns.

**Dynamic values.** Atmos has five kinds of values that live in the heap
and are shared by reference: tables, functions (with what they capture),
native values (resources of C, such as files and windows), tasks, and pools
of tasks. Numbers, booleans, tags, and strings are values with no identity
and need no owner.

**Baseline.** Underneath, the runtime counts references and has no tracing
collector. Counting alone reclaims what is no longer referred to, at a
deterministic point, but it does not reclaim cycles, and it does not tie the
death of a value to a point of the program text: a value lives as long as
its last reference, wherever that is.

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
refers to that value. The place may be a `pub`, whole or a field.

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

## 5. Why It Matters as a Memory Mechanism

We compare structured memory with the mechanisms it borrows from, along
seven criteria: whether the point of death is deterministic, whether
cycles are reclaimed, whether resources other than memory are released,
whether the programmer writes annotations, whether there are pauses, what
it costs at run time, and when errors are reported.

**Tracing collectors.** Structured memory does not replace a collector, and
the two could be combined. A collector reclaims garbage while its block is
still alive; pins do not. Pins give what a collector does not: a known point
at which a value is finalized, which matters for tasks and native resources
more than for memory. Atmos leaves the collector out by choice, for the
absence of pauses and of scans of the heap. The price is that unreachable
data waits for the end of its block, and that an unpinned cycle leaks. The
safety of the pins does not depend on counting, but the counts are part of
the semantics: a collector would keep the safety and lose the deterministic
end of a value that nobody owns, such as a dropped task.

**Plain reference counting.** Pins add three things to counting: a cycle
that is pinned is reclaimed at the end of its block; the point of death is
lexical, not "when the last reference goes"; and finalization aborts tasks
and closes resources, not only frees memory.

**Ownership and borrowing.** Rust checks lifetimes statically, with
annotations and a borrow checker. Pins need no annotation, and aliasing is
free inside a block: any number of names may refer to a value, and cyclic
or shared structures need no `Rc`, `Weak`, or `unsafe`. The cost is that
errors are reported when the program runs, not when it compiles.

**Regions.** In the regions of Tofte and Talpin, and in Cyclone, values are
allocated in regions with a lexical lifetime, inferred or annotated, and
functions are polymorphic over regions. With pins, the blocks of the
program are the regions: there are no region variables and no inference,
values move out of a block as its result, and finalization runs code
rather than only freeing memory.

**RAII and `defer`.** RAII and `defer` give the same point of death for a
resource. Pins add a guarantee that no reference outlives that point, one
order of finalization for `pin` and `defer`, and the same behavior on
abortion from the outside as on a normal exit.

**Arenas.** An arena frees all its values at once, as a block frees what it
pins. But an arena must be passed through the code that allocates, and it
works per allocator, not per value: a value cannot leave its arena. A pin
works per value, needs no parameter, and a value that is not pinned may
leave its block.

**Nurseries and scopes.** The scopes of structured concurrency own tasks
only; the data of the tasks still relies on a collector. Structured memory
owns tasks and what the tasks use, with the same rule.

| | point of death | cycles | resources | annotations | pauses | errors |
|---|---|---|---|---|---|---|
| tracing GC | no | yes | no (finalizers) | no | yes | none |
| reference counting | last reference | no | yes | no | no | none |
| Rust ownership | lexical | with `Weak` | yes (`Drop`) | lifetimes | no | static |
| regions | lexical | yes | no | regions | no | static |
| RAII, `defer` | lexical | n/a | yes | no | no | none (dangling) |
| arenas | arena | yes | no | arena parameter | no | none (dangling) |
| nurseries | lexical (tasks) | n/a | tasks | no | no | none |
| structured memory | lexical | if pinned | yes | `pin` | no | dynamic |

## 6. Why It Requires Structured Programming

Structured memory is sound and useful only under structured programming.
Each of its rules relies on a property that structure gives.

- **Every point of execution has a block.** A value is pinned by the block
  in which it is declared. A language with detached threads, or with code
  that runs outside any block (a `go` statement, a callback registered
  globally), has values with no block to own them.
- **Blocks own tasks.** A task cannot hold a value past its own block,
  because the task itself ends with its block. Without that, a task could
  carry a value of a block that is gone.
- **Abortion is a primitive.** A block can always be terminated, with all it
  contains. This is what lets the end of a block be the end of its values,
  whether the block completes or is aborted from the outside.
- **Finalization is deterministic.** Values are finalized in reverse order,
  from the inside out, interleaved with `defer`, so a finalizer may rely on
  what was created before it.
- **Reactions are synchronous.** No task preempts another in the middle of a
  store, so the check of a store and the store itself are one step.
- **Lexical nesting orders lifetimes.** Because blocks nest, the lifetimes of
  blocks form a total order along any path of execution, and one integer per
  value, the depth of its block, is enough to compare two lifetimes.

Without structure, the mechanism breaks at each of these points: a
detached thread, a `go` statement, a promise that outlives its caller, a
callback in a global registry, a `goto` or an early exit that skips a
finalizer. Structured concurrency is therefore necessary for structured
memory, and structured memory completes it: the same blocks that own tasks
own what the tasks use.

## 7. Examples

Each example is a complete program, run on `main` with its output as shown
(`papers/pin-examples/`); none leaks under the address sanitizer.

**A cycle, reclaimed at the end of its block.** Counting alone would never
free `a` and `b`; their block does.

```
do {
    pin a = [10]
    pin b = [20]
    set a.other = b         ;; a <-> b
    set b.other = a
    print(a.other@0)        ;; --> 20
}                           ;; a and b are reclaimed here
```

**A native resource.** A file is a native value; its block closes it, and
so flushes it, when the block ends.

```
do {
    pin f = io.open("/tmp/pin-ex2.txt", "w")
    io.write(f, "hello\n")
}                           ;; the file is closed here
print(io.read(io.open("/tmp/pin-ex2.txt"), :all))   ;; --> hello
```

**A task and its data, aborted together.** The task owns its log; when
`:stop` aborts the block of `watching`, the task is aborted, its `defer`
runs, and the log is finalized with it.

```
task Blinker (name) {
    pin log = [name]
    defer {
        print("finalized", log@0, #log)
    }
    loop on :tick {
        set log@+ = :blink
    }
}
spawn {
    watching :stop {
        pin b = spawn Blinker("led")
        await :never
    }
    print(:stopped)
}
emit :tick
emit :tick
emit :stop                  ;; --> finalized  led  3
                            ;; --> stopped
```

**A constructor.** A function builds a value with `val` and gives it away,
unpinned, as its result; the caller decides who owns it.

```
func point (x, y) {
    val p = [x=x, y=y]
    set p.norm = math.sqrt(x*x + y*y)
    p
}
do {
    pin q = point(3, 4)
    print(q.norm)           ;; --> 5.0
}                           ;; q is reclaimed here
```

**A pool of tasks, pinned outside a loop.** The tasks live in the pool, not
in the body of the loop, and end with the block of the pool.

```
task Conn (id) {
    defer {
        print("closed", id)
    }
    await :never
}
do {
    pin conns = tasks()
    loop i in 3 {
        spawn @conns Conn(i)
    }
    print("open", #conns)   ;; --> open  3
}                           ;; --> closed 0, closed 1, closed 2
```

**An escape, caught.** Storing a pinned value in a place of an outer block
is the error that the check prevents.

```
var keep = nil
do {
    pin t = [1, 2, 3]
    set keep = t            ;; ERROR: invalid escape : table pinned at ex6-escape.atm:4
}
```

**An explicit outward reference.** `set :out` lets an outer place refer to
an inner value; the place is reset when the value ends.

```
var cur = nil
do {
    pin dlg = [title="Save"]
    set :out cur = dlg
    print(cur.title)        ;; --> Save
}
print(cur)                  ;; --> nil
```

## 8. Implementation

Atmos compiles to C. The runtime counts references and has no tracing
collector. The checks compare block depths and never read a count, and
finalization at the end of a block is driven by the block. Only the end of a
value that nobody owns reads a count, and there the counts are part of the
semantics: a value that is no longer referred to is finalized at once, at a
deterministic point, so a dropped task is aborted, and a dropped native
closed, at the moment it is dropped. A tracing collector would also collect
the cycles that counting leaks, but would lose that moment. Each object header carries its reference count, a state byte, the
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

Counting alone does not reclaim cycles. A pinned cycle is broken at the end
of its block, since the pin cuts what it finalizes. An unpinned cycle is not
collected: its counts never reach zero, and it leaks. A cyclic structure
must therefore be pinned, by its block or by a pinned value that holds it; a
function that builds one returns it unpinned, and the caller pins it. A
recursive closure through a variable is such a cycle, and is declared with
`func` or `pin var`; the compiler warns when a variable is assigned a
function that refers to it. This is a choice: a local trial deletion was
implemented and then removed (atmos-pin#18, `f999c52` on `main`), and a tracing
collector would make the restriction unnecessary.

An `emit` that carries a pinned value compares two depths, where the target
is attached and where the value is pinned, and does not walk the tree of
tasks. The compiler also warns at compile time when a pin is given away: in
a constructor that is itself given away, and in a `return`, `break`,
`escape` with a literal tag, `until c, v`, or `while c, v`.

## 9. Costs Measured

The figures below were reported by the sessions that implemented the rules,
except those of the CLBG, which were measured for this paper.

- **The ceiling check.** 2,000,000 `pin`s of two small tables took 0.95 s,
  against 0.75 s without the check; the same loop with `val` took 0.75 s
  (`.claude/plans/261115-pin.md`).
- **`set :out` and the mark of finalized data.** No cost: 20 million table
  reads took 0.70 s before and 0.68 s after, and binary-trees and the write
  loop were unchanged (`.claude/plans/TODO-pin.md`).
- **The ports.** On the final rules, 12 ports pass their suites unchanged
  (`atmos-lang#81`, merge `64005fd` into `main`).

- **The checks as a whole, on a real program.** Doom, demo1, counted in
  instructions on `main` at `34c381b`: with no rendering, 263.7 M with the
  checks against 256.3 M with `ATMOS_PIN=off` (the same generated C, linked
  against the runtime without the checks; its 16 tests give identical
  results under both). The checks cost 7.4 M instructions, 2.8% of the run,
  and about 13% of what the Atmos program adds to the engine written in C
  (207.9 M). With rendering: 4828.2 M against 4819.1 M, 0.2% (report of
  `atmos-lang`, 2026-11-21; doom `doc/bench.md`).

- **Code size, on a microcontroller.** env-esp32, board build (ESP32), the
  program with its runtime: `init` takes 140,631 bytes with the checks and
  136,037 without (`ATM_PIN_OFF`, 3.3% less); `ultrat` takes 215,219 and
  198,269 bytes (7.9% less). With `ATMOS_PIN=unchecked` (checks removed,
  semantics kept): 139,246 and 204,550 bytes. Static DRAM changes by 40 bytes
  in both. Its 6 host tests give the same output in the three modes; heap and
  speed on the board are not measured (report of `atmos-lang`, 2026-11-21,
  `libmain.a`, `lang` at `34c381b`).

**The checks on the CLBG (measured for this paper).** Seven programs of the
Computer Language Benchmarks Game, the ones of Câmara (2024), written with
the same algorithm and the same output in C, in Lua, and in Atmos
(`papers/pin-bench/`). Lua runs unchanged on Lua 5.4.6 and on LuaJIT 2.1.
Atmos is built three times from the same generated C: with
`ATMOS_PIN=off`, with `ATMOS_PIN=unchecked`, and with the whole mechanism.
Each program is compiled once and only the run is timed, 5 runs each, on
one x86-64 virtual machine with 4 cores (gcc 13.3, `-O2`). The machine is
shared and noisy (the runs of one program spread up to 32%), so the table
gives the best of the 5 runs, in seconds; medians are in `summary.md`.

| program | size | C | Lua 5.4 | LuaJIT | Atmos off | unchecked | Atmos | Atmos / off | Atmos / Lua 5.4 |
|---|---|---|---|---|---|---|---|---|---|
| binary-trees | 16 | 0.21 | 2.96 | 1.01 | 3.02 | 2.93 | 3.13 | 1.04 | 1.06 |
| fannkuch-redux | 9 | 0.02 | 0.25 | 0.05 | 0.74 | 0.76 | 0.76 | 1.02 | 3.04 |
| fasta | 1,000,000 | 0.15 | 1.32 | 0.48 | 1.64 | 1.64 | 1.75 | 1.07 | 1.33 |
| fib | 35 | 0.02 | 0.74 | 0.08 | 1.22 | 1.23 | 1.29 | 1.06 | 1.75 |
| mandelbrot | 1000 | 0.07 | 0.69 | 0.07 | 1.42 | 1.44 | 1.56 | 1.10 | 2.27 |
| n-body | 500,000 | 0.03 | 0.99 | 0.12 | 2.98 | 3.26 | 3.44 | 1.15 | 3.48 |
| spectral-norm | 500 | 0.01 | 0.40 | 0.02 | 0.75 | 0.72 | 0.78 | 1.03 | 1.96 |

These are the figures on `main` at `9ea77e1` (2026-11-26). A first run, two
days earlier (`results-1175264.csv`), gave 19% to 33% for the mechanism,
and showed where it went: fib allocates no table and paid about 30%. The
generated C was the same in the three builds; the difference was in the
inline functions of the runtime, which tested the type of each value that
was named, stored, or returned, and walked every slot of a block when it
ended, even when the slots held only numbers (15% more instructions on
fib, by callgrind). Two changes followed (atmos-pin#32): the end of a block
walks only its variables, and a value that can never be an object is
assigned with no test. The mechanism now costs 2% to 15% on these
programs. What remains is the parameters of each call (fib) and the
variables that only hold numbers (n-body).

Two more readings. The cost is in the checks, not in the semantics:
`unchecked`, which keeps end-of-block finalization and deep pins, runs as
fast as `off`, within the noise, except on n-body. And Atmos is slower than
Lua 5.4 on these kernels, by 1.1 to 3.5 times, with or without the checks:
that gap belongs to the code generator, not to `pin`. Atmos compiles to
C, but the C is a direct translation of a dynamically typed program: values
are 16 bytes with a type tag, variables live in the frame of the task, in
the heap, and each operation tests types. Compiling removes the dispatch
loop of an interpreter, not this work, and the virtual machine of Lua 5.4
does less of it per operation. The first run was 1.3 to 5.4 times slower
than Lua 5.4; reading variables in place in operators, inline comparison and
division of floats, and counted loops with no call (paper-pin#16, in
`9ea77e1`) took mandelbrot from 5.4 to 2.3 times and fannkuch from 4.6 to
3.0. C and LuaJIT only place the figures. Peak memory is the same in the
three builds of Atmos (binary-trees: 50 to 53 MB, against 57 MB on Lua 5.4
and 75 MB on LuaJIT; the others about 2 MB). These programs are small
kernels with little allocation except binary-trees; on a real program,
Doom, the checks cost 2.8% (above, measured before these changes).

`ATMOS_PIN=unchecked` keeps the semantics of `pin` with no checks;
`ATMOS_PIN=off` also keeps `pin` only for tasks and pools (`doc/manual.md`).

## 10. Limitations

**The check is dynamic.** An escape is reported when the program executes
the store that would cause it, on the paths that execute. The compiler warns
for some cases (a pin given away by a block, an unpinned recursive closure),
but most errors are found by running the program.

**Shared structures need a common owner.** A value shared by two parts of a
program must be pinned by a block that encloses both, or kept unpinned and
given away. A value pinned explicitly while something else holds it is an
error (`invalid pin : shared table`). There are no weak references.

**Pins do not collect.** Garbage created while a block is alive waits for
the end of that block. A block that lives long, such as the main loop or a
module, may accumulate it. With reference counting, an unpinned cycle is
never reclaimed: a cyclic structure must be pinned, by its block or by a
pinned value that holds it. The compiler warns for the common case, a
recursive closure through a variable; other cycles leak silently. A tracing
collector beside the counts would remove this limitation.

**`set :out` is unchecked.** A place of an outer block may refer to a value
of an inner one with `set :out`, with no check. The place is reset when the
value ends, so a reader must not keep what it reads beyond that point.

**Ownership of shared values.** The receiver of an event cannot pin it,
since the emit owns it. A global event cannot carry a value of an inner
block, so programs send identifiers instead. A native value replaced in an
owned place is closed when its last holder lets it go: deterministic, but
not a point of the text.

**Costs that scale.** A value that leaves its block is transferred deeply,
at a cost proportional to its size.

**Structure is required.** The mechanism needs a language with structured
tasks and abortion (Section 6).

## 11. Related Work

**Prior work.** Dyn-lex (2023) attached every dynamic value, with no keyword,
to the block of its first assignment, with all checks at run time, and moved
values explicitly with `drop`. Its states FLEET, MUTAB, and IMMUT are the
ancestors of unpinned and pinned values. It rejected a value moved to a
deeper block while it still had references, the hole that the ceiling of
Atmos closes. Câmara (2024) compared dyn-lex with Lua 5.4 on programs of the
Computer Language Benchmarks Game, and found that they translate with no
change of algorithm. Atmos makes the pin explicit, adds the floor and the
ceiling, allows reassignment of owned places, and applies the same rules to
tasks.

**Structured concurrency.** Trio's nurseries, Kotlin's coroutine scopes,
Swift's task groups, and Java's structured task scopes own tasks; Céu and
Esterel add abortion and deterministic finalization. Structured memory
extends their ownership to data.

**Regions.** Tofte and Talpin's region inference, its use in the MLKit,
Cyclone's lexical regions, and the scoped memory of Real-Time Java tie the
lifetime of data to the program's structure, with static checks or with
checks at assignment, as Real-Time Java does.

**Ownership.** Rust, ownership types, Vale, and Lobster give each value a
single owner, checked statically or with a mix of static and dynamic
checks.

**Reference counting.** Swift's ARC, Perceus, and trial deletion for cycles
(Bacon and Rajan) improve counting; Atmos keeps plain counting and leaves
cycles to pins.

**Resource management.** RAII, `defer`, `with`, `using`, and
`try-with-resources` release resources at the end of a scope, with no
guarantee about references that survive it.

**Arenas and escape analysis.** Arenas and zones (Zig, Odin, Apache pools)
free values in bulk; escape analysis allocates on the stack what does not
escape. Both decide per allocation site what pins decide per value.

## 12. Future Work

- Static detection of escapes, for a subset of the checks, and elision of
  the dynamic check where the block of both sides is known, and of the
  tests of type that remain on parameters and on variables that only hold
  numbers (Section 9).
- A formal semantics, and a proof that no use outlives its block.
- A tracing collector beside reference counting, for unpinned cycles only,
  keeping the counts as the semantics of everything else.
- Scoped assignment for module tables and global state, such as
  `with m.x = v {...}`.

## 13. Conclusion

Structured concurrency gave tasks a home; structured memory gives one to
every dynamic value. A single rule, attach a value to a block and store it
only where that block lasts, covers tasks, data, and resources, and its
check costs one comparison per store. The blocks of the program become its
memory regions. The mechanism depends on structure, and in turn completes
it: the blocks that own the tasks also own what the tasks use.
