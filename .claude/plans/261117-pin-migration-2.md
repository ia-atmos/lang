# Migrating to the rules of `pin` of 2026-11-17

For every session with code in Atmos. A second step after `261115-pin-migration.md`.

The compiler (`ia-atmos/lang`, `main` at `84f8627` or later): the rules of `pin` changed again, approved by the author in the session on the rules of pin (2026-11-17). Everything since the migration you did:

1. "fresh" is now called "unpinned" (docs, messages, plans).
2. `val`/`var` accept an unpinned value again (the error "expected pinned value" and the compile error "expected 'pin'" are gone). A name gives the value a ceiling: it may only be pinned at the block of its outermost name or outside, else `invalid pin : table named at f:N`.
3. Case A: `set` of an unpinned value into a `pin var`, a field of a pinned table, or the pub of a pinned task works; the old value is released when its last reference goes (at the latest at the end of the block of the place).
4. `pin val x` / `pin var x`. A bare `pin x` is `pin val x` and can NO LONGER be assigned: a pin that is `set` later must be `pin var`.
5. Parameters take modifiers: `func f (var x, val y, pin a, pin var z, b, ...)`; `var` is mutable, `pin` makes the body own the argument (also `task` and lambdas).
6. NO IMPLICIT GIVE-AWAY: a pinned value never leaves its block. `do { pin s = ... ; s }`, `func f () { pin t = [] ; ... ; t }`, `return(t)`/`break(t)`/`escape(:X,t)` of a pin, or a task's final value that is a pin of its body, are now `invalid escape : table pinned at f:N`. Fix: declare it `val` (unpinned, held by a name), and let the receiver `pin` it.
7. A pool held by a `val` works: `func mk () { val ts = tasks() ; spawn @ts T() ; ts }`.
8. A deep pin adopts only unnamed values; a named value stored in or captured by a pinned value is only referenced (e.g. `pin over = \(x,y){ inside(r,x,y) }` with `r` a parameter now works; the `val over` workaround may go back to `pin`).
9. Declarations of prototypes are names, not pins: `func f`, `task T`, `val func f` hold an unpinned prototype, so a local function (recursive too) can be returned; `pin func f` gives an owner. Inside a function, `func g` with `g` unknown is now a local `val func g` (no longer an error).
10. Cycles: without a `pin`, reference counting applies, and an unpinned cycle leaks: a cyclic structure must be pinned (or held by a pinned value). The collection of unpinned cycles (2026-11-18..20) was removed by atmos-pin#18 (2026-11-21). A recursive closure through a variable is declared with `func` or with `pin var`.

Still open (do not work around yet): an unpinned holder whose floor rises (`val z = [] ; set z.k = k` with `k` pinned outside fails: write `val z = [k=k]`), `with`, a flag that removes the checks.

What to do: move your compiler to 84f8627 or later, run your tests, fix what fails (mostly item 6: `pin` -> `val` at the declaration that is given away; item 4: `pin` -> `pin var`), remove workarounds that items 2, 3, 7, 8, 9 make unnecessary, check your tests are identical, commit in small grains, and report once to ceu-atmos (session_011eECogKVQ7ZzmWCoMST2wK): what failed and how you fixed it, what workarounds you removed, and anything strange (with a 5-10 line snippet). Reference: doc/manual.md, "Pin" and "Prototypes". Each session got, by message, a static scan of its repository (a lower bound: it misses one-liners and values given away inside constructors).

## Stable (2026-11-17)

The author declared these rules stable until the next round of
decisions: no project needs to expect another migration of the same
lines. What is still open, and may change later: the scoped assignment
`with`, and a flag that removes the checks.
Since `5da4b8e`: a compile-time warning for a pin given away by its
block (`c325dd4`); `X.in` is the name of `X.xin` (an alias, to be
removed); a branch of `par :any` gives only the values its destination
takes (`444b54e`).

Later on 2026-11-17, decided by the author: the floor of an unpinned
value that only its name refers to rises when it receives a value of an
inner block, up to the block of its name. `val out = [] ; loop ... {
set out@+ = o } ; out` works, with `o` pinned outside.
