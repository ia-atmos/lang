# pool: proposals ready for the author

One screen each, for `atmos-lang#2`, `#4`, `#33`: the real case in the
ports (clones of 2026-11-20), the options, the recommendation. Prepared
by the session under the rule "Work with no input" (`atmos-lang#95`);
none is decided.

## `atmos-lang#2`: phases inside a tick

**The case.** Two things must happen in a fixed order in every tick,
whatever the order in which the tasks were created: in Pingus, the
pingus move, and then the objects of the world look at them; in
Lessmilk, the bodies move, the game updates, the overlaps are tested.

**What the ports wrote.** One event per phase, emitted in order by the
one task that owns the frame:

```
;; sdl-pingus/game/game.atm:102
emit @(:parent) :tick           ;; the pingus move
emit @(:parent) :tock           ;; exits, traps and belts look at them

;; ray-lessmilk/lib/game.atm:176
emit @:global :pre
physics_pre()
emit @:global :particles
emit @:global :update
emit @:global :overlap
```

An `emit` runs to completion before the next statement (manual,
"Structured Deterministic Concurrency"), so the phases cannot
interleave. Lessmilk has up to seven of them (`:time`, `:state`, `:pre`,
`:update`, `:post`, `:render`), following Phaser.

**Options.**

- `#2a`: nothing in the language; the manual gains a paragraph and an
  example of the convention (a frame task that emits the phases).
- `#2b`: a priority on an await or on a task (`await :tick, 2`), which
  reorders the traversal of one emit. It breaks the rule that the order
  of awaking is the order of the source, the base of the determinism of
  the language, and costs a sort of the awaiting tasks in each emit.

**Recommended: `#2a`.** Both ports reached the same form alone, it reads
well, and it costs one traversal of the tree per phase (measured by no
port as a problem). To do: the paragraph in the manual.

## `atmos-lang#4`: an `emit` that names its receiver, instead of `emit @2`

**The case.** A task emits to an ancestor counted by hand, `emit @2
(e)`: adding a `watching` or a `par` around the emit changes the count
and the event goes silently to the wrong task.

**What the ports wrote.** Today no port has an `emit @N` left. Across
the five ports the targets are:

| Target | Uses |
|---|---|
| `emit @:global` | 47 |
| `emit @t` (a task held in a variable or an argument) | 34 |
| `emit @:task` | 25 |
| `emit @(:parent)` | 15 |
| `emit @(task)`, the running task itself | 3 |
| `emit @N` | 0 |

The receiver arrives as an argument (`task Emitter (t, ...)`), or is the
task itself taken with `task` before a nested block.

**Options.**

- `#4a`: no new syntax; the manual says, in "Emit", that a numeric
  level is fragile and shows the two forms above.
- `#4b`: besides `#4a`, the compiler warns on `emit @N` with `N > 1`
  (`:parent` covers `1`).
- `#4c`: remove the numeric target. It departs from the reference, and
  breaks its tests that use it.

**Recommended: `#4a`.** The ports stopped using the numeric form
without being told to. `#4b` is cheap if he wants the push.

## `atmos-lang#33`: time emitted to a pool or a task

**The case.** SNKRX slows the game down (a slow motion at the end of an
arena, down to 0) while the interface keeps moving in real time. With
time emitted to a pool, the arena would be a pool that receives `0.3 *
dt` and the interface another that receives `dt`, and every `await 1s`
and `:clock` inside each would follow its own time.

**What the port wrote.** One global factor in the environment, and the
real time taken from the event of the tick where it is needed:

```
;; sdl-snxkx/eng/eng.atm:107
slow = func (f) { `EG_SLOW = eg_dbl($f);` }     ;; the clock runs at f times
;; sdl-snxkx/eng/anim.atm:161
task WaitReal (s) {                             ;; `await` in real time
    var timer = 0
    loop e on :tick {
        set timer += tick_dt(e.us)
        until timer > s
    }
}
```

The cost of the workaround: what runs in real time cannot use the
timers of the language (`await 1s`, `watching 2s`), and counts by hand
(`WaitReal`, `UISprings`). It is one port, about ten places.

**Options.**

- `#33a`: not now; the global factor and `:tick` stay. The runtime
  rejects a number emitted to anything but the global target (`invalid
  emit : expected global`).
- `#33b`: `emit @ts (dt)`: time passes only for the timers under the
  target. In the runtime: the nearest timer (`CLK_MIN`) is global today,
  and would become one per subtree, or a walk of the subtree in each
  emit of time; `await` with a duration would have to know which clock
  it belongs to (the nearest ancestor that received time?). It is a
  design of its own, and departs from the reference.
- `#33c`: `toggle` on the task that emits the ticks, which Pingus already
  does to pause (`toggle clock(false)`, `clock` being its task): it
  gives stop and go, not a slower time.

**Recommended: `#33a`.** One port, with a workaround of two small tasks.
`#33b` is worth it if a second port needs two clocks; the question to
settle then is which clock an `await 1s` listens to.
