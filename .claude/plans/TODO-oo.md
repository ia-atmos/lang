# TODO-oo: functions of the same name dispatched by tag

Session `atmos-oo`. Material: `261118-oo.md`. Cases: `261118-oo-cases.md`.

Coordination (2026-11-24): status and questions for the author go to
the session `atmos` (session_011eECohhJbc4GMpAj3yyeEo); measurements on
the ports and merges into main stay with `atmos-lang`
(`.claude/plans/261124-atmos.md`).

## Index of items

Items are `#N` (standing rule in `TODO.md`); from other sessions,
`atmos-oo#N`. A number is never reused or changed.

| # | Title | State |
|---|---|---|
| #1 | Is there a case that justifies syntax, or does a table/library do for now? | decided 2026-11-19: a table for now ("Tabela por enquanto") |
| #3 | A paragraph in the manual ("User Types") with the table idiom | by the session, to be confirmed (2026-11-20): text and two test cases, merged into main by `atmos-lang` (rebased: `a4bde26` the manual, `628ee8a` the tests) after the suite passed there |
| #4 | The badguys of SuperTux as the test of inheritance of behavior | done 2026-11-20 by reading the port of `sdl-tux` (`261118-oo-inherit.md`): #1 stands |
| #5 | Suggestion to `sdl-tux`: split `badguy.atm` by class as records of closures (form B of `261118-oo-inherit.md`) | open: passed by `atmos-lang` to `sdl-tux` on 2026-11-20; it is of `sdl-tux` to take or not |
| #6 | Whether the manual also shows inheritance (form B: a class is a function that fills a record of methods) | open: not written; recommended only after `sdl-tux` tries it |
| #7 | Clauses of a function chosen by the tags of its arguments (`func f (x :T)`, one or more parameters, tasks too): a prototype | on `oo` at `9991ed1` (2026-11-25): main merged in, the manual written, suite passing; measured by `atmos-lang` on 2026-11-25 (2197 files: same diagnostics, generated C the same once globals are renumbered; no port changes); done: merged into main by `atmos-lang` as `9c6458f` (2026-11-25, "#7 sim") (`261118-oo-clauses.md`) |
| #8 | A form to call the parent clause | decided 2026-11-23 ("Super"): `super(...)`, implemented on `oo` (`0ba6917`) |
| #10 | Update the ports that can use clauses (first `sdl-tux`, `badguy.atm`; then the `collision` and the characters of SNKRX) | passed on (2026-11-26): he said `sdl-tux` does `badguy.atm` ("sim, o sdl-tux faz"); told `sdl-tux`; nothing here |
| #11 | How `super()` works for tasks (his question, via sdl-tux#24) | answered (2026-11-26); he named it `Super` for tasks, `super` for functions; prototype on branch `oo-task` (not asked for: he wanted only the answer here), `await Super()` or `spawn Super()`; suite pass=1099 with ASAN; not to be merged unless he asks |
| #9 | An `await T(...)` whose spawn fails before the task exists marked the pattern as owning a task | done: on main (`a0f6262`, taken by `atmos-lang` on 2026-11-21); the branch `oo-own` is obsolete |

## Waiting for the author


To confirm or undo when he reads:

- **#3**, settled by the session under the rule "Work with no input":
  the paragraph and the tests `tst/cases/tag-table-01` and `-02` are on
  main (`a4bde26` the manual, `628ee8a` the tests). To undo: revert
  those two commits.
- **#1** was re-examined against SuperTux (#4) and stands.

## Next

- **#5**, **#6** (see the index).
- If syntax is reopened: tasks and closures as clauses first (the
  methods of a badguy are closures over a task); then what is
  dispatched on (a tagged table or also a bare tag); the parent clause;
  values with no tag; one argument or several; where clauses may be
  declared; the grammar of `x :Tag` with `atmos-args`; prototypes as
  names with `atmos-pin` (`261118-oo.md`, section 4).

## Known and not done

- The branch `oo` has the prototype of #7 and #8 (parser, runtime, 10
  test cases, the manual), over main of 2026-11-25. Not done: `super`
  in tasks, local and anonymous clauses, a cache.
- The originals (C++, Lua) were not read: what is said of them comes
  from the `Class::method` comments of the ports and from memory.

## Reported and not acknowledged

- #3 as settled by the session, #4, #5, #6, #7, #8 (2026-11-20).

## Decided by the author

- #1, 2026-11-19: "Tabela por enquanto". No syntax for dispatch by tag
  now; the table idiom is what the language offers.
- #7, 2026-11-20: "ok va em frente", to the prototype of clauses with
  one or more tagged parameters, chosen by specificity, with no
  grouping and no order. Not yet an approval to merge.
- #10, 2026-11-26: "sim, o sdl-tux faz": the ports do their own rewrites.
- #11, 2026-11-26: `Super` for tasks, `super` for functions; this session
  only answers, the game is done in `sdl-tux`.
- #8, 2026-11-23: "Super": the parent clause is called by `super(...)`.
- #7, 2026-11-25: "#7 sim", the merge of clauses and `super` into main.
- #7, 2026-11-23: merge the clauses and update the ports that can use
  them, but wait until next week ("agora iremos aguardar").
