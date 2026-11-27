# `set :out` only for tasks (atmos-pin#35, proposal)

Proposal under discussion with the author (2026-11-27, in `atmos-pin`).
Nothing here is implemented. Decided by him so far: items 1, 2 and 3
below, with 3.2 preferred. Everything marked "to decide" is open.

## Why

A task has two things that data does not have:

- a dead state: a reference to a task that ended stays valid;
- an event: who refers to it may `await t` or `watching t` and react.

So a place of an outer block that refers to a task of an inner block
needs no reset to `nil` and no error on use: the task dies with its
block, and the reference says so.

## The rules

1. `set :out place = v` accepts only a task or a pool. Any other value is
   an error (`invalid set :out : expected task or pool`). Decided.
2. `:out` stays mandatory: a plain `set` of a task of an inner block into
   an outer place is still `invalid escape`. Decided.
3. The place is never reset: it keeps the task. When the block of the
   task ends, the task is aborted and is dead; the reference stays valid.
   Decided ("nem seria necessário o set nil").
4. The `pub` of a task that ended is `nil` (3.2). The data pinned with
   the task is finalized with it, as today, and the `pub` no longer
   refers to it. So every task may leave with `:out`: its `pub` never
   outlives it. The final value of the task stays readable: it already
   obeys the rules, since only a value that may leave is returned.
   Preferred by the author; prototype below.
5. To decide: a plain `set pub.x = v` accepts data pinned in the body
   block of the task itself, since both end together. Today this needs
   `set :out pub.x = v`, which rule 1 removes. Data of an inner block of
   the task stays an `invalid escape`.
6. To decide: a dead pool. Iterating it gives nothing (no error). A
   `spawn` into it is an error, since the new task would have no block.
7. To decide: `invalid access : finalized ...`. A dead task raises
   nothing: `t.pub` is `nil`, and its state says it is dead. The error
   can then only come from data read through a `pub` and kept in a name
   (`val es = t.pub.enemies`, used after `t` ended). Options: (a) keep
   the error for that case, recommended, since an emptied table that
   reads as empty hides the bug; (b) remove it: the table reads as
   empty, a pool as empty, a closed native as closed. Memory is safe
   either way (the counts keep the husk).
8. To decide: how to test the state of a task. Today
   `atmos.status(t) == :dead`. The author's idea: a subtype of the type,
   `t ?? :xtask.dead` (also `.running`, `.suspended`). This replaces
   atmos-pin#33 (the test of a dead value). Assessed by `atmos-lang`
   (2026-11-27): feasible and cheap, no new field. One rule changes:
   the type rule of `??` is an exact comparison today, and becomes a dot
   prefix, as for tags, so that `t ?? :xtask` stays true (`match` and
   `catch` follow). Error messages and `tostring` keep the base name.
   The states are three (running, suspended, dead); muted is a flag,
   not a state. It recommends removing `atmos.status` (one use in the
   ports, sdl-tux, and 9 cases). It would give no subtype to pools,
   since a finalized pool is an error today; with item 6 a dead pool is
   valid, so `:tasks.dead` would be needed.

## What it removes from the runtime

- The registry of `set :out` (`OUTS`, `a_out_reg_*`, `a_out_ended`, the
  hash of #30), and the finalizer that each `set :out` adds to its block.
- The hole in a vector when a value ends (#22).
- The `pub` of a dead task kept until its pool ends (#27), and the
  retire of the `pub` of a freed task into its pool (#34).
- The question of a test of a dead value (#33), by item 8.
- If 7b: the mark of finalized data (`A_FIN`) and its test at each use.

## Prototype of 3.2

Local, not committed (2026-11-27, main 02f2ca7): the `pub` is cleared
where a task ends (4 lines in `rt/task.c`). 560 of 563 cases unchanged.
The 3 that change read the `pub` after the end on purpose: `out-03` and
`out-pub-02` (written for the behavior of #27) and `task-03`.

## The uses in the ports

66 lines with `set :out` outside tests and comments (read from the
clones on 2026-11-27, no session asked):

| What the place gets | Uses | Where | With the proposal |
|---|---|---|---|
| A task or a pool | 22 | sdl-snkrx: `A.w.objs@(b).task = task` (18); sdl-flare: `hud.ens`, `hud.npcs`; ray-lessmilk: the pools `pub.tw` (2) | Unchanged |
| The `pub` of a task | 8 | sdl-flare: `hud.enemy`, `hz.src` (3), `sp.summoner`, `combat.last_enemy`, `hp.cursor_enemy`, `loot.dropping` | The place gets the task; readers use `t.pub.x` |
| A table of a screen or game that is a task | 30 | ray-lessmilk: `Physics.scr` (14), `Trace.q` (11); env-ray: `game` (4); sdl-flare: `hud.events` | The place gets the task of the screen (`me`); the data goes to its `pub` |
| Loose data | 6 | sdl-snkrx: springs in `owner.pub.springs@id` (3); ray-lessmilk: the tables `r.tw`, `e.tw`, `s.tw` | The place gets the task that owns the datum, or the datum moves into a task |

Per port: sdl-snkrx 21 (18 unchanged), ray-lessmilk 30 (2 unchanged),
sdl-flare 11 (2 unchanged), env-ray 4. The types of `e.tw`, `s.tw`
(ray-lessmilk) and whether `game` (env-ray) is set inside a task are to
confirm with the ports.

Examples:

```
;; sdl-flare, the enemy shown by the hud
set :out hud.enemy = e              ;; before: e is the pub of the enemy
if hud.enemy { draw(hud.enemy.hp) }

set :out hud.enemy = t              ;; after: t is the task of the enemy
if hud.enemy && hud.enemy.pub { draw(hud.enemy.pub.hp) }
```

```
;; ray-lessmilk, the bodies of a screen
pin phys = Physics_screen()         ;; before
set :out Physics.scr = phys

set pub = [phys = Physics_screen()] ;; after, in the task of the screen
set :out Physics.scr = me           ;; readers: Physics.scr.pub.phys
```

Checked: in sdl-flare a dead enemy is a corpse whose task is still
alive (`e.corpse`, a timer), so the hud keeps reading its `pub`. No port
keeps a registry that would grow with dead tasks: sdl-snkrx removes
`objs@b` with the body, sdl-flare resets `loot.dropping` each frame, the
other places are single fields that are overwritten.

## Costs

- A reader tests two levels: the place may be empty, and the `pub` of a
  dead task is `nil`.
- A place that is never overwritten keeps a dead task in memory (its
  header and final value) while it refers to it.
- 44 of the 66 uses change, most by one level of indirection.

## Next

After the author decides 5, 6, 7 and 8: implement on `pin`, migrate the
ports with their sessions, and ask `paper-pin` to rewrite the section on
`set :out`.
