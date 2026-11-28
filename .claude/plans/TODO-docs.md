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
| 2 | Tracking scheme: a base commit per document, a status command, three files for meld | approved 2026-11-28 ("a"); built: `etc/docs.sh`, moved to `.claude/bin/docs.sh` by #12, the table "Bases" below |
| 5 | The commit of `main` that his merge in progress started from (to set the bases; until then the bases are the first versions, so the status lists every automatic commit) | asked 2026-11-28 |
| 6 | Three states per document: pending (his version differs from the "compared" commit, the commit of `main` he compared against; not "seen", his correction), new (`git diff <compared>..main`), done | in discussion 2026-11-28, step by step; `.claude/bin/docs.sh` to be adapted once settled |
| 7 | The reference (base) of `README.md` | decided 2026-11-28: atmos-lang/atmos@5977b07:README.md ("a"), not a7d4a61 (one line, `# ceu-atmos`) |
| 9 | The reference of every document is the `main` of atmos-lang/atmos (the author: the "truth"), today a06998a. 5977b07 is 4 commits ahead of it on the branch `ccr-438e7160-xrby3i`, with changes only in plans: `doc/` and `README.md` are identical in both | reported 2026-11-28 |
| 10 | An `ia/` directory for material that only sessions maintain: the section "Development" of the README as `ia/development.md`, `etc/docs.sh` as `ia/docs.sh`; `.claude/plans/`, `tst/`, `src/`, `rt/`, `doc/` stay | replaced by #12 |
| 11 | Move `src/` and `rt/` under `src/` (`src/compiler/`, `src/runtime/`), done by `atmos-lang` with every dependent repository updated at once | decided 2026-11-28 ("#11b, mas nao hoje"): not today; remind him on Wednesday 2026-12-02 (a reminder is scheduled in this session) |
| 12 | Instead of `ia/`, `.claude/` holds what only sessions maintain: the section "Development" of the README (Test, Layout, Differences) as `.claude/development.md`, `etc/docs.sh` as `.claude/bin/docs.sh` | decided 2026-11-28 ("#12a"); done (744fee70); the section removed from `README.md` too, at his word ("pode mexer em README.md tb"), in 77c96766 |
| 13 | `papers/` to leave the repository root too | the author, 2026-11-28: "em algum momento ... nao agora" |
| 3 | After his version: automatic sessions edit directly, or write to a pending list | to ask after #2 |
| 4 | Landing pages in other repositories: which are in scope | to ask after #3 |

## Bases

Read by `.claude/bin/docs.sh` (atmos-docs#2). The base is the last commit of
`main` whose version the author has taken into his merge; a trailer
`Docs-Base: <doc> <sha>` in a commit of his overrides it when newer. The
pass is `first` until his first merge of the document (meld then shows
the reference, his version, and `main`), then `next` (the base, his
version, and `main`). This session updates the table on his word.

| Document | Base | Reference | Pass |
|---|---|---|---|
| `doc/manual.md` | a31920b | a31920b | first |
| `doc/guide.md` | 5d5ad76 | https://github.com/atmos-lang/atmos@5977b076d1adee4263c6cd6b3879d0c73da2d26c:doc/guide.md | first |
| `README.md` | a7d4a61 | https://github.com/atmos-lang/atmos@5977b076d1adee4263c6cd6b3879d0c73da2d26c:README.md | first |
