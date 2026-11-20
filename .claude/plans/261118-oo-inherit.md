# atmos-oo: inheritance of behavior in the ports

Analysis of 2026-11-20 by the session `atmos-oo` (item #4), under the
rule "Work with no input". It reads the first port with a real class
hierarchy, the badguys of SuperTux, and rereads Flare and Pingus, to
check whether decision #1 ("a table for now", no syntax for dispatch by
tag) stands. Read: sdl-tux `dcd65d8` (`game/badguy.atm`, 973 lines),
sdl-flare and sdl-pingus at their heads of the day.

**Result**: #1 stands. The hierarchy of SuperTux is carried today by
plain tables and closures, with override and call to the parent
(section 3, form B), and the prototypes run. But the first survey was
wrong in one point: a hierarchy of three levels does exist in a port,
and the port wrote it as a closed chain of tests, the form that
dispatch was meant to avoid.

## 1. SuperTux: what the original has and what the port wrote

The C++:

```
BadGuy
├── WalkingBadguy
│   ├── SnowBall, SmartBall, Spiky, SSpiky      (data, and SSpiky sleeps)
│   └── MrIceBlock ── SmartBlock
├── Jumpy
├── BouncingSnowball
├── FlyingSnowBall
└── Stalactite
```

The port, `game/badguy.atm`:

| What | How |
|---|---|
| The 10 classes | rows of a table `BADGUYS`, by the tag of the level object: `kind`, sprite, `walk_speed`, `max_drop`, `squish`, `sleeps` |
| Classes that differ by data only (SnowBall, SmartBall, Spiky; SmartBlock) | rows with other values: no code |
| The 6 kinds with code of their own | one task `BadGuy (life, objs, o, cls, uid)`, 900 lines, 38 local functions |
| A virtual method | a local function that tests `cls.kind` and calls `walk_active_update`, `jumpy_active_update`, `ice_active_update`, ... |
| A call to the parent | a direct call (`base_active_update()` in four kinds; `walk_active_update()` from the ice block while `:normal`), or a fall through (`:normal => nil ;; as a walking badguy, below`) |
| The fields of the subclasses | all in one table `b`, for every badguy (`st`, `shake_timer` of the stalactite; `ice`, `flat_timer` of the ice block; ...) |

Tests of `cls.kind`: 32, in 10 functions.

| Function | Tests |
|---|---|
| `collision` | 10 |
| `collision_solid` | 5 |
| `active_update` | 5 |
| the start of the task (the constructor) | 4 |
| `initialize` | 3 |
| `update`, `turn_around`, `try_activate`, `set_dir`, `kill_fall` | 1 each |

So: 5 methods that vary by kind, 6 kinds, 3 levels. A new kind edits
these 10 functions and the table `b`. It is the closed `match` of
`261118-oo.md`, in its largest instance in the ports.

Why one task: the 38 functions are closures over the locals of the task
(`id`, `l`, `b`, `pub`, `cls`), and call each other freely (`set_action`,
`set_state`, `kill_fall`, `turn_around`). To split by class, these have
to reach the other file some other way.

## 2. Flare and Pingus, reread

| Port | Hierarchy of the original | In the port |
|---|---|---|
| Flare | `Entity` with `Avatar` and `Enemy` (two leaves) | two tasks, `Hero` and `Enemy`; the shared code (`take_hit`, `power_activate`) takes the `pub` of either and tests a flag, `t.hero` (11 tests in `combat.atm`). The states (`Stance`, `Move`, `HPower`; `EStance`, `EPower`, `EHit`) are a task each. No kinds of enemy in code: a stat block from a file |
| Pingus, actions | `PinguAction`, 22 leaves, one level | a task each, in a table by tag |
| Pingus, objects | `WorldObj`, about 25 leaves, one level | a task each (`Guillotine`, `Smasher`, `Spike`, ...), chosen by one `match`; what they share is plain functions (`in_zone`, `zone_any`) |

One level deep in both: nothing overridden, no call to a parent. A
leaf is a task, and the base is either the task around it (Pingus) or
functions that take the value (Flare). The table idiom is enough.

## 3. The hierarchy of SuperTux three ways

Reduced to three classes over two methods (`update`, `squished`), with
a parent call at each level. The three programs run with the compiler
at main `c4e99c4` under the address sanitizer and print the same 19
lines.

### A. As the port: the class tested inside each method

```
val func update () {
    if cls.kind == :iceblock {
        if b.ice == :normal { walk_update() } else { base_update() }
        return nil
    }
    if cls.kind == :jumpy {
        base_update()
        print(cls.name, "jump")
        return nil
    }
    walk_update()
}
```

### B. A record of methods per badguy, built by its classes in order

A class is a function that fills the record `m`. It calls its parent
first, replaces a field to override, and keeps the old value to call
the parent. No dispatch: `m.update()`.

```
func Base (m, cls, b) {
    set m.update   = func () { print(cls.name, "move") }
    set m.squished = func () {
        if cls.squish { print(cls.name, "dies") } else { print(cls.name, "hurts") }
    }
}
func Walker (m, cls, b) {
    Base(m, cls, b)
    val super = m.update
    set m.update = func () { super() ; print(cls.name, "walk", cls.speed) }
}
func IceBlock (m, cls, b) {
    Walker(m, cls, b)
    val walk = m.update
    set b.ice = :normal                 ;; its own field
    set m.update = func () {
        if b.ice == :normal { walk() } else { print(cls.name, "move") }
    }
    set m.squished = func () { set b.ice = :flat ; print(cls.name, "flat") }
}
pin BADGUYS = [
    snowball   = [class=Walker,   name="snowball",   speed=80, squish=true],
    mriceblock = [class=IceBlock, name="mriceblock", speed=80, squish=false],
]
...
pin m = []
cls.class(m, cls, b)
m.update()
```

### C. A table of methods per class, by a hierarchical tag

A function `send` walks up the tag (`:Bad.Walker.Ice`, `:Bad.Walker`,
`:Bad`); the parent is called by its tag; the methods take the value.
It is the library of `261118-oo.md` 3.2, and what the syntax of 3.1
would be sugar for.

```
set M@(:Bad.Walker) = [
    update = func (s) { send(:Bad, "update", s) ; print(s.name, "walk", s.speed) },
]
set M@(:Bad.Walker.Ice) = [
    update = func (s) {
        if s.ice == :normal { send(:Bad.Walker, "update", s) } else { send(:Bad, "update", s) }
    },
]
...
send(s.tag, "update", s)
```

### Compared

| | A (the port) | B (records of closures) | C (table by tag) |
|---|---|---|---|
| A new kind | edits 10 functions | a new function, a row in `BADGUYS` | a new entry of `M` |
| A kind in its own file | no | yes | yes |
| Override | a test in the method | replace the field | an entry in the child |
| Call to the parent | a direct call by name | the saved old field | `send` with the parent tag |
| The locals of the task (`id`, `l`, `b`) | closures, free | closures, passed once to the class | a `self` in every method and helper |
| Fields of a subclass | all in one table | added by the class | added at construction |
| Cost | none | a record of closures per badguy | a walk per call (cacheable) |
| Language needed | today | today | today, with 15 lines of library |

## 4. Where the table idiom of the manual strains

The paragraph proposed for the manual (#3) is `AREA@(s.tag)(s)`: one
table, one level. Against SuperTux:

- **No parent.** A kind with no entry gives `nil`, not the entry of its
  parent. It needs a default (`AREA@(k) || base`) or the walk of form C.
- **Override with a call to the base** has no place in a flat table:
  it needs the parent by name (C) or the saved field (B).
- **Methods that share the locals of a task** do not fit functions
  stored in a global table, unless they take a `self`. Form B keeps
  the closures, and fits the way the ports already write tasks.
- **Data-only subclasses fit perfectly**: 5 of the 10 classes are rows.

What syntax (3.1) would add over B: nothing that B cannot say; over C:
the walk and the `self` written by the compiler. Neither is needed to
port the hierarchy, and B asks for no dispatch at all. What B costs is
a record of closures per object, small for badguys.

## 5. Conclusions

- #1 stands: no syntax. The deepest hierarchy of the ports is carried by
  form B in the language of today.
- Correction to `261118-oo-cases.md` (section 4, "the walk up the
  hierarchy: never needed"): true of the five ports of then; SuperTux
  has three levels and needs either the saved parent (B) or a walk (C).
- If syntax is ever reopened, SuperTux is the case to measure it on, and
  the first question is still tasks and closures: the methods of a
  badguy are closures over a task, not functions of a tagged table.
- A suggestion for `sdl-tux`, which is its to take or not: `badguy.atm`
  split by class in form B (a file per kind, `BADGUYS` with
  `class=...`), which removes the 32 tests of `cls.kind`.
