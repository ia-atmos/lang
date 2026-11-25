-- spectral-norm (CLBG), 0-based as the C version
local function A (i, j) local ij = i + j; return 1.0 / (ij*(ij+1)/2 + i + 1) end
local function Av (n, v, o) for i = 0, n-1 do local s = 0.0; for j = 0, n-1 do s = s + A(i,j)*v[j] end; o[i] = s end end
local function Atv (n, v, o) for i = 0, n-1 do local s = 0.0; for j = 0, n-1 do s = s + A(j,i)*v[j] end; o[i] = s end end
local function AtAv (n, v, o, t) Av(n, v, t); Atv(n, t, o) end
local n = tonumber(arg and arg[1]) or 100
local u, v, t = {}, {}, {}
for i = 0, n-1 do u[i] = 1.0 end
for _ = 1, 10 do AtAv(n, u, v, t); AtAv(n, v, u, t) end
local vBv, vv = 0.0, 0.0
for i = 0, n-1 do vBv = vBv + u[i]*v[i]; vv = vv + v[i]*v[i] end
print(string.format("%.9f", math.sqrt(vBv/vv)))
