# TODO: context (issue 21)

Session `atmos-context`. Material: `261118-context.md`. Design:
`261118-context-semantics.md`.

## Waiting for the author (one at a time, in this order)

1. **When enter and leave run** (asked 2026-11-18): around each resume
   of a task inside, lazy (recommended), or once around the block per
   emit. See `261118-context-semantics.md`, sections 3 to 6.
2. Filter: bracket every reaction inside, or only those to some events
   (`context ... with :draw`)? Recommendation, to confirm after 1: no
   filter (the lazy switch already costs nothing when nothing inside
   awakes).
3. Can `leave` receive what `enter` returned (the pico idiom
   `val old = pico.set.layer(l)` ... `pico.set.layer(old)`)?
   Recommendation: yes.
4. Syntax: the issue's `=> {}` / `<= {}`, the `atmos-lang` plan's
   `:init :enter :body :leave`, or `context (enter, leave) { ... }`
   with functions as expressions.
5. Errors: `emit`, `spawn`, `abort` inside `enter`/`leave`.

## Known, not done

- SNKRX (`ia-atmos/sdl-snxkx`) and env-ray (`ia-atmos/env-ray`): access
  refused to this session (add_repo denied). Their cases come from
  `261118-context.md`, not from their code.
- The three comments of issue 21: not readable (only search reaches
  GitHub outside `ia-atmos/lang`).
- Nothing sent to `atmos-lang` (formerly `ceu-atmos`) yet (no decision).
- Nothing told to `atmos-grab` yet: both touch the resume of a trail;
  tell it when 1 is decided.

## Reported, not acknowledged

- 2026-11-18: answered the author's question "does every reaction
  include events that are not drawing?": with the lazy resume, a key
  costs nothing unless a task inside awakes on it; if one does, that
  is decision 2 (filter).
- 2026-11-18: the author's note (from the other session) names the
  canvas of the stars `STARS`, not `BACK`: the examples here follow
  `261118-context.md`; to check in SNKRX when it can be read.

- 2026-11-18: `lua-atmos` has no `context` (no branch); `atmos-lang`
  has only a plan (`260628-context.md`), once per emit.
- 2026-11-18: no real case awaits between set and restore; `context`
  fixes no bug in the ports today, it saves writing and makes the
  stars of SNKRX one switch per pool.

## Decided

(nothing yet)
