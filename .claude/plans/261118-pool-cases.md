# pool: the cases of item 1 in the ports

Item 1: an `await` over a pool with a condition ("a pingu enters my
zone"). Gathered on 2026-11-18 from the repositories of the five ports
(read-only clones, at sdl-snxkx `4445236`, sdl-flare `5a12403`,
sdl-pingus `13a4096`, ray-lessmilk `41fd87a`, sdl-tux `49f46dc`).

## 1. What each port wrote instead

The classes:

- **(a) await**: one task waits until some member satisfies a condition,
  and then acts once.
- **(b) query per tick**: every tick, act on every member that satisfies
  a condition (an await that wakes once does not cover it).
- **(c) pairs**: collision passes, N x M.
- **(d) other**: lookups by id, liveness of one member, sweeps at an
  event.

| Port | Collection | (a) | (b) | (c) | (d) |
|---|---|---|---|---|---|
| Pingus | `pin pingus = tasks()`, reads `t.pub` | 6 | 4 | 0 | 7 |
| SNKRX | plain tables of records (`A.enemies`, `A.units`); the tasks live in other pools | 33 | ~10 | 21 + 5 | 6 |
| Flare | pool `es`, but walked through a parallel table `ens` | 0 (1 by `:all`) | 6 | 1 | 3 |
| Lessmilk | plain tables and a quadtree of bodies | 2 | 0 | 7 | 2 |
| SuperTux | `pin objs = tasks()`; collisions in C, sent as `emit @t :hit` | 0 | 0 | 0 (1 in C) | 0 |

### Pingus: the origin, the only port over a pool with `pub`

```
;; game/objs/zone.atm
func zone_any (g, pos, x1, y1, x2, y2) {
    loop _, t in g.pingus {
        until in_zone(pos, t.pub, x1, y1, x2, y2), true
    } || false
}
;; game/objs/spike.atm:18
await :tick until zone_any(g, pos, 16-5, 0, 16+5, 32)
```

The same shape in the smasher (`smasher.atm:25`), the fake exit
(`extra.atm:158`), the switch of a door (`extra.atm:332`). The
guillotine and the laser exit (`guillotine.atm:7`, `laser_exit.atm:6`)
scan inside `loop on :tick` and need the pingu that matched. After the
wake, 5 of the 6 act on the pingus in the zone (the one that matched,
or all of them): the result should be the task, not `true`.

Class (b): the exit (`exit.atm:16`, every pingu in it, every tick, a
fix recorded in `doc/todo.md:33`), the conveyor belt, the teleporter,
the blocker (a pingu against all pingus). Cost today: W objects x P
pingus per tick.

### SNKRX: the most numerous, but over tables

```
;; src/attack.atm:116
task Cooldown (u, d, ready) {
    var timer = 0
    loop us on :clock {
        set timer += us / 1000000
        set u.cooldown = math.min(timer / (d*u.aspd_m), 1)
        until (timer > d*u.aspd_m) && ready()
    }
    set u.cooldown = 0
}
;; src/characters.atm:331
pin near = \(rs) { #enemies_in_circle(A, u.x, u.y, rs) > 0 }
"vagrant" => loop {
    await Cooldown(u, 2, \{ near(96) })
    val e = closest_enemy(A, u.x, u.y, 96)
```

30 characters (and one summon) wait for "an enemy within a radius", then
search again for the one to attack. The level waits for "no enemy left"
(`level.atm:44`, `until #A.enemies == 0`), the table counterpart of
`await :all`. Enemies are records in a table, because a table cannot
keep the tasks of a pool (`doc/compiler.md`, section 2): a feature over
pools only applies here if the port moves them to pools.

### Flare: nothing of class (a)

The one pool-wide condition is "the map is cleared" (`play.atm:340`:
every enemy has `hp <= 0`, corpses stay in the pool), an `await :all`
by a condition. The waits that look like (a) are on one entity (the
hero within a distance, a summoner dead). Hazards against enemies are a
pass that keeps the order of the original.

### Lessmilk and SuperTux: collisions

Lessmilk: `await :overlap until overlap_any(body, :enemy)` (2 games),
over the quadtree of the physics, not over `pub`; the rest are passes
of overlaps with work for each pair. SuperTux already turns a collision
into an event to the one object (`emit @t :hit [other, bits]`), and
each object awaits `:hit until it@0 == PLAYER`.

## 2. What follows from the cases

1. **The form already exists.** `await :tick until f()` re-evaluates
   `f` at each event, and evaluates to its result (the table of
   "Await", row "Condition"). Pingus writes exactly that. What is
   missing is a function over a pool that gives the first task that
   satisfies a predicate (or `nil`), and so the task as the result.
2. **Checking only on changes saves nothing here.** In every case the
   members move every tick (pingus, enemies), so a runtime that
   re-checks a waiter only when a `pub` changes would re-check as often
   as the scan does, and pay a barrier on every write of a table that
   is a `pub` (`set t.pub.x = ...` is a write to a table, not to the
   task).
3. **The real cost is W x P**, and only a spatial structure (a grid, a
   quadtree, as Lessmilk has) removes it. That is a library, not the
   language.
4. **The pool is not where the ports keep their entities** (SNKRX,
   Flare, Lessmilk), because of the rules of `pin` (a table cannot keep
   tasks of a pool, a pool cannot be stored in a module). That friction
   is item 40, in the compiler session.

## 3. The options for item 1

| | Option | In the language | Cost in the runtime |
|---|---|---|---|
| A | A function of the library over a pool: `find(ts, f)` gives the first live task whose `f(t)` is truthy, or `nil`. Used as `val t = await :tick until find(ts, \(t) { in_zone(pos, t.pub) })`. Documented in "Await" as the idiom | No new syntax | None: a scan of the pool per event that reaches the waiter, the same as the ports write by hand |
| B | A new pattern, `await :some ts until f` (a member of the pool satisfies `f`, re-checked at every event that reaches the waiter), giving the task and the pool, as `:any` does | A new leaf of pattern; `:any ts until c` already means "a task ended, and `c`", so it needs another word | Small: the scan moves into `leaf_match` in C (saves the calls of the loop in Atmos, not the W x P) |
| C | Re-check on change: the runtime marks a task whose `pub` was written, and wakes the waiters of the pool to test only it | A new pattern, as B | Large: a barrier on every write to a table that is a `pub` (a flag in the table and a list per pool), and the members change every tick in all cases, so nothing is saved |
| D | Members signal: each member emits to the pool of zones (`emit @zones :moved [t]`), and each zone awaits `:moved until in_zone(it)` | Needs `emit` to a pool (not in "Emit" today) | Same W x P, as events; it inverts who iterates. SuperTux does it from C |

Without the library, A is already written today with a loop that
gives a value: `await :tick until loop _, t in ts { until f(t), t }`
(Pingus writes `until ..., true` and loses the task).

Recommended: **A**, now, with the idiom in the manual, and a `filter`
companion for the (b) cases that act on all that match. B only if a
port shows the calls of the loop in its profile. C and D do not reduce
the cost that the cases have.
