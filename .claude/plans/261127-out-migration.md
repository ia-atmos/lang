# Migration: `set :out` only for tasks (atmos-pin#35)

For the sessions of the ports. The change is on branch `pin` of
`ia-atmos/lang` and reaches `main` only after the author approves the
merge. The design is in `261127-out-tasks.md`.

## What changed

1. `set :out place = v` takes only a task or a pool that a block owns (a
   `pin`, a pool, a `spawn` statement). Anything else is
   `invalid set :out : expected task or pool : got table`, or
   `invalid set :out : expected pinned task` for an unpinned one, which
   goes to the place with a plain `set`.
2. The place is never reset to `nil`. When the block of the task ends,
   the task is aborted and out of scope, and the place still refers to
   it.
3. A task ends in two steps. It terminates: `t ?? :xtask.dead`, and its
   `pub` and final value (`t.ret`, `await t`) remain while it is in
   scope, as before. It leaves its scope (the block it is attached to
   ends): `t ?? :xtask.dead.gone`, and a use of its `pub` or final value
   is `invalid access : dead task pinned at f:N`.
4. A dead pool (`ts ?? :tasks.dead`) raises on an iteration and on a
   `spawn`; an iteration gave nothing before. Dead data
   (`x ?? :table.dead`) raises as before, and the message says `dead`
   where it said `finalized`.
5. `type(t)` of a spawned task has its state: `xtask.running`,
   `xtask.suspended`, `xtask.dead`, `xtask.dead.gone`. The type rule of
   `??` accepts a prefix, so `t ?? :xtask` is true in any state.
6. `atmos.status` is removed.

## How to migrate

| Before | After |
|---|---|
| `set :out g.x = t` with `t` a task or a pool | unchanged; `g.x` no longer becomes `nil`: test `g.x ?? :xtask.dead.gone` (`:tasks.dead`), or `watching g.x {...}` |
| `set :out g.x = t.pub` (the pub of a task) | `set :out g.x = t`, and readers use `g.x.pub.f` |
| `set :out g.x = tab`, a table of a screen or game that is a task | keep the table in the `pub` of that task, `set :out g.x = task`, readers use `g.x.pub...` |
| `set :out pub.x = v`, data pinned in the task itself | create the datum in the `pub` (`set pub.x = [...]`), and name it (`val x = pub.x`) |
| `set :out g.x = v`, loose data | move the datum into the `pub` of the task that owns it, and refer to the task |
| `if g.x {...}` on a place of `set :out` | `if g.x && !(g.x ?? :xtask.dead.gone) {...}` |
| `atmos.status(t) == :dead` | `t ?? :xtask.dead` |
| `type(t) == :xtask` | `t ?? :xtask` |

## To watch

- `type(t)` is never exactly `xtask`: a comparison with `==` is false.
- A name that kept data read through a `pub` (`val bag = t.pub.bag`)
  refers to a dead table after `t` leaves its scope.
- A place that is never overwritten keeps a task that is out of scope in
  memory.
- `spawn { ... }` is transparent: `task` inside it is the enclosing
  task.
- The functions of the standard library written in C (`X.print`,
  `X.copy`, ...) see a dead table as empty, with no error (as before
  this change for a finalized table).
