# Development

Maintained by the sessions; moved out of `README.md` on 2026-11-28 (the
author, atmos-docs#12). The README of the author keeps no section on it.

The plan, with the settled decisions and the milestones, is in
[`.claude/plans/261109-c-backend.md`](plans/261109-c-backend.md).

## Test

```
make test
```

Before a merge that touches `rt/` or the `Makefile`, `make test-cfgs`
builds the runtime in every configuration of the options, from clean
and with `-Werror`, in `build/cfgs`: a warning in any of them would
reach the output of a program built with it. `atmos-lang` runs it.

- `tst/rt/`: unit tests of the runtime, in C.
- `tst/lex/`, `tst/ast/`: lexer and parser golden cases.
- `tst/golden/`: the cases of the reference implementation that this
  compiler reproduces, extracted by `tst/extract.py`.
    - Some are adapted to this language (base 0, operators that
      became functions), as `tst/adapt.py` tells. `ADAPTED.md` lists
      them.
    - Those left out are in `tst/dropped/`, with the reasons in
      `INDEX.md`.
- `tst/cases/`: golden programs, built with the address sanitizer
  (leaks included).

## Layout

```
src/    compiler: lexer, parser, resolver, coder
rt/     runtime: values, strings, tables, operations, tasks, stdlib
rt/atmos.h   C API for generated code, natives, and environments
tst/    tests
doc/    guide, manual
exs/    examples
```

## Differences

From the [original implementation][atmos-lua], which compiles to Lua:

- Indexes start at `0`: vectors, numeric loops, and strings.
- An unknown identifier is a compile error: globals are static.
- `await` is only valid lexically inside a task body.
- `catch` only catches `throw`.
- `break`, `until`, and `while` need a lexically enclosing `loop`.
- `skip` is new: it goes to the next step of the enclosing `loop`.
- A `spawn` with no pool and no assignment that is the last statement
  of its block is a
  compile error, if the task may await: it would be aborted with the
  block, at once. The ways out are a pin outside the block, as in
  `pin t = if c { spawn T() }`, or a pool, as in `spawn @ts T()`.
- Numbers are 64-bit integers or floats, as in Lua 5.3: `7 / 2` is
  `3.5`, `7 // 2` is `3`, and a float always shows as one (`5.0`).
- Operators follow C:
    - precedence: `a + b * c` needs no parentheses;
    - `%` has the sign of the dividend, and `//` truncates;
    - bitwise `&` `|` `^` `~` `<<` `>>` on integers, binding stronger
      than comparisons;
    - compound assignment: `set x += 1`.
- `**`, `?>`, `!>`, `=>=`, `=<=`, `===`, and `=!=` are functions:
  `math.pow`, `X.xin`, `X.nin`, `X.gte`, `X.lte`, `X.eq`, and `X.neq`.
  The strict `X.gt` and `X.lt` are new.
- Statements take their arguments with no parentheses: `return x + 1`,
  `break`, `until i == 10`, `throw :X, "msg"`, `abort t`, `emit e`.
    - Lists need no parentheses either: `return 1, 2`,
      `val a, b = 1, 2`, `set x, y = y, x`.
    - `(a, b)` is still a list anywhere, and `(f())` has a single
      value, as in Lua.
    - The predicates of await patterns too: `await :X until it.n > 3`.
      Inside `<...>`, the operators `>` `>>` `&&` `||` need
      parentheses, or it is an ambiguity error.
- A declaration, an assignment, or a `defer` is only valid as a
  statement of a block. Using one as a value, as in
  `print(val x = 1)`, is a compile error.
- `include "file.atm"` joins files at compile time: the statements of
  the file take the place of the include. There is no `require`.
- `X.load(s)` reads data written with the constructors of the
  language, as a JSON of Atmos: tables, vectors, tags, numbers,
  durations, strings, and comments. No expressions. `X.dump(v)` writes
  it back.
  `X.loadx(s)` also accepts names defined before the value
  (`T = [1]  [T, T]`), each use being a copy, and `include "file"`
  for a file of definitions.
- Native literals are C code, not Lua: `` `:number strlen("abc")` ``.
    - A leading tag converts the expression: `:number`, `:boolean`,
      `:string`, `:atmos`. With no tag, the code is a statement.
    - `` `:prelude.c ...` `` declares at file scope, as in `#include`
      (`:prelude.h` is also seen by the files that `import` it).
    - `$x` refers to the variable `x`.
- The libraries are `string`, `vector`, `math`, `io`, and `os`, with
  0-based positions, instead of the libraries of Lua.
- Every dynamic value belongs to a block: tasks, pools, tables,
  functions, and natives.
    - A `pin` (`pin val` or `pin var`) owns a value that was just
      created, and so does a parameter `pin x`. A `val` or a `var`
      refers to a value that belongs to a block, or holds an unpinned
      one, which only a block at the name or outside may adopt.
    - A pinned value is freed when its block ends, cycles included,
      along with the unnamed unpinned values that were stored in it.
      Without a pin, reference counting applies, and an unpinned cycle
      leaks: a cyclic structure must be pinned.
    - It cannot escape its block: storing it in an outer variable, in
      a table of an outer block, in a `throw`, emitting it outwards, or
      returning it raises `invalid escape : table pinned at prog.atm:10`.
      A value that a block gives away is a `val`.
    - An unpinned task that is dropped is aborted at once.

[atmos-lua]: https://github.com/atmos-lang/atmos/
