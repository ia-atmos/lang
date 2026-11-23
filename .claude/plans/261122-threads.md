# atmos-threads: running work outside the reactive loop

Material for the session `atmos-threads`, where the author discusses
threads in Atmos. Gathered on 2026-11-22 by the session that maintains
the compiler (`atmos-lang`). It has the request (section 1), what exists
today (2), where the ports and environments met the need (3), what a
thread meets in the runtime (4), and questions (5). Nothing in section 5
is decided.

## 1. The request

**Autor** (2026-11-22, in `atmos-lang`):
> Agora vamos falar sobre threads? Nao existe suporte hoje, certo? Abra
> uma sessao atmos-threads se for o caso. Discutiremos la

## 2. What exists today

On `main` (2026-11-22, `04fe436` and after).

- **No threads.** Tasks are cooperative: they "execute uninterruptedly
  up to explicit `await`" (manual, line 137). The program is one OS
  thread; an emit runs every task it awakes to completion before the
  next event.
- **`thread` is a reserved word** (`src/lexer.c`, `KEYS`) and the
  parser treats it as a prefix that takes no suffix
  (`src/parser.c`, `parser_2_suf`: `check_str("thread")`, beside
  `emit`, `spawn`, `mute`), but there is no grammar for it: a program
  cannot write `thread { ... }`. It is a placeholder kept from the
  reference.
- **The reference** (`atmos-lang/atmos`, in Lua) has `thread { ... }`.
  Its four tests were dropped when the golden tests were extracted
  (`tst/dropped/0516` to `0519`, from `thread.lua`, marked "native with
  Lua code" because they sleep with `os.execute`):

  | Test | What it shows |
  |---|---|
  | `0516-thread-exec-00` | `thread { await :OK }` is an error: "invalid await : expected enclosing task" (a thread cannot await) |
  | `0517-thread-exec-01` | `val v = thread { 42 }` inside a task gives 42 |
  | `0518-thread-exec-02` | a thread with a heavy loop returns its sum (5050) |
  | `0519-thread-exec-03` | two threads in a `par :any` under `watching 20s`: the quick one wins, the other is aborted |

  So in the reference a `thread` is an expression inside a task: the
  task awaits it, the block runs elsewhere and cannot await, and its
  value comes back as the value of the expression; an abort of the task
  abandons the thread.
- **Céu** (the predecessor) had `await async/thread do ... end`, with
  the same shape.

## 3. Where the ports and environments met the need

- `env-uv` (libuv): "the threads of libuv are not done" (README); its
  `doc/ceu.md` lists `async-01` and `threads-01` of `ceu-libuv` as not
  ported, and `timer-02` lost the sleep in a thread between the timers.
  The file operations of libuv already run in libuv's own thread pool
  and come back as events (`uv.atm`).
- `env-esp32`: the board runs FreeRTOS; its C drivers use FreeRTOS
  tasks and workers (`drv/esp32/ir.c`, `gpio.c`, `espnow.c`) that hand
  events to the single Atmos loop.
- The game ports (Pingus, SNKRX, Doom, Flare, Tux) run one frame loop
  and asked for no thread; Pingus loads its data before playing.
- `atmos-cli` (`261120-cli.md`): compile time, not run time, is what
  takes long today.

## 4. What a thread meets in the runtime

- **Reference counts are not atomic** (`rt/atmos.h`: `rc++`, `--rc` in
  `a_inc`/`a_dec`), so two threads may not share a value with no copy
  or a lock.
- **Global state**: the library tables (`atm_g_*` in `rt/std.c`,
  `rt/l*.c`), the stack of the current task, the trace of calls, the
  registry of `set :out`, the scheduler. A second thread running Atmos
  code needs its own, or a lock around all of it.
- **`pin` and the blocks**: a value belongs to a block of the program;
  a thread that outlives the task that started it (the abort of `0519`)
  holds values of a block that ended.
- **Determinism**: the order of awaking is part of the semantics; the
  end of a thread must arrive as an event at a defined point (as a
  timer or an event of an environment does), not interrupt a task.
- **Boards**: the ESP32 has two cores and FreeRTOS; an RP2040 has two
  cores; a microcontroller with one core has no gain.

## 5. Questions

- **What for.** Heavy computation off the frame loop (the `0518`/`0519`
  case); blocking calls of C libraries (files, network, a decoder);
  using several cores; or only the reference's construct for parity.
- **The form.** `val v = thread { ... }` inside a task, as the
  reference; a library function (`atmos.thread(f, args)`) giving a
  task to await; or only an API of environments (a C call that runs in
  a pool and emits its result), with no construct in the language.
- **What the block may touch.** Nothing shared (arguments copied in,
  the result copied out, as messages); read-only access to values of
  the task; or shared values with locks. Whether it may call C,
  `print`, the library.
- **Await and emit inside.** An error, as in the reference (`0516`),
  or allowed in a loop of its own.
- **Abort.** What happens to a thread whose task is aborted: abandoned
  and its result dropped (the reference), cancelled cooperatively, or
  the task waits for it.
- **The runtime.** One runtime per thread (an Atmos state each, values
  copied between them), or one runtime with a lock; atomic counts or
  not.
- **Order.** When the result of a thread arrives: as an event emitted
  by the loop between two events (deterministic in the order of the
  loop, not in time), like the end of a timer.
- **Builds and environments.** An option of the build
  (`ATMOS_THREADS=...`) so that a board with one core pays nothing;
  how `env-uv` (libuv's pool), `env-esp32` (FreeRTOS) and the SDL
  environments fit.
- **The keyword.** Keep `thread` reserved, give it a meaning, or free
  it.
