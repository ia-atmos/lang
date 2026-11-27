# Migration: `set :out` only for tasks (atmos-pin#35)

For the sessions of the ports. The change is on branch `pin` of
`ia-atmos/lang` and reaches `main` only after the author approves the
merge. The design is in `261127-out-tasks.md`.

## What changed

1. `set :out place = v` takes only a task or a pool. Anything else is
   `invalid set :out : expected task or pool : got table`.
2. The place is never reset to `nil`. When the block of the task ends,
   the task is aborted, and the place refers to a dead task.
3. A dead value keeps its reference valid (`==`, `??`, `await t`,
   `watching t`, storing, passing), and a use of what it held is an
   error, `invalid access : dead <task|pool|table|...> pinned at f:N`
   (it was `finalized`): the `pub` of a dead task, read or written; an
   iteration of or a `spawn` into a dead pool; an index, a length, an
   iteration of a dead table.
4. The `pub` ends with its task. Reading `t.pub` after `t` terminated is
   an error. It was readable until the block of the task ended.
5. A task may store in its `pub`, with a plain `set`, a value pinned in
   its body block (`pin spring = [...] ; set pub.spring = spring`). A
   value pinned in an inner block of the body is still `invalid escape`.
6. `type(t)` of a spawned task has its state: `xtask.running`,
   `xtask.suspended`, `xtask.dead`. Any other dead value has `.dead`:
   `tasks.dead`, `table.dead`. The type rule of `??` accepts a prefix,
   so `t ?? :xtask` is true in any state.
7. `atmos.status` is removed.

## How to migrate

| Before | After |
|---|---|
| `set :out g.x = t` with `t` a task or a pool | unchanged; `g.x` no longer becomes `nil`: test `g.x ?? :xtask.dead` (`:tasks.dead`), or `watching g.x {...}` |
| `set :out g.x = t.pub` (the pub of a task) | `set :out g.x = t`, and readers use `g.x.pub.f` |
| `set :out g.x = tab`, a table of a screen or game that is a task | keep the table in the `pub` of that task, `set :out g.x = <the task>`, readers use `g.x.pub...` |
| `set :out pub.x = v`, data of the task itself | `set pub.x = v` |
| `set :out g.x = v`, loose data | move the datum into the task that owns it, and refer to the task |
| `if g.x {...}` on a place of `set :out` | `if g.x && !(g.x ?? :xtask.dead) {...}` |
| `atmos.status(t) == :dead` | `t ?? :xtask.dead` |
| `type(t) == :xtask` | `t ?? :xtask` |
| `print(t.pub)` after `t` ended | read it before the end, or use the final value: `await t` |

## To watch

- A task that publishes a result in `pub` and terminates: the result is
  gone. Return it as the final value instead.
- `return(pub)`: the table of the `pub` is pinned with the task and ends
  with it, so the final value is a dead table. Return a value that is
  not the `pub`.
- The final value of a task (`await t`) lasts while the block of the
  task lasts; after that it is `nil`.
- A name that kept data read through a `pub` (`val bag = t.pub.bag`)
  refers to a dead table after `t` ends.
- A place that is never overwritten keeps a dead task in memory.
- The functions of the standard library written in C (`X.print`,
  `X.copy`, ...) see a dead table as empty, with no error (as before
  this change for a finalized table).
