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
