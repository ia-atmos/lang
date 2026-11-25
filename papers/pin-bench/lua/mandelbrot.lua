-- mandelbrot (CLBG); prints a checksum of the bytes instead of the image
local n = tonumber(arg and arg[1]) or 200
local sum, bits, nb = 0, 0, 0
local delta = 2.0 / n
for y = 0, n-1 do
    local ci = y*delta - 1.0
    for x = 0, n-1 do
        local cr = x*delta - 1.5
        local zr, zi, zr2, zi2 = 0.0, 0.0, 0.0, 0.0
        local bit = 1
        for _ = 1, 50 do
            zi = 2.0*zr*zi + ci; zr = zr2 - zi2 + cr
            zi2 = zi*zi; zr2 = zr*zr
            if zi2 + zr2 > 4.0 then bit = 0; break end
        end
        bits = bits*2 + bit; nb = nb + 1
        if nb == 8 then sum = sum + bits; bits = 0; nb = 0 end
    end
    if nb > 0 then
        while nb < 8 do bits = bits*2; nb = nb + 1 end
        sum = sum + bits; bits = 0; nb = 0
    end
end
print(string.format("P4 %d %d checksum %d", n, n, sum))
