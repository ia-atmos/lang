# context: the block form of issue 21 (#21)

Session `atmos-context`. On 2026-11-26 the author said in `env-ray`
(checked in its transcript): "mande essa preferencia
`context { :init {} :enter {} :body {} :leave {} }`". It came from
`games/retro_maze_3d/main.atm`, where the screen is cleared right after
`begin_texture_mode`: today that is either a first task inside the block
or two inline lambdas, which he found bad. This reopens #4 (decided D on
2026-11-22: `context (enter, leave) {...}` | `context X {...}`).

## 1. Proposal

```
context [<P>] {
    :init  { ... }      ;; optional, once, before the first enter
    :enter { ... }      ;; optional, at each switch in
    :body  { ... }      ;; the code that lives in the context
    :leave { ... }      ;; optional, at each switch out
}
context [<P>] X {...}   ;; kept: X gives [enter, leave] (libraries)
```

```
context <:draw> {
    :enter { ray.begin_texture_mode(target) ; ray.clear(GREEN1) }
    :body  { loop { ...the screens... } }
    :leave { ray.end_texture_mode() }
}
```

| Question | Proposal |
|---|---|
| How `:leave` gets what `:enter` set (#3) | through a variable declared in `:init`, visible in the three other sections: `:init { var old }`, `:enter { set old = pico.set.layer(l) }`, `:leave { pico.set.layer(old) }`. The value returned by `:enter` is not used |
| What `:init` is | ordinary code at the start of the block, run once, outside the context (not bracketed); its variables and `defer`s live as long as the block (the issue's `acquire()` / `defer { release(res) }`) |
| `:enter`, `:leave` | compiled as functions closing over `:init`; same rules as today (#5): no `await`, and `emit`, `spawn`, `abort`, `mute` are errors |
| `:body` | the block of today: a transparent task |
| Order, presence | fixed order `:init :enter :body :leave`; only `:body` is required |
| `<P>` | right after `context`, as today: `context <:draw> {...}` |
| `context X {...}` | kept for libraries (`ray.ctx.mode2d(...)`, `eng.on(STARS)`, `pico.ctx.layer(l)`): no port changes |
| `context (enter, leave) {...}` | removed: the block form replaces it; no port uses it (they all use `context X`) |

## 2. A caveat for the clear in `:enter`

`:enter` runs at each switch in, not once per frame. With `<:draw>` and
a single context on the screen that is once per `:draw` (measured in
`env-ray`: exactly 1 per frame). If another context drawing elsewhere
switches in between, in the same `:draw`, `:enter` runs again and the
clear erases what was drawn. For `retro_maze_3d` this does not happen
(one texture context); the manual will say it.

## 3. Cost

Parser and coder only: the sections become the two functions and the
block of today; the runtime does not change. Tests: the context cases
in the lambda form move to the block form.

## 4. Refined with the author (2026-11-26)

- No `:init` (variables are declared in the block) and no `:body` (the
  rest of the block is the body).
- `enter {...}` and `leave {...}` are statements inside the block, like
  `defer`: they see the variables declared before them; the context
  applies from where `enter` executes (code before it runs without the
  state).
- `leave` receives `it`, what `enter` returned (or `leave \(x) {...}`).
- `enter`/`leave` once per emit: unchanged; they repeat only when code
  inside runs code outside (`emit`, `spawn` into an outer pool, `abort`,
  `mute`).
- Pending: `atmos-pin` on closures capturing the block's own locals.

```
context <:draw> {
    val target = ray.load_render_texture(640, 480)
    enter { ray.begin_texture_mode(target) ; ray.clear(GREEN1) }
    leave { ray.end_texture_mode() }
    loop { ... }
}
```

## 5. A context as a value, bound with `in` (#23, the author, 2026-11-26)

Decided one by one with the author:

- The value is `:Context [enter=f, leave=g]`; its identity is the value:
  what is bound to the same value shares one context, with no switch.
- `in [<P>] X, cmd` binds it; cmd is `spawn T(...)`, `spawn {...}`,
  `await P` (what the await starts, not the code after), `tasks(n)`, or
  `do {...}`. The comma is the separator he asked for; no new keyword.
- `in X, do {...}` replaces `context X {...}`; `context [<P>] {...}`
  only defines, with the sections `:enter` and `:leave` (tags, not the
  contextual names of section 4).
- Rejected by him: grouping without a block ("a indentação engana").
- Mine, to confirm: the filter goes right after `in` / `context`.

```
val world = ray.ctx.mode2d(0, 0, 0, 0, 0, SCALE)
pin coins = in <:draw> world, tasks()
in <:draw> world, spawn { loop on :draw { tiles() } }
spawn @coins Coin(c)
await :all coins            ;; #22: the pool is outside, no emit :taken
```
