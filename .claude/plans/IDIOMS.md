# Idioms of Atmos

The rules that came out of the author's reviews of Atmos code in the
projects, gathered for every session (created 2026-11-26 by `atmos`, on
the author's "crie", `atmos#29`). Read this file before writing Atmos.

How it grows: when the author corrects or approves an excerpt, the
session records the rule in its own project and sends it to `atmos`,
with where it came from (`session#N`) and a before and an after.
`atmos` adds it here. A rule here does not change the language: when a
correction needs a change of the compiler, it goes to `atmos-lang`.

Sources so far: `env-iup` (its `.claude/plans/IDIOMS.md`, items
`env-iup#18` to `#23`), `pico-1oom#16`, `env-ray#48` and `#49`, `atmos-oo#11`. The
examples of `context` follow lang d0fe0ac.
Since lang 9d4561a the predicate of a `loop e on` names the event by
the variable of the loop (`until e.press`, not `it.press`).

## In a program

### A condition that only filters an event goes in the pattern (env-iup#22)

The loop does not wake for the events it would discard.

```
;; before
loop e on :button [h=cnv] {
    if e.press {
        <...>
    }
}

;; after
loop e on :button [h=cnv] until e.press {
    <...>
}
```

It does not apply when the condition needs something computed inside
the body.

### A predicate of more than one term goes in parentheses (env-iup#20)

A refinement of the rule above. The author: "Para um until desse
tamanho, vale a pena parenteses em volta".

```
loop e on :button until (e.press && (e.but == :left) && e.status.double) {
    <...>
}
```

### Exclusive cases go in an `ifs` (env-iup#22)

```
;; before
if (e.but == :left) && (c == nil) { <...> }
if (e.but == :right) && (c != nil) { <...> }

;; after
ifs {
    (e.but == :left) && (c == nil)  => { <...> }
    (e.but == :right) && (c != nil) => { <...> }
}
```

### A loop with no variable has none

`loop in n {...}`, not `loop _ in n {...}` (the author, 2026-11-22). A
`loop _,v in t` that uses the value stays.

### A statement takes its argument with no parentheses (env-iup#18)

`emit`, as `await`, `throw`, and `return`, is a statement, not a call.

```
;; before
emit(:action [h=but])

;; after
emit :action [h=but]
```

As the left operand of a binary operator, a call or an await with no
parentheses is an error since lang 0e270fa (atmos-lang#141, #143, #145,
from pico-frescogo#29): the author, "Por fora. Pois queremos usar a forma
sem parenteses. Exemplo: set x = f y". Keep the form with no
parentheses, and put the parentheses outside when it is an operand.
`1 + f y` stays valid: only the left operand is concerned.

```
;; before (an error: invalid call : use parentheses to disambiguate)
set x = f y + 1
;; after
set x = (f y) + 1                    ;; or f(y + 1), if that is meant

;; before (an error: invalid await : use parentheses to disambiguate)
await CONF.radar.resaque * 1ms
;; after
await(CONF.radar.resaque * 1ms)      ;; the duration
                                     ;; or (await x) * 1ms, the value of the await
```

### A small function is a lambda (pico-1oom#16)

The author, on getters and setters: "Pq não usar lambda?".

```
;; before
CFG_ITEM_BOOL("undo", func () { game_opt_undo_enabled }, func (v) { set game_opt_undo_enabled = v })

;; after
CFG_ITEM_BOOL("undo", \{ game_opt_undo_enabled }, \{ set game_opt_undo_enabled = it })
```

### What follows a state change goes in `:enter` (env-ray#49)

What must happen right after a context sets its state (a clear after the
begin) goes in its `:enter`, not in a separate first task inside the
context.

```
;; before
context <:draw> ray.ctx.texture_mode(target) {
    spawn {
        loop on :draw { ray.clear(GREEN1) }
    }
    loop { <...the screens...> }
}

;; after
context <:draw> {
    :enter { ray.begin_texture_mode(target) ; ray.clear(GREEN1) }
    :leave { ray.end_texture_mode() }
    loop { <...the screens...> }
}
```

### A pool is declared where its awaiter sees it (env-ray#48)

A pool whose tasks draw in a mode is bound with `in`, declared where the
code that awaits it can see it; no invented event to get its end out.

```
;; before
spawn {
    context <:draw> ray.ctx.mode2d(<...>) {
        pin coins = tasks()
        <...>
        await :all coins
        emit :taken
        await false
    }
}
await :taken

;; after
val world = ray.ctx.mode2d(<...>)
pin coins = in <:draw> world, tasks()
<...>
await :all coins
```

### `super` in a function, `Super` in a task (atmos-oo#11)

The author: "deveria ser `Super` para task e `super` para funcao", as
task names are capitalized. `Super(...)` is the parent task, to be
awaited or spawned.

```
;; before
task Bad (b :Bad.Walker) {
    par { await super() } with { loop on :wall { turn(b) } }
}

;; after
task Bad (b :Bad.Walker) {
    par { await Super() } with { loop on :wall { turn(b) } }
}
func area (s :Shape.Rect.Sq) { super() + 1 }
```

Caveat: `super` for functions is on lang main (9c6458f); `Super` for
tasks exists only on the branch `oo-task` (a84cb6c), not on main.

### The idiom of the library is fine (env-iup#23)

A datum of the program kept in the control, as IUP does
(`cell=id` at creation, `iup.get(e.h, "CELL")` in the event), is not a
workaround: the C version does the same.

## In an environment

### An event does not carry an encoding of the library (env-iup#19, env-iup#21)

What the library packs in a string of fixed positions or in a bare
number is decoded by the environment, with the tests of the library
itself: a table of booleans, a tag.

```
;; before
(string.sub(e.status, 5, 5) == "D")     ;; the 6th byte of a string of IUP
(e.but == 1)                            ;; the left button

;; after
e.status.double
(e.but == :left)
```

The same thing has one name everywhere: the buttons are `left`,
`middle`, `right` both in `e.but` and in `e.status`.
