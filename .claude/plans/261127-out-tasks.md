# `set :out` only for tasks (atmos-pin#35, proposal)

Proposal agreed with the author (2026-11-27, in `atmos-pin`). Nothing
here is implemented, and the implementation waits for his go.

## Why

A task has two things that data does not have:

- a dead state: a reference to a task that ended stays valid;
- an event: who refers to it may `await t` or `watching t` and react.

So a place of an outer block that refers to a task of an inner block
needs no reset to `nil`: the task dies with its block, and the reference
says so.

## The one rule of a dead value

A reference to a value that ended stays valid. What ends is its content,
and a use of the content is an error. This holds for a task, a pool, and
pinned data, with no special case:

- on the reference, always valid: `==`, `??`, `type`, printing, storing
  it, passing it, `await t` and `watching t` (which return at once, with
  the final value of the task, which left it when it returned);
- on the content, an error (`invalid access : dead task`, `dead pool`,
  `dead table`): the `pub` of a dead task (read or write), a field, an
  index, a length or an iteration of finalized data, an iteration of or
  a `spawn` into a dead pool.

Every value that ended has the subtype `.dead` of its type:
`:xtask.dead`, `:tasks.dead`, `:table.dead` (and so for a function and
a native). So one test serves all: `x ?? :table.dead`. This is what a
finalized table already does today (`==` and `type` work, an index
raises); the rule extends it to the task and to the pool. It closes
atmos-pin#33 (the test of a dead value).

## The rules of `set :out`

1. `set :out place = v` accepts only a task or a pool. Any other value is
   an error (`invalid set :out : expected task or pool`). Decided.
2. `:out` stays mandatory: a plain `set` of a task of an inner block into
   an outer place is still `invalid escape`. Decided.
3. The place is never reset: it keeps the task. When the block of the
   task ends, the task is aborted and is dead; the reference stays valid.
   Decided ("nem seria necessário o set nil").
4. The `pub` ends with its task: the data pinned with the task is
   finalized, the `pub` is released, and a use of the `pub` of a dead
   task is an error, by the rule above. The program never sees a `nil`
   there. So every task may leave with `:out`: its `pub` never outlives
   it. Decided (#35c, #35e; it was "3.2, the pub is nil").
5. A plain `set pub.x = v` accepts data pinned in the body block of the
   task itself, since both end together. The capability is the one of
   today's `set :out pub.x = v`, which rule 1 removes; only the writing
   changes. Data of an inner block of the task stays an
   `invalid escape`. Decided (#35a).
6. A dead pool raises an error on any use of its content (iteration,
   `spawn`), as a finalized pool does today. Decided (#35b).
7. `invalid access : finalized ...` stays, as `invalid access : dead
   ...`, for data too: the case left is data read through a `pub` and
   kept in a name (`val es = t.pub.enemies`, used after `t` ended).
   Decided (#35c).
8. The state of a task is a subtype of its type: `:xtask.running`,
   `:xtask.suspended`, `:xtask.dead`. Decided (#35d). Assessed by
   `atmos-lang` (2026-11-27): feasible and cheap, no new field. One rule
   changes: the type rule of `??` is an exact comparison today, and
   becomes a dot prefix, as for tags, so that `t ?? :xtask` stays true
   (`match` and `catch` follow). Error messages and `tostring` keep the
   base name. Muted is a flag, not a state. It recommends removing
   `atmos.status` (one use in the ports, sdl-tux, and 9 cases): to
   confirm with the author.

## What it removes from the runtime

- The registry of `set :out` (`OUTS`, `a_out_reg_*`, `a_out_ended`, the
  hash of #30), and the finalizer that each `set :out` adds to its block.
- The hole in a vector when a value ends (#22).
- The `pub` of a dead task kept until its pool ends (#27), and the
  retire of the `pub` of a freed task into its pool (#34).
- The question of a test of a dead value (#33), by the subtype `.dead`.

## Prototype of the `pub` that ends with its task

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
if hud.enemy && !(hud.enemy ?? :xtask.dead) { draw(hud.enemy.pub.hp) }
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

- A reader tests before it uses: the place may be empty, and the task
  may be dead (`t ?? :xtask.dead`), or it waits with `watching t`.
- A place that is never overwritten keeps a dead task in memory (its
  header and final value) while it refers to it.
- 44 of the 66 uses change, most by one level of indirection.

## Next

With the author's go: implement on `pin` (with `atmos-lang` for the
subtype and the prefix rule of `??`), migrate the ports with their
sessions, and ask `paper-pin` to rewrite the section on `set :out`.
Open with the author: whether `atmos.status` is removed.
