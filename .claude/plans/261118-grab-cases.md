# grab: the real cases, before and after

Session `atmos-grab`, 2026-11-18. The code "after" is a sketch of the
design under discussion (not decided): see `TODO-grab.md`.

## 0. The issue and its comment (atmos-lang/atmos#22)

The body (2026-01-29) proposes `grab P { ... }`: only the block reacts
to events that match `P`, released when the block ends; reentrancy is
open (a stack, or an error).

The comment (2026-05-20, by the author) says that a bare `grab` "breaks
lexical hierarchy by hijacking events from outer scopes", and asks for
an enclosing `grabbable` that authorizes it:

```
;; error: no enclosing `grabbing :key.dn`
grab :key.dn { ... }

;; delegates capture of `:key.dn` to its subtree
grabbable :key.dn {
    par {
        every :key.dn { ... }        ;; receives `:key.dn` normally
    } with {
        watching :X {
            grab :key.dn {
                every :key.dn { ... } ;; monopolizes within this block
            }
        }
    }
}
```

So the capture is bounded twice: above by the `grabbable` (what is
outside it never notices), below by the `grab` block (what receives).

## 1. Sketch used below

```
grabbable P { B }   ;; B may be captured, for events that match P
grab Q { B }        ;; an event that matches P and Q, traversing the
                    ;; nearest grabbable, reaches only this block
```

- `P`, `Q`: await patterns, so `<!:draw>` ("all but :draw") comes for
  free and replaces `toggle ... with :draw`.
- The nearest `grabbable` is found in the tree of tasks, not in the
  text: `Options` is a task spawned deep inside the arena and finds the
  `grabbable` of `main.atm`.

## 2. SNKRX: the options over the arena and the shop

Today (`src/options.atm`, `src/arena.atm:217`, `src/shop.atm:872`): the
modal is spawned INSIDE the screen it covers, so it cannot `toggle` it.
The time is slowed to 0 (`eng.slow`), which freezes the arena, but the
input is not: the buttons and cards of the shop still answer
`:mouse.mv` and `:mouse.dn` behind the modal.

```
;; main.atm
par {
    loop {                              ;; the screens
        val next = do {
            pin s = spawn (g.screen@0)(g.screen@1)
            val e = await <:goto || s>
            ...
        }
        ...
    }
} with {
    loop e on :goto { ... }             ;; the transitions
}

;; src/options.atm
task Options (kind) {
    ...
    loop {
        val e = await <:Button.click || :key.dn>
        ...
    }
    await TweenReal(0.25, st, [v=1], linear)
}
```

After:

```
;; main.atm: the screens may be captured; Input, WindowKeys and the
;; transitions are outside, and still see every key
par {
    grabbable true {
        loop { ... }                    ;; the screens, unchanged
    }
} with {
    loop e on :goto { ... }
}

;; src/options.atm: only the options see the input while open
task Options (kind) {
    ...
    grab <:key.dn || :key.up || :mouse.dn || :mouse.up || :mouse.mv> {
        loop {
            val e = await <:Button.click || :key.dn>
            ...
        }
    }
    await TweenReal(0.25, st, [v=1], linear)
}
```

`eng.slow` stays: it is about the time of the game, not the input.

## 3. SNKRX: the tutorial over the shop

Today (`src/shop.atm:741`): a toggle block driven by an event that a
sibling branch emits.

```
task Shop (run) {
    eng.cursor(true)
    par {
        toggle on :Shop.on with :draw {
            await ShopMain(run, task)
        }
    } with {
        loop {
            await :Tutorial.open
            emit @:task :Shop.on [false]
            val how = par :any {
                await Tutorial()
            } with {
                await RunClock(run)
            }
            emit @:task :Shop.on [true]
            if how == :escape {
                emit @:task :Pause.open
            }
        }
    }
}
```

After: no toggle block, no `:Shop.on`.

```
task Shop (run) {
    eng.cursor(true)
    par {
        await ShopMain(run, task)
    } with {
        loop {
            await :Tutorial.open
            val how = grab <!:draw> {
                par :any {
                    await Tutorial()
                } with {
                    await RunClock(run)
                }
            }
            if how == :escape {
                emit @:task :Pause.open
            }
        }
    }
}
```

Open: `grab` as an expression (the value of its block, as `do`).

## 4. env-iup: 7GUIs circle drawer (`exs/7guis/circles.atm`)

Today the modality is IUP's: `iup.set(dlg, "ACTIVE", false)` greys out
the main dialog, and IUP sends no events from it.

```
task adjust (c) {
    ...
    iup.set(dlg, "ACTIVE", false)       ;; the dialog holds the main one
    iup.show(adj)
    watching :close [h=adj] {
        loop on :value [h=sld] { ... }
    }
    iup.set(dlg, "ACTIVE", true)
    ...
}
```

After (with `grabbable true` around the main `watching`):

```
task adjust (c) {
    ...
    iup.show(adj)
    grab <!:action [h=cnv]> {           ;; the canvas is still drawn
        watching :close [h=adj] {
            loop on :value [h=sld] { ... }
        }
    }
    ...
}
```

Weak case: the grey look of a disabled dialog is IUP's and would be
lost; the toolkit already does modality. Kept for the filter
(`<!:action [h=cnv]>`).

## 5. env-ray: pause of the classics (`classics/classic.atm`)

Today: a sibling of the game toggles it, through its reference.

```
pin play = spawn Play()
spawn {
    loop {
        await :key.dn [key='P']
        toggle play(false) with :draw
        set phase = :pause
        watching :key.dn [key='P'] { ... "GAME PAUSED" ... }
        toggle play(true)
        set phase = :play
    }
}
await(play)
```

After (with `grabbable true` around the `loop` of `Classic`):

```
spawn Play()            ;; or kept in a pin, for `await(play)`
spawn {
    loop {
        await :key.dn [key='P']
        set phase = :pause
        grab <!:draw> {
            watching :key.dn [key='P'] { ... "GAME PAUSED" ... }
        }
        set phase = :play
    }
}
```

Neutral case: the pause is already outside the game, so `toggle`
works; `grab` only drops the reference. It also asks what happens to
the timers of the game: `<!:draw>` captures the clock, so the game
time stops, as with `toggle`; on release the game needs the zero tick
that `toggle t(true)` sends today (`rt/task.c`, `a_toggle`). Same for
env-sdl and env-pico (`exs/rocks/main.atm`).

## 6. Pingus (`main.atm`)

Today: the options are a sibling of the screens, which a `toggle on
:screens` block silences (no filter), driven by `emit :screens [b]`.

After:

```
grabbable true {
    par :any {
        ...the screens, without the toggle block...
    } with {
        loop {
            await <:go.options || :key.dn [key="F5"]>
            grab true {
                await Options()
            }
        }
    } with { await Overlay() } with { await :quit }
}
```

Open: `Overlay` (cursor, frames) would be captured too; it needs
`grab <!:draw>`, or to live outside the `grabbable`. The pause of the
clock in `game/game.atm:249` stays a `toggle` (time, not input).

## 7. Prior art

| System | Who receives | Nesting | Bound |
|---|---|---|---|
| Tk `grab` | the grab window and its descendants (local: this app) | one grab per display; a new one replaces the old, which the program must restore | `grab -global` for the whole display |
| X11 `XGrabKeyboard/Pointer` | the grab window | one active grab per device | the display |
| GTK `gtk_grab_add` | the widget and its descendants | a stack: the last added wins, removing it restores the previous | the application (window group) |
| IUP `IupPopup` | the modal dialog | nested modals stack | the application |

Closest to the sketch: GTK (descendants, stack). The `grabbable` bound
has no direct analogue; the nearest is the window group of GTK.
`lua-atmos` and `atmos-lang/atmos` have no grab (checked: no `grab` in
`src/`, only `toggle`).
