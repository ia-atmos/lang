# Guide

A tour of Atmos in short steps.
Each step has a complete program, which runs alone, and the output it
prints.
The programs are in [`tst/guide/`](../tst/guide/) and are part of the
tests of the compiler (`make test-guide`).
The [manual](manual.md) has the details of every construct.

1. [Tasks & Events](#1-tasks--events)
2. [External Environments](#2-external-environments)
3. [Lexical Structure](#3-lexical-structure)
    - [Lexical Scheduling](#31-lexical-scheduling)
    - [Lexical Hierarchy](#32-lexical-hierarchy)
4. [Compound Statements](#4-compound-statements)
5. [More about Tasks](#5-more-about-tasks)
    - [Public Data](#51-public-data)
    - [Task Pools](#52-task-pools)
    - [Muting](#53-muting)
6. [Errors](#6-errors)
    - [Stack Traces](#61-stack-traces)
7. [Events as Data](#7-events-as-data)
    - [Loops over Events](#71-loops-over-events)
    - [Functions over Collections](#72-functions-over-collections)
8. [Where to Go Next](#8-where-to-go-next)

# 1. Tasks & Events

Tasks are the basic units of execution in Atmos.

A `task` declares a prototype, and the `spawn` primitive starts a task from
it.
The `await` primitive suspends a task until a matching event occurs, and the
`emit` primitive broadcasts an event, awaking all tasks awaiting it:

<!-- tst/guide/01-tasks.atm -->

```
task T (i) {
    await :X
    print("task " ++ i ++ " awakes on X")
}
spawn T(1)      ;; starts a task, which awaits `:X`
spawn T(2)      ;; starts another one
emit :X         ;; awakes both
```

Output:

```
task 1 awakes on X
task 2 awakes on X
```

In the example, `:X` is a [tag](manual.md#hierarchical-tags) that identifies
the event.
Both tasks start and suspend at their `await`, and the `emit` awakes them.

Therefore, based on the `await` and `emit` primitives, Atmos supports
*reactive scheduling* for tasks.

# 2. External Environments

An environment is the external component that bridges input events from the
real world into an Atmos application.
These events can be timers, key presses, network packets, or other kinds of
inputs, depending on the environment.

The main body of the program is itself a task, which can await and react to
the environment.
After it starts, the environment runs a continuous loop that waits for
external input and forwards it to the program as events.

The default environment only deals with time.
The next example relies on it to count 3 seconds:

<!-- tst/guide/02-clock.atm -->

```
print "Counts 3 seconds:"
loop in 3 {
    await 1s
    print "1 second..."
}
print "3 seconds elapsed."
```

Output:

```
Counts 3 seconds:
1 second...
1 second...
1 second...
3 seconds elapsed.
```

In the example, `1s` is a [clock duration](manual.md#literals), which
allows Atmos to await time.
The program terminates when its main body terminates.

Other [environments](manual.md#environments) connect a program to a
library, such as SDL, raylib, or a web page.
They are separate projects: a program names its environment when it is
compiled, and then awaits the events that the environment emits.
The next fragment, for the SDL environment, opens a window and reacts to
key presses (it is not one of the tested programs of this guide, since it
needs that environment):

```
sdl.window [title="keys", width=200, height=200]

loop e on :key.dn {
    print("key pressed", e.key)
}
```

# 3. Lexical Structure

In Atmos, the lexical organization of tasks determines their lifetimes and also
how they are scheduled, which helps to reason about programs more statically,
based on the source code.

## 3.1. Lexical Scheduling

The reactive scheduler of Atmos is deterministic and cooperative:

1. `deterministic`:
    When multiple tasks spawn or awake concurrently, they activate in the order
    they appear in the source code.
2. `cooperative`:
    When a task spawns or awakes, it takes full control of the application and
    executes until it awaits or terminates.

Consider the following program, which spawns two anonymous tasks that await
the same event `:X`:

<!-- tst/guide/03-scheduling.atm -->

```
print "1"
spawn {         ;; task #1
    print "a1"
    await :X
    print "a2"
}
print "2"
spawn {         ;; task #2
    print "b1"
    await :X
    print "b2"
}
print "3"
emit :X
print "4"
```

Output:

```
1
a1
2
b1
3
a2
b2
4
```

In the example, the scheduling behaves as follows:

- The main body prints `1` and spawns the first task.
- The first task takes control, prints `a1`, and suspends, returning the
  control back to the main body.
- The main body prints `2` and spawns the second task.
- The second task starts, prints `b1`, and suspends.
- The main body prints `3`, and broadcasts `:X`.
- The first task awakes, prints `a2`, and terminates.
- The second task awakes, prints `b2`, and terminates.
- The main body prints `4`.

### 3.1.1. Synchronous Timers

Determinism and predictability also extend to timers because they all advance
together, with the same clock ticks emitted by the environment.
The scheduler guarantees that timers in concurrent tasks awake in perfect
synchrony, regardless of the granularity of the ticks:

<!-- tst/guide/04-timers.atm -->

```
spawn {
    par :any {
        await 30ms
        print "30ms"        ;; never prints
    } with {
        await 10ms
        print "10ms"        ;; prints
        await 15ms
        print "15ms"        ;; prints, aborts the other branch
    }
}
emit @:global 30ms          ;; 30ms elapse at once
```

Output:

```
10ms
15ms
```

In the example, instead of waiting for the environment, the program itself
emits the passage of `30ms`, in a single tick.
Although the `await` in the first branch appears first in the source and
matches the tick, both `await`s in the second branch expire first
(`10+15<30`).
The program prints `10ms` and `15ms`, but never `30ms`, since the second
branch terminates and aborts the whole `par :any` composition.

## 3.2. Lexical Hierarchy

Tasks form a hierarchy based on the source position in which they are spawned.
Therefore, the lexical structure of the program determines the lifetime of
tasks.

In the next example, the outer task terminates and aborts the inner task before
it has the chance to awake:

<!-- tst/guide/05-hierarchy.atm -->

```
spawn {
    spawn {
        await :Y    ;; never awakes: aborted when :X occurs
        print "never prints"
    }
    await :X        ;; awakes, terminates, and aborts the nested task
    print "outer task terminates"
}
emit :X
emit :Y
```

Output:

```
outer task terminates
```

### 3.2.1. Pinned Tasks

A task lives at most as long as the block that spawns it.
A `spawn` that is a statement, as in the examples so far, attaches the task to
the enclosing block.
To hold a task in a name, the name is declared with
[`pin`](manual.md#pin), which also attaches the task to the block of the
declaration:

<!-- tst/guide/06-pin.atm -->

```
task T (name) {
    defer {
        print(name ++ " is aborted")
    }
    await false     ;; never awakes
}
do {
    pin t = spawn T("t")
    print "block terminates"
}                   ;; aborts `t`
print "after the block"
```

Output:

```
block terminates
t is aborted
after the block
```

In the example, the task `t` never awakes by itself.
When the `do` block terminates, it aborts `t`, which executes its `defer`
clause, detailed next.

### 3.2.2. Deferred Statements

A task can register deferred statements to execute when it terminates or
aborts:

<!-- tst/guide/07-defer.atm -->

```
spawn {
    spawn {
        defer {
            print "nested task aborted"
        }
        await false     ;; never awakes
    }
    await :X
}                       ;; on :X, aborts the nested task
emit :X
```

Output:

```
nested task aborted
```

The nested task never awakes, but executes its `defer` clause when its
enclosing task terminates.

Tasks and deferred statements can also be attached to explicit blocks:

<!-- tst/guide/08-defer-block.atm -->

```
do {
    spawn {
        defer {
            print "task aborted"
        }
        await false
    }
    defer {
        print "first defer"
    }
    defer {
        print "second defer"
    }
    print "block terminates"
}
```

Output:

```
block terminates
second defer
first defer
task aborted
```

We attach a `spawn` and two `defer`s to an explicit block.
When the block terminates, it executes its deferred statements and aborts the
task, in the reverse order in which they appear in the source code.
The aborted task has a pending `defer`, which then also executes.

# 4. Compound Statements

Atmos provides many compound statements built on top of tasks.

A `loop on` awaits an event at the beginning of each iteration.
A `watching` awaits its body to terminate, or aborts it when its first
argument occurs:

<!-- tst/guide/09-loop-on.atm -->

```
watching 350ms {
    loop on 100ms {
        print "100ms elapse"    ;; prints every 100ms
    }
}
print "done"
```

Output:

```
100ms elapse
100ms elapse
100ms elapse
done
```

The loop never terminates by itself: the `watching` aborts it after `350ms`,
when it has printed three times.

A `watching` also terminates when its body does:

<!-- tst/guide/10-watching.atm -->

```
spawn {
    watching 1s {
        await :X
        print "X happens before 1s"
    }
    print "watching terminates"
}
emit :X
```

Output:

```
X happens before 1s
watching terminates
```

A `par` spawns multiple bodies concurrently and rejoins depending on its tag
suffix:

- a `par :all` rejoins after all bodies terminate,
- a `par :any` rejoins after any body terminates, and
- a `par` without any suffix never rejoins.

The next example awaits three events, in whatever order they occur:

<!-- tst/guide/11-par-all.atm -->

```
spawn {
    par :all {
        await :X
        print "X"
    } with {
        await :Y
        print "Y"
    } with {
        await :Z
        print "Z"
    }
    print "X, Y, and Z occurred"
}
emit :Z
emit :X
emit :Y
```

Output:

```
Z
X
Y
X, Y, and Z occurred
```

# 5. More about Tasks

## 5.1. Public Data

Each task has a special variable `pub` to expose public data to the outside:

<!-- tst/guide/12-pub.atm -->

```
task T () {
    set pub = 10    ;; exposes `pub` to the outside
    await false
}
pin t = spawn T()
print(t.pub)        ;; reads `pub` of `t`
```

Output:

```
10
```

## 5.2. Task Pools

A task pool, created with the `tasks` primitive, allows that multiple tasks
share a parent container in the hierarchy.
When the pool goes out of scope, all attached tasks are aborted.
When a task terminates, it leaves the pool.

The next example attaches four tasks to a pool, and traverses the pool after
some of them terminate:

<!-- tst/guide/13-pools.atm -->

```
task T (id, ms) {
    set pub = id
    print(:start, id, ms)
    await (ms * 1ms)
    print(:stop, id, ms)
}

do {
    pin ts = tasks()
    loop i in 4 {
        spawn @ts T(i, (i+1) * 100)
    }
    await 250ms
    loop _, t in ts {
        print(:pending, t.pub)
    }
}                   ;; aborts the tasks still in the pool
print "done"
```

Output:

```
start	0	100
start	1	200
start	2	300
start	3	400
stop	0	100
stop	1	200
pending	2
pending	3
done
```

In the example, we first create a pool `ts`.
Then we use `spawn @ts` to spawn and attach 4 tasks into the pool.
Each task sleeps for a different time before terminating.
After `250ms`, two of them have terminated.
The loop over `ts` traverses the tasks that are still attached, and prints
their ids.
Then the block of `ts` terminates, aborting the tasks that did not complete.

## 5.3. Muting

A task can be muted (and unmuted) to remain alive but unresponsive (and back
to responsive) to upcoming events:

<!-- tst/guide/14-mute.atm -->

```
task T () {
    await :X
    print "awakes from X"
}
pin t = spawn T()
mute t(true)
emit :X         ;; ignored
print "muted"
mute t(false)
emit :X         ;; awakes
```

Output:

```
muted
awakes from X
```

Just after we spawn the task, we mute it, making it unresponsive to events.
Therefore, the first `emit :X` is ignored by the task.
Then, we unmute the task, and the second `emit` awakes it.

Atmos also provides a `mute on` statement, which awaits its body to terminate,
while also observing a tagged boolean event: when receiving `true`, the body is
muted; when receiving `false`, the body gets the events back:

<!-- tst/guide/15-mute-on.atm -->

```
spawn {
    mute on :Pause {
        loop on 100ms {
            print "100ms elapse"
        }
    }
}
await 250ms
print "pause"
emit(:Pause [true])     ;; mutes the block above
await 300ms             ;; nothing prints
print "resume"
emit(:Pause [false])    ;; gives it the events back
await 200ms
```

Output:

```
100ms elapse
100ms elapse
pause
resume
100ms elapse
100ms elapse
```

While muted, the body gets no clock: its timers stop, and go on from where
they were when it is unmuted.

# 6. Errors

Atmos provides `throw` and `catch` primitives to handle errors, which take in
consideration the task hierarchy, i.e., a parent task catches errors from child
tasks:

<!-- tst/guide/16-errors.atm -->

```
task T () {
    spawn {
        await :X
        throw :Y
    }
    await false
}

spawn {
    val ok, err = catch :Y {
        spawn T()
        await false
    }
    print(ok, err)
}

emit :X
```

Output:

```
false	Y
```

In the example, we spawn a parent task that catches errors of type `:Y`.
Then we spawn a named task `T`, which spawns an anonymous task, which awaits
`:X` to finally throw `:Y`.
Outside the task hierarchy, we `emit :X`, which only awakes the nested task.
Nevertheless, the error propagates up in the task hierarchy until it is caught
by the top-level task, which gets `false` and the error `:Y`.

## 6.1. Stack Traces

An error that no `catch` handles terminates the program with a trace.
The trace starts at the line that raised the error, and goes through the calls
and the awaits that the error crossed, from the innermost to the outermost:

<!-- tst/guide/17-traces.atm -->

```
func check (v) {
    if v < 0 {
        throw :negative
    }
    v
}
task T () {
    await 10ms
    check(-1)
}
val v = await T()
print(v)
```

Output:

```
==> ERROR:
 |  17-traces.atm:11 (await)
 |  17-traces.atm:9 (call)
 v  17-traces.atm:3 (throw)
==> negative
```

In the example, the main body awaits the task `T`, which calls `check` after
`10ms`, which throws the error.
The trace shows the `throw` in line 3, the call in line 9, and the `await` in
line 11, which is where the error leaves the task and reaches the main body.

# 7. Events as Data

A sequence of events over time is also a sequence of values.
Atmos processes it with the same constructs it uses for control: a loop that
awaits each occurrence, and plain functions over the values.

## 7.1. Loops over Events

A `loop on` binds each occurrence of an event to a variable.
The next example filters the occurrences of `:V`, and prints only the odd
values that they carry:

<!-- tst/guide/18-loop-on-events.atm -->

```
spawn {
    loop e on :V {
        if (e.v % 2) == 1 {     ;; only the odd values
            print(e.v)
        }
    }
}
loop i in 6 {
    emit(:V [v=i])
}
```

Output:

```
1
3
5
```

Each `emit` carries a table tagged as `:V`, which the loop receives in `e`.

A task is also a unit of a sequence: awaiting a task prototype spawns it and
suspends until it terminates.
The next example treats each complete execution of `XY`, an `:X` followed by a
`:Y`, as one item to count:

<!-- tst/guide/19-task-sequence.atm -->

```
task XY () {
    await :X
    await :Y
}
spawn {
    loop i in 2 {
        await XY()      ;; a complete X, then Y
        print(i + 1)
    }
    print "done"
}
emit :X
emit :X
emit :Y     ;; 1
emit :X
emit :Y     ;; 2, done
emit :Y
```

Output:

```
1
2
done
```

Since each iteration awaits a complete `XY`, the second `:X` in a row and the
last `:Y` alone count for nothing.

## 7.2. Functions over Collections

The library [`iter`](manual.md#iter) has functions over anything that a
loop can traverse, such as `filter`, `map`, `find`, `any`, and `sum`.
The syntax `\{ ... }` creates an anonymous function with a single parameter
`it`:

<!-- tst/guide/20-iter.atm -->

```
val xs = [3, 8, 5, 12, 7]
val odd = iter.filter(xs, \{ (it % 2) == 1 })
X.print(odd)
X.print(iter.map(odd, \{ it * 10 }))
print(iter.sum(xs), iter.any(xs, \{ it > 10 }))
```

Output:

```
[3, 5, 7]
[30, 50, 70]
35	true
```

A pool of tasks is also a collection, of the tasks attached to it:

<!-- tst/guide/21-iter-pool.atm -->

```
task T (id) {
    set pub = id
    await :Stop
}
pin ts = tasks()
loop i in 3 {
    spawn @ts T(i * 10)
}
val t = iter.find(ts, \{ it.pub == 10 })
print(t.pub, #iter.filter(ts, \{ it.pub > 0 }))
```

Output:

```
10	2
```

# 8. Where to Go Next

The [manual](manual.md) describes the whole language.
Some constructs that this guide does not cover:

- [`pin`](manual.md#pin) and the memory model, for tables and other values
  besides tasks;
- [`context`](manual.md#context), which sets up a state around every
  resumption of a block, and [`stage` and `solo`](manual.md#solo), which
  restrict who receives an event;
- [tables](manual.md#table), [tags](manual.md#hierarchical-tags), and the
  [standard libraries](manual.md#standard-libraries);
- the [integration with C](manual.md#integration-with-c) and how an
  [environment](manual.md#environments) is written.
