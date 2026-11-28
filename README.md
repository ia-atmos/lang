# The Programming Language Atmos

***Structured Event-Driven Concurrency***

[
    [About](#about)                 |
    [Hello World!](#hello-world)    |
    [Install & Run](#install--run)  |
    [Documentation](#documentation) |
    [Environments](#environments)   |
    [Resources](#resources)
]

<img src="atmos-logo.png" width="250" align="right">

# About

Atmos is a programming language that reconciles *[Structured Concurrency][sc]*
and *[Event-Driven Programming][events]*, extending classical structured
programming with two main functionalities:

- Structured Deterministic Concurrency:
    - A `task` primitive with deterministic scheduling provides predictable
      behavior and safe abortion.
    - A `tasks` container primitive holds attached tasks and controls their
      lifecycle.
    - A `pin` declaration attaches a task or tasks to its enclosing lexical
      scope.
    - Structured primitives (e.g., `par`) compose concurrent tasks with lexical
      scope.
- Event Signaling Mechanisms:
    - An `await` primitive suspends a task and waits for events.
    - An `emit` primitive signals events and awakes awaiting tasks.

Atmos is inspired by [synchronous programming languages][sync] like [Céu][ceu]
and [Esterel][esterel].

This repository is the implementation of Atmos that compiles to C:

- No virtual machine, no interpreter, and no tracing garbage collector.
- The whole program becomes a single C file, which a C compiler builds.
- Tasks have no stacks of their own: a task is a record with its variables
  and the point where it resumes.
- Memory is reclaimed by reference counting, and by the lexical scope of
  `pin` declarations.

The original implementation of Atmos, which compiles to [Lua][lua], is at
[atmos-lang/atmos][atmos-lua].
The two differ in some points, which are [listed below](#differences).
The functional streams and the threads of the original are not part of this
implementation.

**Experimental: work in progress.**

[sc]:           https://en.wikipedia.org/wiki/Structured_concurrency
[events]:       https://en.wikipedia.org/wiki/Event-driven_programming

[sync]:         https://fsantanna.github.io/sc.html
[ceu]:          http://www.ceu-lang.org/
[esterel]:      https://en.wikipedia.org/wiki/Esterel
[lua]:          https://www.lua.org/
[atmos-lua]:    https://github.com/atmos-lang/atmos/

# Hello World!

During 5 seconds, displays `Hello World!` every second:

```
watching 5s {
    loop on 1s {
        print "Hello World!"
    }
}
```

The program body is a task in Atmos that behaves as follows:

- The `watching` command will execute its block during 5 seconds.
- The `loop on` will execute its block every second.
- Once the `watching` terminates, the body reaches its end, and the program
  exits cleanly.

Time is part of the language: with no environment, a program follows the real
clock.

# Install & Run

Atmos requires a C compiler and `make`:

```
git clone https://github.com/ia-atmos/lang
cd lang/
make
./atmos exs/hello.atm
```

The `make` builds the compiler `./atmos` and the runtime libraries in
`build/`.

The compiler translates a program to C, builds it, and runs it:

```
./atmos hello.atm [args]    # compiles, builds, and runs
./atmos -c hello.atm        # emits C only, to the standard output
./atmos -o hello hello.atm  # builds only
./atmos -t hello.atm        # enables test blocks
./atmos -e env.atm game.atm # includes an environment first
```

Environment variables:

- `ATMOS_CC`: C compiler (default `cc`).
- `ATMOS_CFLAGS`: C flags (default `-O1`).
- `ATMOS_LDFLAGS`: linker flags, which go after the program
  (e.g. `-lSDL2`).
- `ATMOS_ASAN`: if set, builds the program with the address sanitizer.
- `ATMOS_FLOAT`: if `32`, a float is a C `float` with 32 bits.

# Documentation

- [Guide](doc/guide.md): a tour in short steps, each a program that runs
- [Manual](doc/manual.md)
- [Examples](exs/)

# Environments

An environment is an external component that bridges input events from the real
world into an Atmos application.

The default environment only deals with time: it follows the real clock, and
terminates the program when there is nothing left to awake.

Other environments are written in C, in the Atmos program itself or in a file
that it includes:

- [`sdl.atm`](https://github.com/ia-atmos/env-sdl):
    An environment that relies on [SDL][sdl], with a window, keys, mouse,
    images, text, sounds, and timers.
- [`ray.atm`](https://github.com/ia-atmos/env-ray):
    An environment that relies on [raylib][raylib], with keys, mouse,
    textures, sounds, and fonts.
- [`pico.atm`](https://github.com/ia-atmos/env-pico):
    An environment that relies on [pico-sdl][pico], with layers, anchors,
    and positions relative to the window.

Each has a test mode with no screen, with a fixed number of frames and
scripted input.

An environment may have a compiler of its own, such as `atmos-pico`, so that a
program needs no `include`: `make env` builds one (see the
[manual](doc/manual.md#environments)).

[sdl]:          https://www.libsdl.org/
[raylib]:       https://www.raylib.com/
[pico]:         https://github.com/fsantanna/pico-sdl

# Resources

- [A toy problem][toy]: Drag, Click, or Cancel
- A simple but complete 2D game in Atmos:
    - [`exs/rocks/`](https://github.com/ia-atmos/env-sdl/tree/main/exs/rocks)
      of `env-sdl`
- Academic publications (Céu):
    - http://ceu-lang.org/chico/#ceu
- Mailing list (Céu & Atmos):
    - https://groups.google.com/g/ceu-lang

[toy]: https://fsantanna.github.io/toy.html
