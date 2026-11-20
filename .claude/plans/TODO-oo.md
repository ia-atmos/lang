# TODO-oo: functions of the same name dispatched by tag

Session `atmos-oo`. Material: `261118-oo.md`. Cases: `261118-oo-cases.md`.

## Index of items

Items are `#N` (standing rule in `TODO.md`); from other sessions,
`atmos-oo#N`. A number is never reused or changed.

| # | Title | State |
|---|---|---|
| #1 | Is there a case that justifies syntax, or does a table/library do for now? | decided 2026-11-19: a table for now ("Tabela por enquanto") |
| #3 | A paragraph in the manual ("User Types") with the table idiom | by the session, to be confirmed (2026-11-20): text and two test cases on the branch `oo` (`484869b`), suite passing over main `c4e99c4`; sent to `atmos-lang`, through which it goes to main |
| #4 | The badguys of SuperTux as the test of inheritance of behavior | done 2026-11-20 by reading the port of `sdl-tux` (`261118-oo-inherit.md`): #1 stands |
| #5 | Suggestion to `sdl-tux`: split `badguy.atm` by class as records of closures (form B of `261118-oo-inherit.md`) | open: passed to `atmos-lang` on 2026-11-20; it is of `sdl-tux` to take or not |
| #6 | Whether the manual also shows inheritance (form B: a class is a function that fills a record of methods) | open: not written; recommended only after `sdl-tux` tries it |

## Waiting for the author

Nothing blocks. To confirm or undo when he reads:

- **#3**, settled by the session under the rule "Work with no input":
  the paragraph and the tests `tst/cases/tag-table-01` and `-02` are on
  the branch `oo`. To undo: do not merge the branch, or revert the
  commit.
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

- Nothing implemented in the compiler; the branch `oo` has the
  paragraph of #3 and its two tests.
- The originals (C++, Lua) were not read: what is said of them comes
  from the `Class::method` comments of the ports and from memory.

## Reported and not acknowledged

- #3 as settled by the session, #4, #5, #6 (2026-11-20).

## Decided by the author

- #1, 2026-11-19: "Tabela por enquanto". No syntax for dispatch by tag
  now; the table idiom is what the language offers.
