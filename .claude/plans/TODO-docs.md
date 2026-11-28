# TODO of the session `atmos-docs`

Supports the author while he edits the documentation by hand (a meld of
the reference, the automatic edits, and his version). This session does
not write the documentation and never commits his version for him.

Last update: 2026-11-28.

## Documents in scope (map of 2026-11-28, `main` at ef93083)

| Document | Reference | First automatic commit | Automatic commits since the reference | Lines (reference to `main`) |
|---|---|---|---|---|
| `doc/manual.md` | atmos-lang/atmos@5977b07:doc/manual.md, copied unchanged in a31920b | ac02de2 (2026-11-10) | 153, plus 53 merges; last 6aa2014 (2026-11-27) | +2759 -749 |
| `doc/guide.md` | atmos-lang/atmos@5977b07:doc/guide.md (last changed there in a9fac62, same as their `main`) | 5d5ad76 (2026-11-26), a rewrite, not a copy | 1 after the port (15e51f0) | +5 since the port |
| `README.md` | none copied: written here in 3eb3e3e (over the author's initial commit a7d4a61); it "follows" atmos-lang/atmos's README | 3eb3e3e (2026-11-10) | 41 | +228 -36 |

Constraints: the examples of the guide are `tst/guide/*.atm`, checked by
`make test-guide`; env-web's step player needs the lead-in before each
example to end with a colon. Other repositories (env-web, env-* READMEs)
are not mapped yet: in scope only if the author says so.

## Items

| # | Item | State |
|---|------|-------|
| 1 | Map of the documents (above) | reported 2026-11-28 |
| 2 | Tracking scheme: a base commit per document, a status command, three files for meld | proposed 2026-11-28, waiting for the author |
| 3 | After his version: automatic sessions edit directly, or write to a pending list | to ask after #2 |
| 4 | Landing pages in other repositories: which are in scope | to ask after #3 |
