# TODO: session `atmos-threads`

The session that discusses threads in Atmos with the author, and
prototypes them on the branch `threads` of this repository (cut from
`origin/main` at `13d753f`). Material: `261122-threads.md`. This session
does not merge into `main`: when the author approves a merge,
`atmos-lang` measures the ports and merges.

Last update: 2026-11-26.

Work with no input: `261125-threads-runtime.md`, the global state a
thread meets in `rt/` (by the session, to be confirmed).

## Index of the items (`atmos-threads#N`)

| # | Title | State |
|---|---|---|
| #1 | What threads are for | decided by the author, 2026-11-26: computation. "É um bloco restrito sem a semântica de concorrência estruturada, sem pin, etc. Basicamente pra cálculo mesmo, mas com a semântica quase toda da parte não concorrente de atmos" (the middle way of `261125-threads-runtime.md`) |
| #2 | How values of the task enter the block and the result leaves it | decided by the author, 2026-11-26 ("#2.1"): deep copy in at the start, deep copy out at the end, the block's own heap freed when it ends (as Lua Lanes: one `lua_State` per lane, arguments, upvalues and results copied, strings interned per state; only "deep userdata" is shared, with its own lock). Nothing to implement for now (author: "Nao vamos implementar nada agora") |
| #3 | A shareable type between the program and a computation block (as the "deep userdata" of Lanes) | to explore, asked by the author 2026-11-26 ("Vamos explorar isso. Me acorde na quarta que vem"); wake-up scheduled for 2026-12-02 10:00 (Sao Paulo). Author, 2026-11-27: "Acho que esse novo tipo precisaria usar um lock v {} em volta sem concorrencia dentro" (he asked for no comment) |

## Facts (2026-11-22)

- No support: `thread` is a reserved word, parsed as a prefix with no
  grammar (`src/lexer.c`, `src/parser.c`).
- The reference: `val v = thread { ... }` inside a task; the block runs
  elsewhere, cannot await (`0516`), its value is the value of the
  expression (`0517`, `0518`); an abort of the task abandons it (`0519`).
- The runtime is single threaded: non-atomic reference counts, global
  library tables, global scheduler and trace, values owned by blocks.

## Shapes on the table

| Shape | What it is | Cost to the runtime |
|---|---|---|
| A. Expression `thread { ... }` | as the reference; the task awaits, the result arrives as an event of the loop | a second interpreter state, or copies in and out |
| B. Library call `atmos.thread(f, ...)` | gives a task to await; same mechanics as A, no grammar | same as A |
| C. Environments only | a C call runs in a pool (libuv, FreeRTOS) and emits its result; no Atmos code off the main thread | none in the runtime |

## Coordination (since 2026-11-24)

- `atmos` (session_011eECohhJbc4GMpAj3yyeEo) is the managing session
  (atmos-lang#119, handoff `261124-atmos.md`): coordination, status and
  questions for the author go through it. Measurements on the ports and
  merges into `main` stay with `atmos-lang`.
- Item numbers stay local to this session (standing rule #71, confirmed
  by the author 2026-11-24): `#1`, `#2`, ... here, seen from outside as
  `atmos-threads#N`. Only `atmos` and `atmos-lang` share the sequence of
  `ISSUES.md`.
- Before writing Atmos, read `.claude/plans/IDIOMS.md` on main (standing
  rule `atmos#29`, 2026-11-26). A general rule that comes out of the
  author's review of an excerpt of this session goes to `atmos`
  (source `atmos-threads#N`, a before and an after).

## Strings (the author's question, 2026-11-26)

The text of a string is immutable, but its header is not: every push
and pop counts it (`a_inc` / `a_dec`, not atomic), `a_str` inserts in
the one intern table `TAB` when a string is made (concatenation,
`tostring`), and the last release removes it. Equality is by pointer
(`a_same`), so a string made in the block must be re-interned in the
main table when it crosses back. So: no private copy of the text is
needed, only no shared writes to the headers and to `TAB`.
