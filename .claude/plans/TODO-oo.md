# TODO-oo: functions of the same name dispatched by tag

Session `atmos-oo`. Material: `261118-oo.md`. Cases: `261118-oo-cases.md`.

## Index of items

Items are `#N` (standing rule in `TODO.md`); from other sessions,
`atmos-oo#N`. A number is never reused or changed.

| # | Title | State |
|---|---|---|
| #1 | Is there a case that justifies syntax, or does a table/library do for now? | decided 2026-11-19: a table for now ("Tabela por enquanto") |
| #3 | A paragraph in the manual ("User Types") with the table idiom | waiting for the author (asked 2026-11-19); text on the branch `oo` |
| #4 | Port a few badguys of SuperTux as the test of inheritance of behavior | not asked yet; belongs to the session of `sdl-tux` |
| #2 | sdl-flare was not surveyed | closed 2026-11-19: surveyed after the author pointed that it is public; no case that repeats (`261118-oo-cases.md`, 2.5) |

## Waiting for the author

- **#3** Add to the manual, under "User Types", a paragraph and an
  example of the table idiom (a table from tags to functions or tasks,
  `AREA@(s.tag)(s)`). Recommended: yes. The text is on the branch `oo`;
  no change to the compiler.

## Next, one at a time

- **#4** Port a few badguys of SuperTux (deep inheritance in the
  original), to see whether inheritance of behavior asks for dispatch
  by tag or composition of tasks is enough. Recommended: propose it to
  the session of `sdl-tux`.
- If syntax is reopened: tasks as clauses first; then what is
  dispatched on (a tagged table or also a bare tag); the parent clause;
  values with no tag; one argument or several; where clauses may be
  declared; the grammar of `x :Tag` with `atmos-args`; prototypes as
  names with `atmos-pin` (`261118-oo.md`, section 4).

## Known and not done

- No port has inheritance of behavior: the badguys of SuperTux, the
  typical case, are not ported (`261118-oo-cases.md`, 2.6).
- Nothing implemented in the compiler; the branch `oo` has only the
  paragraph of #3.

## Reported and not acknowledged

- #3 (2026-11-19).

## Decided by the author

- #1, 2026-11-19: "Tabela por enquanto". No syntax for dispatch by tag
  now; the table idiom is what the language offers.
