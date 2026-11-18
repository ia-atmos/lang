# atmos-oo: the real cases in the ports

Survey of 2026-11-18 by the session `atmos-oo`, for the first question of
`261118-oo.md` (is there a real case for dispatch by tag?). Code only
(`data/` left out). Read: sdl-snxkx `00cca2c`, sdl-pingus `513617c`,
sdl-tux `a8bd119`, ray-lessmilk `bfa53a1`, env-ray `4daa572`.
**Not read: sdl-flare** (attaching it was refused in this session).

## 1. Counts

| Port | Lines | `match` | `ifs` | Choice by kind repeated in 2+ functions |
|---|---|---|---|---|
| sdl-snxkx | 12,740 | 63 | 34 | 1: `u.character` (52 characters) |
| sdl-pingus | 4,259 | 11 | 22 | 1: the 22 actions, already a table |
| sdl-tux | 1,075 (+ C headers) | 10 | 2 | 1, weak: `o.tag` of sector objects in view and session |
| ray-lessmilk | 9,138 | 1 | 62 | 0 |
| env-ray | 11,034 | 6 | 30 | 0 |

Most `match` choose by a key, a level, an option, an event: not by the
kind of an entity.

| Fact | Count |
|---|---|
| Entities built as tagged tables with a dotted tag (`:Enemy.Boss [...]`) | 0 in all five |
| Dotted tagged tables at all | 15, all events of SNKRX (`:Shop.buy`, `:Button.click`) |
| Kind of an entity kept as | a field: `kind=:enemy`, `character="outlaw"`, `elite="shooter"`; or the tag of a data record loaded from a level (`:exit`, `:tilemap`) |
| Choice by kind that walks a hierarchy | 0 |

## 2. The cases

### 2.1 SNKRX: the character of a unit (the one that repeats)

`src/characters.atm`: `match u.character` twice.

| Where | Arms | What an arm is |
|---|---|---|
| `task Character` (line 333) | 52 | the body of a task: a `loop` with `await Cooldown(...)` and `spawn @acts ...` |
| `task Shoot` (line 46) | 4 + default | statements that `spawn @shots ...` |
| `if u.character == "..."` | 22, in 4 files | a tweak inside another function (`"thief"` doubles the damage) |

The arms of `Character` use the locals of the task around them (`acts`,
`near`, `toward`), and are bodies of tasks, not of functions.

### 2.2 Pingus: the actions of a pingu (already open, with a table)

One file per action (`game/pingu/actions/digger.atm`, 22 files), each a
task. `actions.atm` fills a table by tag, and `pingu.atm` dispatches:

```
pin ACTIONS = []                                ;; pingu.atm
loop k, v in [ angel=Angel, basher=Basher, ... ] { set ACTIONS@(k) = v }
...
await ACTIONS@(act)(p, g, prev)
```

The facts of an action are in other tables by the same tag
(`ACTION_NAME`, `ACTION_MODE`, `ACTION_UNCATCHABLE`), and `OBJ_Z` does
the same for objects. The value dispatched on is a bare tag (`:digger`),
not the first argument (`p`), and the clause is a task.

### 2.3 Pingus and Tux: the objects of a level (one site each)

`game/game.atm:183` in Pingus: `match o` with 22 arms, each
`spawn @pool Kind(g, o)`, in one function. Tux: `match o.tag` in
`view.atm:171` (4 arms) and `session.atm:339` (3 arms), over disjoint
kinds. One `match` each: nothing repeats.

### 2.4 Single sites

SNKRX `match e.elite` (elite.atm:181), `match c.kind` (seeker.atm:90),
`base_stats` (unit.atm:50): one function each.

## 3. A case three ways

Reduced from 2.1; the three run with the compiler at `b2560f7` and print
the same.

As it is:

```
func shoot (u, r) {
    match u.character {
        "outlaw" => "5 knives around " ++ r
        "sage"   => "1 piercing orb at " ++ r
        else     => "1 projectile at " ++ r
    }
}
func cooldown (u) {
    match u.character {
        "vagrant" => 2
        "outlaw"  => 3
        "sage"    => 9
    }
}
```

With dispatch by tag (the syntax of `261118-oo.md` 3.1; it runs today as
`set shoot.ms@(:Char.outlaw) = func ...` and `shoot.call(u, 0)`):

```
func shoot (u :Char, r)           { "1 projectile at " ++ r }      ;; unit.atm

func cooldown (u :Char.vagrant)   { 2 }                            ;; vagrant.atm

func cooldown (u :Char.outlaw)    { 3 }                            ;; outlaw.atm
func shoot    (u :Char.outlaw, r) { "5 knives around " ++ r }

func cooldown (u :Char.sage)      { 9 }                            ;; sage.atm
func shoot    (u :Char.sage, r)   { "1 piercing orb at " ++ r }
```

With a table, as Pingus does (no change to the language):

```
pin CHARS = [
    vagrant = [ cooldown=2 ],
    outlaw  = [ cooldown=3, shoot=func (u, r) { "5 knives around " ++ r } ],
    sage    = [ cooldown=9, shoot=func (u, r) { "1 piercing orb at " ++ r } ],
]
func shoot (u, r) {
    val f = CHARS@(u.character).shoot
    if f => f(u, r) => "1 projectile at " ++ r
}
```

## 4. What the cases say about the suggestion

| Point of 3.1 | In the real cases |
|---|---|
| Clauses of a `func` | both cases that repeat (2.1, 2.2) dispatch **tasks** |
| The tag of the first argument, a tagged table | the kind is a field or a bare tag; no entity is a tagged table |
| The walk up the hierarchy | never needed: the default arm (`else`) is the only "parent" |
| Open (a kind in its own file) | Pingus has it with a table and one line of dispatch |
| The 22 `if u.character == ...` of SNKRX | not helped: tweaks across other functions |
| A clause loses the locals of the function around the `match` | the 52 arms of `Character` share three |

What syntax would save: the registration (10 lines in Pingus), and in
SNKRX a 380-line `match` split in one file per character, after turning
`character` into a tag and the shared locals into parameters.
