#!/usr/bin/env python3
# gen.py <dir> <n> : writes one program per operation, in Atmos (the
# loop inside a function, `OP-f.atm`, and in the main body, `OP-t.atm`)
# and in Lua (`OP.lua`), each one a loop of <n> iterations around the
# operation. `loop` is the empty loop, to subtract.
import sys, os
d, n = sys.argv[1], int(sys.argv[2])
os.makedirs(d, exist_ok=True)
OPS = {
    # name: (Atmos setup, Atmos body, Lua setup, Lua body)
    'loop':   ('', '', '', ''),
    'add':    ('var x = 0.5', 'set x = x + 1.5',
               'local x = 0.5', 'x = x + 1.5'),
    'mul3':   ('var x = 0.5\nvar y = 1.5', 'set x = x*y + y*y - x',
               'local x = 0.5\nlocal y = 1.5', 'x = x*y + y*y - x'),
    'cmp':    ('var x = 0.5\nvar c = 0', 'if x < 2.0 { set c = c + 1 }',
               'local x = 0.5\nlocal c = 0', 'if x < 2.0 then c = c + 1 end'),
    'field':  ('val t = [x=1.5, y=2.5, z=3.5]\nvar s = 0.0', 'set s = s + t.y',
               'local t = {x=1.5, y=2.5, z=3.5}\nlocal s = 0.0', 's = s + t.y'),
    'setf':   ('val t = [x=1.5, y=2.5, z=3.5]', 'set t.y = 1.5',
               'local t = {x=1.5, y=2.5, z=3.5}', 't.y = 1.5'),
    'vget':   ('val v = [1.5, 2.5, 3.5, 4.5]\nvar s = 0.0', 'set s = s + v@2',
               'local v = {1.5, 2.5, 3.5, 4.5}\nlocal s = 0.0', 's = s + v[3]'),
    'vset':   ('val v = [1.5, 2.5, 3.5, 4.5]', 'set v@2 = 1.5',
               'local v = {1.5, 2.5, 3.5, 4.5}', 'v[3] = 1.5'),
    'call':   ('func f (a) { a }\nvar s = 0.0', 'set s = f(1.5)',
               'local function f (a) return a end\nlocal s = 0.0', 's = f(1.5)'),
    'new':    ('var t = nil', 'set t = [x=1.5, y=2.5]',
               'local t = nil', 't = {x=1.5, y=2.5}'),
    'global': ('var s = 0.0', 'set s = s + G',
               'local s = 0.0', 's = s + G'),
}
for k, (ia, ba, il, bl) in OPS.items():
    open(f'{d}/{k}-f.atm', 'w').write(
        f'val G = 2.5\nfunc main () {{\n{ia}\nloop i in {n} {{\n{ba}\n}}\n}}\nmain()\n')
    open(f'{d}/{k}-t.atm', 'w').write(
        f'val G = 2.5\n{ia}\nloop i in {n} {{\n{ba}\n}}\n')
    open(f'{d}/{k}.lua', 'w').write(
        f'local G = 2.5\nlocal function main ()\n{il}\nfor i=1,{n} do\n{bl}\nend\nend\nmain()\n')
print(' '.join(OPS))
