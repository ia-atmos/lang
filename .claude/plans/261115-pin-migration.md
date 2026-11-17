# Migrating a project to the new rules of `pin`

For every session that has code in Atmos (ports, environments,
examples). Written by the compiler session (`ia-atmos/lang`). The
rules were approved by the author on 2026-11-15; the compiler has them
since the commit `98ad03a` of `main`. The plan and what was added to
the rules are in `261115-pin.md`; the reference is `doc/manual.md`,
sections "Lexical Memory Management", "Pin", "Pub", "Native".

## What changed, in short

- Every dynamic value (table, function, native, task, pool) belongs
  to a block. It is *fresh* when it is created, and *pinned* when a
  block owns it.
- **`val` and `var` no longer accept a fresh value.** The first
  declaration is always `pin`; a `val` only refers to a value that
  has an owner. `val t = [1, 2]` is now the error
  `invalid assignment : expected pinned value`.
- A fresh value stored in a pinned value (a field of a pinned table,
  the `pub` of a pinned task) is pinned with it, in the block of the
  holder. The pin is deep.
- A fresh value may hold pinned values: `f([x=a])`,
  `await :close [h=dlg]`, `emit :close [h=dlg]`, `[] ++ ships ++
  shots` now work with no pin. The value is born with a *floor*, the
  block of what it holds, and cannot leave it.
- A function may pin what it receives: `task H (c0) { pin c = c0 }`.
- A captured variable belongs to the block that declares it:
  `set r = spawn @ts T()` inside a function now works when `r` and
  `ts` are in the same block.
- A fresh task that is dropped is aborted at once (a `spawn` in an
  expression whose value is not used, a result that the caller
  ignores). It was kept until the end of the task that spawned it.
- What a result of a block holds leaves with it, tasks and pools
  included.
- The message of an escape changed:
  `invalid escape : table pinned at prog.atm:12`.

## Steps

1. Record the state of your tests with the compiler you use today
   (traces, screenshots): the migration must not change them.
2. Move your checkout of the compiler to `main` (`98ad03a` or later),
   and the commit that your setup script fixes.
3. Compile and run. Since `612af83` a constructor or a function
   declared with `val` or `var` is a compile error (`invalid
   declaration : expected 'pin'`), also on paths that never execute.
   For the rest, each `invalid assignment : expected pinned value` at
   `file:N` at run time: change `val` or `var` to `pin` in that line,
   and repeat. The run-time error only shows on the paths that
   execute: read the rest of the code for `val x = f()` where `f`
   builds a value, and `val ok, e = catch`.
4. Rewrite what a change of keyword does not solve (next section).
5. Remove the workarounds that the old rules forced (item 40 of the
   list of the compiler): pins added only to pass a value in a
   constructor, tables of context, tasks kept in tables to be
   assigned from a function, the task that accepts its own
   connection.
6. Check that your tests give what they gave in step 1. Expected
   outputs that contain `invalid escape : pinned value` change to the
   new message, and nothing else should.
7. Commit in small grains, and report (last section).

## What a change of keyword does not solve

| Before | Now | Why |
|--------|-----|-----|
| `val M = [...]` (the table of a module or of an environment) | `pin M = [...]` | A fresh value is declared with `pin` |
| `val rect = [...]` then `set pub = [rect=rect]` in a task | `set pub = [rect=[...]]` then `val rect = pub.rect` | The `pub` is visible outside the task: it cannot hold a value pinned in its body. The task owns what it creates in the `pub` |
| `var acc = [0,0]` ... `set acc = [0,0]` (since 2026-11-17: `pin acc = [0,0]` ... `set acc = [0,0]`) | `pin acc = [0,0]` ... `set acc@0 = 0 ; set acc@1 = 0`, or keep it as a field of a pinned table (`set st.acc = [0,0]`) | A variable does not own: it cannot receive a fresh value |
| `var cb` ... `set cb = make()` (a fresh table or closure from a call) | a field of a pinned table: `pin st = [] ; set st.cb = make()` | The same. `set f = func () {...}` is accepted: it is the declaration of a prototype |
| `set list@+ = func () { set k = k+1 }` with `k` a `var` of an inner block | the block gives the closure as its result: `set list@+ = do { var k = 0 ; func () {...} }` | A closure that captures a variable belongs to the block of the variable |
| `func f () {...}` inside a block, capturing a local of the block | `val func f () {...}` | The first form declares a variable of the program, which outlives the block |
| `val ok, e = catch :X {...}` with a table thrown | `pin ok, e = catch :X {...}` | The thrown value arrives fresh |
| `pin e = [...] ; emit(e)` to tasks outside the block; `pin e = await(:X)` | `emit(:X [...])` with the event built in the emit; `val e = await(:X)` | An event is shared by who receives it: no block owns it, and a receiver cannot pin it |
| `keep(spawn T())`, `(spawn T())`, a function that spawns and whose result is ignored | `pin t = spawn T()`, a pool, or a field of a pinned table | A fresh task that is dropped is aborted at once |
| `pin tmp = [] ; <fill tmp> ; set outer.list@+ = [x=tmp]` (a value that is built and then given to a table of an outer block) | `set outer.list@+ = do { pin tmp = [] ; <fill tmp> ; [x=tmp] }` | A pinned value does not leave its block, except as the result of the block: the store then pins it outside |
| `var shown = []` ... `set shown = []` (to empty a vector) | `pin shown = []` ... `vector.clear(shown)` | A variable does not own a fresh value |
| `val new = [...] ; vector.insert(cs, new)` with `cs` of an outer block | `val new = vector.insert(cs, [...])` | The insert pins the fresh value in `cs` and gives it back |
| `var pos` ... `set pos = get_mouse()` (a variable that follows the last result of a function) | `pin pos = [x=0, y=0]` ... `val p = ...` is not possible either: copy the fields (`pin p = get_mouse() ; set pos.x = p.x ; set pos.y = p.y`), keep it as a field of a pinned table (`set st.pos = get_mouse()`), or let the function return numbers (`val x, y = get_mouse()`) | A variable does not own a fresh value |
| `x or []` assigned to a `val` or a `pin` | decide: `pin y = if x { X.copy(x) } else { [] }`, or keep it as a field | A value that is sometimes fresh and sometimes owned fits neither |

## C in an environment

- A value that C creates (`a_tab`, `a_func`, `a_nat`) is fresh. If it
  goes to the program as the result of a function, the program pins
  it (`pin tex = sdl.image(...)`).
- A value that C keeps for the whole program and gives to the program
  many times (a default font, a table of constants) must have an
  owner: call `a_own(v)` once, when it is created.
- An event that C emits (`atm_emit`, `atm_emit_to`) is owned by the
  emit: nothing to do, unless it holds a value of the program.
- **An event built in C that holds a value pinned in the program**
  (the handle of a control, in `[h=btn]`): a plain `a_setindex` into
  the fresh table of the event now fails, and the field is not set.
  Build the event with `a_tab_field(evt, key, v)`, which is a field
  of a constructor (the table takes its floor from the value), and
  do not drop its status. Found by `env-iup`: the symptom was an
  `await :action [h=but]` that never matched.
- `a_nat_holds(child, parent)`, right after the child is created,
  declares that a native holds another value: the child is born with
  its floor in the block of the parent, and is finalized before it.

## What to report

To the compiler session (`ceu-atmos`, session_011eECogKVQ7ZzmWCoMST2wK),
by message, when the migration is done and pushed. The author asked
for it, and it goes to him as it is:

1. **Was every problem of `pin` of this project solved?** Which
   workarounds were removed, with the file and the line of each.
2. **What is strange, not intuitive, or still needs a workaround**,
   each with a minimal snippet (5 to 10 lines).
3. **The cost**: how many lines changed from `val`/`var` to `pin`,
   how many places needed a rewrite and of which kind (the table
   above), and whether the tests are identical.
4. Anything in the compiler that looks wrong: a small program that
   reproduces it.

Do not wait for the report to ask: if a rule blocks you and no
rewrite of the table fits, send the snippet at once.
