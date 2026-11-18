# pin: handoff to the session `atmos-pin`

On 2026-11-18 the author decided that everything about `pin` is
discussed, decided, and implemented in the session `atmos-pin`
(session_011eECogSDhkVfyt7qme5DLh, branch `pin`), and no longer in the
session that maintains the compiler (`atmos-lang`). This file hands over
what `atmos-lang` held. From now on the ports report what they find
about `pin` to `atmos-pin`.

What `atmos-lang` keeps doing for `pin`, as for any language session:
measuring a branch on the ports before a merge, migrating the tests of
this repository after it, and telling the ports.

## 1. Three decisions that were waiting for the author in `atmos-lang`

Each was recommended ("do it") and not answered. They are now yours to
put to him.

1. **Refine "a deep pin adopts only unnamed values"** (decision 6 of
   2026-11-17). Today a named unpinned value that a pinned value holds
   is only referenced. Proposed: it is adopted, as before that decision,
   when the block of the holder is the block of its name or an outer
   one (safe: the holder outlives the name), and only referenced when
   the holder is in an inner block (the Pingus case, `pin over =
   \(x,y) { inside(r, x, y) }` with `r` a parameter). Cases that need
   a workaround today:
   - Lessmilk, `lib/scene.atm` `node_add(g, n)`: a tree of display
     nodes that grows during the game. `set g.kids@+ = n` with `n` a
     parameter only references the node, so the group stays unpinned
     and cannot receive children that hold globals. Rewritten as
     `node_add(level, \{ Sprite(IMG, 0, 0) })`, which passes a
     function so that the node arrives unnamed. Its session: "passing a
     value through a parameter changes whether it is adopted".
   - env-ray, drturtle: `set game.enemies = [] ; val es = game.enemies`
     kept instead of `val es = [] ; set game.enemies = es`, which would
     only reference a value named in an inner block.
   Minimal program (fails at the last `add`, `invalid escape : table
   pinned at m.atm:1`):
   ```
   pin IMG = [ w=1 ]
   func Spr (img) { :Spr [ img=img ] }
   func add (g, n) {
       set g.kids@+ = n
       g.kids@(#g.kids - 1)
   }
   task T () {
       pin level = [ kids=[] ]
       val snake = add(level, [ kids=[] ])
       val s = add(snake, Spr(IMG))
       print(s.img.w)
   }
   spawn T()
   ```
   Your cycle collection may change the trade-off: check whether it
   already makes this case work.
2. **Extend the compile-time warning** `pinned value given away by its
   block (expected 'val')` (resolver, `chk_giveaway`, `c325dd4`) to a
   pin given away inside a constructor that is the value of the block
   (`pin ps = [] ... [ps=ps]`), and to `break x` and `escape(:X, x)`.
   Missed by the warning and by the static scan in the migration:
   Flare 5 (a field of the returned table), env-ray 2 (`[..., ps=ps]`)
   and 1 (`break p`), env-uv 1 (`break c`), env-iup 1 (`return x,
   deps`), pico-1oom 1 (`return rest, version`), Doom 1 (the value of a
   `do` stored in a field), Pingus 1.
3. **A line in the manual**: `pin var` is for values that the block
   creates; `var` for those it only refers to. From Flare: `pin var
   next = [act=...]` then `set next = await act(e)`, where the action
   gives a value that the `pub` of an enemy owns: `invalid assignment :
   unexpected pinned value`, only from the second iteration (frame 21
   of a run), since the first value is unpinned.

## 2. The item of the list of `atmos-lang` (number 40), as it stood

| 40 | The rules of `pin`, approved by the author on 2026-11-15, are implemented here and carried to the eleven projects, all with identical tests (`261115-pin.md`; the single report went to the session of the rules on 2026-11-16). About 1770 lines changed only in the keyword, and about 240 places rewritten. Open, for the author: the two most repeated complaints (a variable cannot receive an unpinned value, in 8 of 11 projects; a value that is sometimes unpinned and sometimes owned has no declaration, in 4); a refinement of rule 1 (the floor of an unpinned value may rise to the outermost block of the program); a longer-lived place that refers to what a shorter-lived block owns (the scoped assignment, or weak references); and the ten choices that the implementation added to the rules. **On 2026-11-17**, in the session of the rules, he approved five more, implemented here (`8a5a202`..`c2a41ea`): `pin val`, `pin var`, and `func f (pin x)`; no implicit give-away (a pinned value never leaves its block, `a_blk_xfer` no longer unpins); a pool held by a `val` checked by the block of its name; cycles documented as the case for `pin`; a deep pin adopts only unnamed unpinned values. Still open there: a flag that removes the checks (item 3 of that list, "not yet"); declarations `func`/`task` as a `val` (with these rules, a function declared in a block cannot be returned, recursive ones included: `func-rec-01` had to call it inside); the `spawn` statement as the only implicit pin; choices 2, 5, 6 of the ten; the scoped assignment `with` | Wait for him. Recommended: the refinement of rule 1, which is sound and small; declarations as a `val`, with `pin func` for an owner |

## 3. Choices of `atmos-lang` that the author did not confirm

- **A bare `pin x` is a `pin val x`** (2026-11-17): it may not be
  assigned; only `pin var` may. The author approved `pin val` and `pin
  var`, and did not say whether the bare form stays. Kept, so that the
  ~1770 pins of the ports keep compiling; the few that are assigned
  (Flare 1, SNKRX 1, by a scan) become `pin var`.
- **A `pin` parameter is sugar** for `pin x = x` at the top of the body
  (the AST shows it so), and works in `func`, `task`, and lambdas.
- **A branch of `par :any` gives only the values its destination
  takes** (`444b54e`): `val e = par :any { pin vis = tasks() ; ... ;
  await :any vis }` raised an escape of the pool, since `await :any`
  gives (value, task, pool).
- **The floor of an unpinned value rises only with one reference**
  (`031f106`, approved): refused when another value already holds it
  (Lessmilk's `Group`: build the list in a `val` first).
- **An exit before a boxed `pin var` is declared** ended a box that did
  not exist (`a087697`, a crash found by Flare).

## 4. Still open in the rules (from the author's earlier answers)

A flag that removes the checks ("not yet"); the scoped assignment
`with`, or another form for a longer-lived table that refers to what a
shorter block owns (case E; your experiments in sdl-flare
`exp-map-context` and ray-lessmilk `exp-screen-context`); the `spawn`
statement as the only implicit pin; choices 2, 5, 6 of the ten of
2026-11-15; a cyclic value returned by a call and dropped without a
name (the known limit of your cycle collection).

Two remarks of the ports for the author, relayed and not answered:
- env-uv: a function that creates a handle and gives it away declares
  it with `val`, the weaker word, for the most "owned" value of the
  file; if it throws in the middle, the handle is closed by the count
  and not by the block.
- Doom and env-uv: the same lines changed three times in three days
  (`val` -> `pin` -> `val`); the rules were declared stable on
  2026-11-17 (`261117-pin-migration-2.md`, "Stable"). A change that
  breaks programs again should say what migrates.

## 5. The second migration of the ports (rules of 2026-11-17)

| Session | State | Note |
|---|---|---|
| pico-frescogo | done | no code with pin yet |
| pico-1oom | done | 5 pins to `val`; 109 saves identical |
| doom | done | 1 place; the WI state back to one constructor with `pin var` |
| sdl-tux | done | 2 places; forced pins back to `val` |
| env-sdl | done | Rocks back to the direct forms |
| env-iup | done | 6 lines in `exs/7guis/cells.atm` |
| env-pico | done | 2 places; the author approved reverting the forced pins |
| env-uv | done | 3 places; `pin` parameter, `fs-10` back to `pin var` |
| sdl-pingus | done | 10 pins to `val`; 3 workarounds removed |
| env-ray | done | 6 places; holders back to `pin var` in 4 games |
| sdl-flare | done | 19 pins to `val`; holder tables removed |
| ray-lessmilk | done | 20 pins to `val`; `Group` and `node_add` rewritten |
| sdl-snxkx | NOT reported | static scan: 36 given away, 1 assigned; reminded on 2026-11-18 |

## 6. Where things are

- The rules as implemented: `doc/manual.md` ("Lexical Memory
  Management", "Pin", "Prototypes", "Pub", "Native").
- History and reports: `261115-pin.md` (the plan, the reports of the 11
  projects, the strange cases A to H, the five decisions of
  2026-11-17), `261115-pin-migration.md` and `261117-pin-migration-2.md`
  (the guides sent to the ports).
- Runtime: `rt/task.c` (`a_pin`, `a_hold`, `pin_deep`, `pin_kid`,
  `adopt_ok`, `a_ceil`, `a_unname`, `a_blk_xfer`, `a_retire`), header
  fields in `rt/atmos.h` (`AObj`: `pinned`, `ceil`, `depth`, `lin`,
  `clin`). Compiler: `src/coder.c` (`gen_dcl`, `gen_assign_x`,
  `blk_close`), `src/resolver.c` (`chk_giveaway`), `src/parser.c`
  (`pin val`, `pin var`, modifiers of parameters in `proto_body`).
- Tests: `tst/cases/` `pin-*`, `repin-*`, `ceil-01`, `deep-*`,
  `xfer-*`, `floor-up-01`, `pool-val-01`, `par-mod-*`, `warn-pin-01`,
  `cycle-01`.
- A static scan for pins given away and pins assigned (a lower bound),
  used for the migration lists, is not in the repository: ask
  `atmos-lang` to run it on the ports.
