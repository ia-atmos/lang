# context: around what enter and leave run

Session `atmos-context`, issue 21 of `atmos-lang/atmos`. Material:
`261118-context.md`. This document covers the first decision: when the
`enter` and `leave` of a context run. The syntax below is the issue's,
used here only as a placeholder (it is a later decision).

```
context {
    => { <enter> }
    <= { <leave> }
    <block>
}
```

## 1. What the references have

| Reference | State |
|---|---|
| `lua-atmos/atmos` | No `context`: neither `main` (2026-10-20) nor any branch has it. |
| `atmos-lang/atmos` | `.claude/plans/260628-context.md`: a plan only. Syntax `context { :init {} :enter {} :body {} :leave {} }`, desugared to `context(enter, leave, block)`, which it lists as a prerequisite to add to `lua-atmos` "around the `emit` dns loop", i.e. once per emit. Not implemented. It mentions an "implicit func-body form" from a comment of the issue, which it rejects. |
| Issue 21 | Three comments, not readable from here. |

## 2. The cases, read again

| Port | Read here? | Is there an `await` between set and restore? | What `context` gives |
|---|---|---|---|
| SNKRX (`Background`, each star of `Stars`) | No: access to `ia-atmos/sdl-snxkx` was refused to this session; shape from `261118-context.md` | No: set, draw, restore in one reaction | Written once around the pool instead of in each star; one switch per pool per frame instead of one per star |
| env-ray (`begin_texture_mode`, `begin_mode2d`) | No: access to `ia-atmos/env-ray` was refused; shape from `261118-context.md` | No: always inside one task, one reaction | Little: the pair is already lexical. Gains only when the drawing for one target is spread over several tasks |
| env-pico, `val old = pico.set.layer(l) ; ... ; pico.set.layer(old)` | Yes | No: functions (`fill` in `tst/life.atm`) and tests, straight-line code | Nothing reactive: `do { val old = ... ; defer { ... } }` already does it |
| env-pico examples (`rocks`, `storm`) | Yes | No | Each `loop on :draw` sets the pencil before drawing and never restores it (set-before-use). A context would set it once for a group (`rocks/main.atm`: two loops that both set `color="white"`) |

No real case awaits between setting and restoring the state: the ports
keep the pair inside one reaction by convention. So `context` is not
fixing a bug the ports have today. Its value is (a) writing the state
once for a subtree of tasks (a pool, a screen), with the tasks inside
not knowing it, and (b) staying correct by construction when a task
inside awaits, emits outward, or is aborted while the state is set.

## 3. The two semantics

**A. Around each emit (the `atmos-lang` plan).** `enter` when the
traversal of an emit enters the subtree of the block, `leave` when it
leaves it.

**B. Around each resume.** The state is set exactly while code that
lives in the subtree of the block runs. The runtime keeps the context
currently applied; before it resumes a trail, it switches to the
trail's context (`leave` of the contexts it exits, innermost first,
then `enter` of those it enters, outermost first), and before it
returns to running code (the end of an `emit`, a `spawn`, an `abort`,
a `toggle`, or of the top-level emit of the environment) it switches
back to the context of that code. The switch is lazy: two trails of the
same context resumed in a row (the stars of a pool) pay one `enter` and
one `leave`, and a subtree in which nothing awakes pays nothing.

Where a task lives decides its context, not where it was spawned from:
a pool declared inside a context runs its tasks inside it; a task that
code inside a context spawns into a pool declared outside does not.
Functions called from inside run inside (draw helpers).

## 4. Programs that tell them apart

Each one prints the output of B; the line of A follows when it differs.

```
;; 1. events that awake nothing inside
spawn {
    context {
        => { print "+A" }
        <= { print "-A" }
        loop on :draw { print "a" }
    }
}
spawn {
    loop on :draw { print "b" }
}
emit :draw      ;; --> +A, a, -A, b
emit :key       ;; (nothing)        A: +A, -A
```

```
;; 2. a task inside emits outward
spawn {
    loop on :hit { print "hit" }
}
spawn {
    context {
        => { print "+A" }
        <= { print "-A" }
        loop on :draw {
            print "a1"
            emit @:global :hit
            print "a2"
        }
    }
}
emit :draw      ;; --> +A, a1, -A, hit, +A, a2, -A
                ;; A: +A, a1, hit, a2, -A    ("hit" runs with A set)
```

```
;; 3. a pool inside: one switch for the whole pool
task Star (i) {
    loop on :draw { print i }
}
spawn {
    context {
        => { print "+A" }
        <= { print "-A" }
        pin ts = tasks()
        loop i in 3 {
            spawn @ts Star(i)
        }
        await false
    }
}
emit :draw      ;; --> +A, 0, 1, 2, -A
```

```
;; 4. the first reaction and the defers
spawn {
    watching :stop {
        context {
            => { print "+A" }
            <= { print "-A" }
            defer { print "d" }
            print "start"
            await false
        }
    }
}
print "main"
emit :stop
;; --> +A, start, -A, main, +A, d, -A
;; A (hook on the emit loop only): start, main, d
```

## 5. Cost in the runtime

- A context is a transparent task, as `toggle on` is, that keeps
  `enter` and `leave`.
- Each task keeps a pointer to its innermost enclosing context, copied
  from its parent (or from the owner of its pool) when it is spawned:
  one store per spawn.
- `trail_resume` compares that pointer with the context currently
  applied: one comparison per resume, equal (no work) in every program
  without contexts. When they differ, it walks up both chains (depth
  1 or 2 in practice) and calls the `leave`s and `enter`s.
- The runtime operations that resume other trails (`emit`, `spawn`,
  `abort`, `toggle`, and the top-level emit) switch back to the context
  of the code that called them before returning: one comparison each.
- The predicates of `await` (`until`, `while`), which run "in the
  context of the awaiting trail", switch as well.
- `enter` and `leave` cannot await (they are functions), and must not
  emit, spawn, or abort: an error at runtime while switching.

A needs a hook at the same places to be correct (the first reaction of
the block is not under an emit; nor are the defers when the block is
aborted from outside; nor is code that an outward emit awakes), so it
is not cheaper: it is B without the cases that B gets right.

## 6. Recommendation

B, lazy. It is the only one in which "the state is set while the code
inside runs" holds for every program; it costs one pointer comparison
per resume; and the lazy switch gives the SNKRX stars one `enter` per
frame for the whole pool.

## 7. Implemented (by the session, to be confirmed)

On the branch `context` (2026-11-20), with B lazy and the syntax
`context (enter, leave) {...}`. In `rt/task.c`:

- `ATrailX` keeps `enter`, `leave` and the value `enter` returned.
- A stack of the contexts applied; `ctx_switch(t, i)` walks up from trail
  `i` of task `t` (the tree of trails, then the trail where the task or
  its pool lives) and leaves and enters the difference.
- `trail_resume` switches to the trail before resuming it, and back to
  the resumer after, except when the traversal of an emit resumed it
  (`LAZY`); `emit_top` goes back to the emitter at the end of the
  traversal. Predicates of `await` switch too.
- `trail_off` of a context leaves it if applied.
- With no context alive and none applied, every hook is one test.

## 8. Decided

- 2026-11-22, the author: B, lazy (section 3). The implementation of
  section 7 stands.
- 2026-11-22, the author: no filter of events (every resume inside is
  bracketed); a filter stays as a TODO (`TODO-context.md` #19).
- 2026-11-22, the author: `leave` receives what `enter` returned.
- 2026-11-22, the author: syntax `context (enter, leave) {...}` or
  `context X {...}` with X giving `[enter, leave]`.
