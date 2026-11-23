# context: a filter of events (#19)

Session `atmos-context`, issue 21. The author asked for a syntax on
2026-11-22, after the report of `env-ray` (#20): `enter`/`leave` run on
every event that wakes a task inside (`:frame`, the clock, keys, the
`watching` of each coin), so `BeginMode2D` runs outside `BeginDrawing`,
2 to 3.5 times per frame, and a mode with side effects could not be
limited to `:draw`.

## 1. Neighbours on main (after `toggle` -> `mute`)

| Construct | Syntax | The pattern |
|---|---|---|
| `mute` (task) | `mute [<Patt>] Expr(Expr)` | between `<` `>`, since an expression follows |
| `mute` (block) | `mute [Patt] on TAG Block` | bare or between `<` `>` |
| `stage`, `solo` | `solo [Patt] Block` | bare or between `<` `>` |

All take an await pattern (`<:X>`, `<!:X>`, `<until ...>`), and no
pattern means every event, the clock included.

## 2. Proposal

```
Context : `context´ [`<´ Patt `>´] `(´ Expr `,´ Expr `)´ Block
        | `context´ [`<´ Patt `>´] Expr Block
```

```
context <:draw> ray.ctx.mode2d(0, 0, 0, 0, 0, SCALE) {
    spawn { loop on :draw { tiles } }       ;; inside the mode
    pin coins = tasks()
    spawn @coins Coin(...)                  ;; its watching: outside the mode
}
```

The pattern goes between `<` `>`, as in the first format of `mute`:
the expression that follows (`eng.on(X)`) would be ambiguous with a
bare pattern. No pattern is today's `context`, every event (#2).

## 3. Semantics

- A context with a pattern is applied only while the traversal of an
  emit that matches the pattern resumes code inside it. For any other
  emit the code inside runs as if the context were not there: the
  switch skips it (the outer contexts still apply).
- Outside an emit (the first reaction after a `spawn`, the defers of an
  `abort` from code, the end of a block) there is no event: a context
  with a pattern is not applied. It is applied only when a matching
  event is being traversed.
- The lazy switch stays: the stars of a pool in a `context <:draw>` pay
  one `enter` per `:draw`, and nothing on other events.
- `enter`/`leave` errors and checks are those of #5.

## 4. Cost

- Contexts without a pattern: unchanged (one test per resume).
- With a pattern: the switch matches the current event against the
  pattern of each filtered context in the chain, once per switch, not
  per task: the lazy switch keeps the count low. The event being
  traversed is kept by `emit_top` (one store per emit).

## 5. Alternatives

| | Form | Against |
|---|---|---|
| A (proposed) | `context <:draw> X {...}` | none found |
| B | `context X on :draw {...}` | `on` after the expression reads as `mute ... on TAG`, where the tag is what switches it, not a filter |
| C | `context X with :draw {...}` | `with` is the separator of `par`; a new meaning |
| D | a bare pattern, `context :draw X {...}` | ambiguous with the expression form (`context f(x) ...`) |

## 6. Tests to add

- `<:draw>`: a task inside that awakes on `:key` runs without the state.
- `<:draw>`: the first reaction of the block runs without the state.
- `<!:draw>`: the complement.
- nested: an outer context without a pattern and an inner one with.
- a pool in a `context <:draw>`: one `enter` per `:draw`, none per `:frame`.

## 7. Decided

- 2026-11-22, the author ("Sim"): A, `context <P> X {...}`, with the
  semantics of section 3. Implemented on the branch `context` 1d48c2a: the
  match is done once per emit per context (`ctime`), the zero tick of
  `mute t(false)` is the event `0`; an error in the pattern counts as
  no match.
- 2026-11-22, the author ("sim"): the three details above are confirmed.
