-- binary-trees (CLBG); runs on Lua 5.4 and LuaJIT
local function make (d)
    if d > 0 then return { make(d-1), make(d-1) } else return { false, false } end
end
local function check (t)
    if t[1] then return 1 + check(t[1]) + check(t[2]) else return 1 end
end
local N = tonumber(arg and arg[1]) or 10
local mind, maxd = 4, math.max(4+2, N)
do
    local t = make(maxd+1)
    print(string.format("stretch tree of depth %d\t check: %d", maxd+1, check(t)))
end
local ll = make(maxd)
for d = mind, maxd, 2 do
    local it = 2^(maxd - d + mind)
    local c = 0
    for _ = 1, it do c = c + check(make(d)) end
    print(string.format("%d\t trees of depth %d\t check: %d", it, d, c))
end
print(string.format("long lived tree of depth %d\t check: %d", maxd, check(ll)))
