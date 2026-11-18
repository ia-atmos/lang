# atmos-context: a block that pushes and pops state on each reaction

Material for the session `atmos-context`, where the author discusses
issue 21 of `atmos-lang/atmos` ("Context primitive"). Gathered on
2026-11-18 by the session that maintains the compiler (`atmos-lang`),
from the code of the ports as it is today. Item 39 of the list of the
compiler session.

## 1. The issue (text only: its three comments could not be read here)

```
context {
    => {...}    ;; executes on enter
    <= {...}    ;; executes on leave
    ...         ;; normal block
}
```

In `lua-atmos`: `context(enter, leave, block)`. With `pico-atmos`:

```
context {
    => { pico.zet.color.draw 'white' }
    <= { pico.zet.color.draw 'black' }
    every :draw {
    }
}
```

## 2. Real cases: state of a library set and restored around drawing

| Port | Where | What it does today | Count |
|---|---|---|---|
| SNKRX | `src/shared.atm` (`Background`, each star of `Stars`), `eng/eng.atm` | Each drawing task switches the canvas and switches it back, every frame: `loop on :draw { eng.target(BACK) ; ... ; eng.target(MAIN) }`; every star of the background does it once per frame | 10 `eng.target` |
| env-ray | `games/wave_collector`, `games/repair`, `games/raylife`, `classics/platformer`, and Lessmilk | `ray.begin_texture_mode(target) ... ray.end_texture_mode()` and `ray.begin_mode2d(...) ... ray.end_mode2d()` around a part of the drawing, always inside one task | 14 |
| env-pico | `pico.atm` (its own documented idiom), tests | `val old = pico.set.layer(l) ; <...> ; pico.set.layer(old)`, and the same for the pencil (color, font) | 287 `pico.set.*` calls, the save/restore form in about 60 places |
| Pingus, Flare, Lessmilk (outside raylib modes) | none found | | 0 |

```
;; SNKRX, today
task Background () {
    loop on :draw {
        eng.target(BACK)
        <draw the checkerboard>
        eng.target(MAIN)
    }
}
```

## 3. Questions for the design

- "On every reaction": on every event that reaches the block, or only
  on the events it awaits? Switching the canvas on each key would be
  waste (the compiler session's open question of 2026-11-12).
- Enter and leave around what: each time a task of the block resumes
  and suspends (so tasks of different contexts interleave correctly in
  one frame), or once around the whole block per emit?
- Nesting: contexts inside contexts restore in reverse (a stack), as
  `defer` does.
- A pool inside a context: does each of its tasks run inside it (the
  case of the stars of SNKRX, one `eng.target` per pool instead of per
  star)?
- The difference with a function that wraps (`with_target(c, f)`) and
  with `defer`: what a context gives that those do not, in a reactive
  program (a task that awaits inside the block leaves the state set for
  others, unless the runtime restores it).
- Cost in the runtime: a hook on resume and suspend of the tasks under
  a context.
- The reference: `lua-atmos` has `context(enter, leave, block)`; read
  its implementation and tests.
