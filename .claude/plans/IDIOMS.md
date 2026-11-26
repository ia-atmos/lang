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
`env-iup#18` to `#23`), `pico-1oom#16`.

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
loop e on :button [h=cnv] until it.press {
    <...>
}
```

It does not apply when the condition needs something computed inside
the body.

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

### A small function is a lambda (pico-1oom#16)

The author, on getters and setters: "Pq não usar lambda?".

```
;; before
CFG_ITEM_BOOL("undo", func () { game_opt_undo_enabled }, func (v) { set game_opt_undo_enabled = v })

;; after
CFG_ITEM_BOOL("undo", \{ game_opt_undo_enabled }, \{ set game_opt_undo_enabled = it })
```

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
