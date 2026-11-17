# envs: an environment written in Atmos

Proposed by the author on 2026-11-17: since Atmos calls C easily, the
interface of an environment could be in Atmos, close to the reference
in Lua (`atmos.env {step, quit, mode}`), instead of a `main` in C.
Draft of the session `envs`, for item 57 and 59. Nothing decided.

## The shape

An environment is a table registered from Atmos; its operations call the
library through native literals:

```
atmos.env [
    mode = [primary=true, secondary=true],
    step = func () { ... },     ;; reads input, emits events; true ends the program
    quit = func () { ... },     ;; frees the library
]
```

One generic `main`, in the runtime, replaces the `main` of each
environment:

```
atm_init; atm_main
loop while alive:
    for each env, in order: if env.step() is true, end
atm_stop
for each env, in reverse: env.quit()
atm_quit
```

- **Primary**: the first registered. Its `step` may sleep (until input,
  or until `atmos.next()`, the time to the next timer) and emits the time.
- **Secondary**: the others. Their `step` never waits and emits no time.
  A second environment that only supports primary is an error, as in Lua.
- **No environment**: the clock, as `rt/main.c` does today (it could be
  itself an environment in Atmos, the `clock` of lua-atmos).
- **Callbacks in C** (IUP, libuv, audio) keep emitting from C, with
  `atm_emit` or `atm_emit_to`, from inside the `step` that pumps the
  library (`IupLoopStep`, `uv_run(NOWAIT)`).
- **Selection** (item 59): `atmos-pico` still includes `pico.atm`, which
  registers itself. Two: `-e pico.atm -e posix.atm`, the order of the
  options being the order of the modes.
- **Test mode** (item 58): a test environment is a primary in Atmos
  that emits scripted input and fixed frames.

Against lua-atmos, one difference stays: the body is the file itself
(the main task), not `loop(f)`.

## Proof that it runs today

With the compiler at `9cdfa4a`, no change to it: a `:prelude` defines
a generic `main` that calls an Atmos `step` with `a_call`, and the
environment is a table in Atmos whose `step` emits a key and 1 ms. The
program sees the keys and `await 2ms` passes by the emitted time. Same
output under `ATMOS_ASAN=1`, no leak.

```
```:prelude
static AVal ENV_STEP;
static int  ENV_N = 0;
int main (int argc, char** argv) {
    atm_init(argc, argv);
    AStatus st = atm_main();
    while (st==A_OK && atm_alive()) {
        int base = a_top;
        a_push(a_num(++ENV_N));
        st = a_call(ENV_STEP, base);
        if (st != A_OK) break;
        int quit = a_top>base && a_truthy(a_stk[base]);
        while (a_top > base) a_clr(&a_stk[--a_top]);
        if (quit) break;
    }
    if (st == A_OK) st = atm_stop();
    if (st != A_OK) a_error_dump();
    atm_quit();
    return st != A_OK;
}
```

func env (t) {
    val s = t.step
    `ENV_STEP = $s; a_inc(ENV_STEP); a_own(ENV_STEP);`
}

;; the environment, in Atmos
env([
    step = func (n) {
        if n > 3 { return true }
        emit(:key.dn [key=n])
        emit(1000)          ;; time, 1ms
        false
    },
])

;; the program
spawn {
    loop {
        val e = await :key.dn
        print("key", e.key)
    }
}
await 2ms
print("2ms")
await :key.dn
print("end")
```

Output: `key 1.0`, `key 2.0`, `2ms`, `key 3.0`, `end`.

## To check with `ceu-atmos`

1. An exception raised by a task during an `emit` inside `step`: does it
   come back as the status of `a_call`, as from `atm_emit`?
2. `atmos.env` as a function of the runtime (it keeps the table with
   `a_own`), and the generic `main` in `rt/main.c` in place of each
   environment's own.
3. The cost of one Atmos call per step (expected negligible; the hot
   paths, as 1000 connections of `env-uv`, keep `atm_emit_to` from C).

## Migration

Each of the five environments moves its `main` from the prelude to a
`step` and a `quit` in Atmos; the C helpers stay as natives. While they
move, a prelude that defines `main` still works (the generic `main` is
used only when none is defined).

## Against the reference (lua-atmos `aa0a99a`, `atmos/run.lua`)

Read in `run.lua`, `api.md`, and the environments `env-socket`,
`env-iup`, `env-js` of `lua-atmos`.

The same, and to keep the same:

- `atmos.env(e)` with `step`, `quit`, `mode = [primary=true, secondary=true]`.
- The runtime writes `mode.current` (`:primary` or `:secondary`) into
  the table; the environment reads it in `step` to decide whether to wait
  and whether to emit time (`env-socket`: `select` with 0 or 0.1 s;
  `env-iup`: `LoopStep` or `LoopStepWait`).
- The first registered must support primary; every other must support
  secondary (an error otherwise).
- `step` returns true to end; `quit` in reverse order, also on an error.
- Time is a number in µs emitted by the primary; `atmos.next()` gives the
  time to the next timer.

What stays different:

| | lua-atmos | Here | Why |
|---|---|---|---|
| Body | `loop(f)`, or `start(f)` | the file is the main task | already the language here; the end of the file is the end of `loop` |
| Registration | `require`, at run time | `include` (or `-e`), and `atmos.env` in the file | same order: the first included is the primary |
| No step (`start`, the browser) | `start(f)` with one environment with no `mode` | to add: one environment with no `step` makes the generic `main` return after `atm_main`; the host emits | needed for the browser (item 52) |
| Library callbacks | Lua functions that `emit` | C that calls `atm_emit` or `atm_emit_to` | the libraries are C; `atm_emit_to` has no equivalent there |
| Sweep of dead tasks | `gc()` after each step | none | reference counting |

## A `main` in the program? (asked by the author, 2026-11-17)

Today the whole file is the main task. The question: an explicit
`main`, a function or a task.

| | The file is the main task (today) | `func main` | `task main` |
|---|---|---|---|
| Awaits | at the top level | no (a function does not suspend the program) | yes |
| Scripts with no events | the file itself | the file itself | the file itself (no `main`) |
| Environments registered before the first reaction | only by order of the code | yes | yes: the top level is the setup, then the runtime spawns `main` |
| Tests (item 58) | the program cannot be run again from a `test` block | — | `test { spawn main(...) ... }` with scripted input, as many times as wanted |
| lua-atmos | — | — | the same as `loop(main)` |

Recommended: an optional `task main (...)`. If the file defines it, the
top level is the setup (includes, `atmos.env`, declarations), and after
it the runtime spawns `main` with the arguments; the program ends when
`main` ends, and its value is the exit status. If the file does not
define it, the file is the main task, as today, so scripts and existing
programs do not change. `func main` gives nothing that the top level
does not give already.

### The trick of the reference (atmos-lang/atmos, `atmos` and `src/exec.lua`)

The command `atmos` wraps the whole file in a function and passes it to
`loop`: `atm_loadfile` builds `(func (...) { <file> \n })(...)` (a
function, so that `return` and `throw` work at the top and `...` are the
arguments; the `\n` keeps a `;;` comment in the last line from eating
the `}`), and the last line of `atmos` is
`atmos.loop(atm_loadfile(input), args...)`. The program `require`s its
environment in its own text (`require "atmos.env.clock"`), so the
environment is registered during the first run of the main task, before
its first `await`, which is before `loop` calls the first `step`. With
no environment, `loop` spins: `hello.atm` requires the clock by hand.

Here the compiler already makes the file the main task, and the
generic `main` calls the first `step` only after `atm_main` returns,
with the default clock when none is registered. So the trick is built
in, and a `task main` is not needed for it. Its remaining value is
tests (item 58): leave it for that discussion.

### No `main` needed for tests either (the author, 2026-11-17)

The game is its own task, and a `test` block runs it. Checked with the
compiler at `c4b7411`, no change:

```
task Game () {
    var n = 0
    loop {
        val e = await :key.dn
        set n = n + 1
        print("key", e.key, n)
        until e.key == 'Q'
    }
    n
}

test {
    pin g = spawn Game()
    emit(:key.dn [key='A'])
    emit(:key.dn [key='Q'])
    assert(g.ret == 2)
    print("test ok")
    return 0
}

print("real run")
val n = await Game()
print("score", n)
```

`atmos -t g.atm` prints `key A 1`, `key Q 2`, `test ok`, exit 0;
`atmos g.atm` skips the test and runs the game. The `return` in the
`test` ends the main chunk before the environment ever steps: the test
block itself is the source of input and time (`emit(:key.dn ...)`,
`emit(16000)`). So item 58 needs no `task main` and no test mode with
variables. What is missing is only on the side of the environment: under
`-t` it should not open the real window, sound, or device at its
`include`. The reference has `atmos.test`; here there is no such flag
(the compiler only drops `test` blocks). To ask `ceu-atmos`: a flag
under `-t`, readable by the environment from Atmos and from C.
