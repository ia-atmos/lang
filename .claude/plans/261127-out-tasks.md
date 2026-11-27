# `set :out` only for tasks (atmos-pin#35, proposal)

Agreed with the author (2026-11-27, in `atmos-pin`), who approved the
implementation ("Sim") and the removal of `atmos.status`. Implemented on
branch `pin` (c37c7ce and the commits after it); the merge into `main`
waits for his approval. For the ports: `261127-out-migration.md`.

Found while implementing:

- `type(x) == 'xtask'` (3 golden cases) is false now: `x ?? :xtask`.
- The C functions of the standard library (`X.print`, `X.copy`) see a
  dead table as empty, with no error, as before for a finalized one.
- `spawn { ... }` is transparent: `task` inside it is the enclosing task.

## Why

A task has two things that data does not have:

- a dead state: a reference to a task that ended stays valid;
- an event: who refers to it may `await t` or `watching t` and react.

So a place of an outer block that refers to a task of an inner block
needs no reset to `nil`: the task dies with its block, and the reference
says so.

## The two steps of the end of a task

Decided by the author on 2026-11-27, after the first implementation
broke env-sdl `exs/rocks` (a loop that reads the `pub` of tasks that
terminated along the way) and 6 golden cases:

- A task terminates: it is dead (`:xtask.dead`). While it is in scope,
  its `pub` and its final value (`t.ret`, `await t`) remain.
- A task leaves its scope: the block it is attached to ends. Its `pub`
  and final value are gone (`:xtask.dead.gone`), and only a place of
  `set :out` may still refer to it.

By the prefix rule, `t ?? :xtask.dead` holds in both, and
`t ?? :xtask.dead.gone` only in the second. A pool and pinned data have
the second step only (`:tasks.dead`, `:table.dead`).

## The one rule of a dead value

A reference to a value whose block ended stays valid. What ends is its
content, and a use of the content is an error. This holds for a task, a
pool, and pinned data, with no special case:

- on the reference, always valid: `==`, `??`, `type`, printing, storing
  it, passing it, `await t` and `watching t` (which return at once);
- on the content, an error (`invalid access : dead task`, `dead pool`,
  `dead table`): the `pub` and the final value of a task that is out of
  scope, a field, an index, a length or an iteration of finalized data,
  an iteration of or a `spawn` into a dead pool.

## The rules of `set :out`

1. `set :out place = v` accepts only a task or a pool
   (`invalid set :out : expected task or pool`), and a block must own it:
   a `pin`, a pool, or a `spawn` statement
   (`invalid set :out : expected pinned task`). An unpinned task, held
   only by names, is a plain value: it goes to the place with a plain
   `set`, which adopts it.
2. `:out` stays mandatory: a plain `set` of a task of an inner block into
   an outer place is still `invalid escape`.
3. The place is never reset: it keeps the task. When the block of the
   task ends, the task is aborted and out of scope; the reference stays
   valid.
4. The `pub` and the final value last while the task is in scope, as
   before this change. So the `pub` cannot hold what is pinned in the
   body of the task (withdrawn: #35a, a plain `set` of body data into
   `pub`): a task creates what it publishes in the `pub` itself.
   `set :out pub.x = data` no longer exists.
5. A dead pool raises an error on any use of its content (#35b).
6. `invalid access : finalized ...` is now `invalid access : dead ...`,
   and stays for data (#35c): the case left is data read through a `pub`
   and kept in a name, used after the task left its scope.
7. The state of a task is a subtype of its type (#35d): `:xtask.running`,
   `:xtask.suspended`, `:xtask.dead`, `:xtask.dead.gone`. The type rule
   of `??` accepts a dot prefix. `atmos.status` is removed.

## What it removes from the runtime

- The registry of `set :out` (`OUTS`, `a_out_reg_*`, `a_out_ended`, the
  hash of #30), and the finalizer that each `set :out` adds to its block.
- The hole in a vector when a value ends (#22).
- The question of a test of a dead value (#33), by the subtype `.dead`.

## The uses in the ports

66 lines with `set :out` outside tests and comments (read from the
clones on 2026-11-27, no session asked):

| What the place gets | Uses | Where | With the proposal |
|---|---|---|---|
| A task or a pool | 22 | sdl-snkrx: `A.w.objs@(b).task = task` (18); sdl-flare: `hud.ens`, `hud.npcs`; ray-lessmilk: the pools `pub.tw` (2) | Unchanged |
| The `pub` of a task | 8 | sdl-flare: `hud.enemy`, `hz.src` (3), `sp.summoner`, `combat.last_enemy`, `hp.cursor_enemy`, `loot.dropping` | The place gets the task; readers use `t.pub.x` |
| A table of a screen or game that is a task | 30 | ray-lessmilk: `Physics.scr` (14), `Trace.q` (11); env-ray: `game` (4); sdl-flare: `hud.events` | The place gets the task of the screen (`task`); the data goes to its `pub` |
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
if hud.enemy && !(hud.enemy ?? :xtask.dead.gone) { draw(hud.enemy.pub.hp) }
```

```
;; ray-lessmilk, the bodies of a screen
pin phys = Physics_screen()         ;; before
set :out Physics.scr = phys

set pub = [phys = Physics_screen()] ;; after, in the task of the screen
set :out Physics.scr = task         ;; readers: Physics.scr.pub.phys
```

Checked: in sdl-flare a dead enemy is a corpse whose task is still
alive (`e.corpse`, a timer), so the hud keeps reading its `pub`. No port
keeps a registry that would grow with dead tasks: sdl-snkrx removes
`objs@b` with the body, sdl-flare resets `loot.dropping` each frame, the
other places are single fields that are overwritten.

## Costs

- A reader tests before it uses: the place may be empty, and the task
  may be out of scope (`t ?? :xtask.dead.gone`), or it waits with
  `watching t`.
- A place that is never overwritten keeps a dead task in memory (its
  header and final value) while it refers to it.
- 44 of the 66 uses change, most by one level of indirection.

## Next

The merge into `main` (the author's approval, then `atmos-lang`), the
migration of the ports with their sessions, and `paper-pin` for the
section on `set :out`.
