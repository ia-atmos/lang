-- fasta (CLBG), simple version, as the C one
local IM, IA, IC = 139968, 3877, 29573
local last = 42
local function rnd (max) last = (last*IA + IC) % IM; return max*last/IM end
local ALU = "GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTGG"..
 "GAGGCCGAGGCGGGCGGATCACCTGAGGTCAGGAGTTCGAGA"..
 "CCAGCCTGGCCAACATGGTGAAACCCCGTCTCTACTAAAAAT"..
 "ACAAAAATTAGCCGGGCGTGGTGGCGCGCGCCTGTAATCCCA"..
 "GCTACTCGGGAGGCTGAGGCAGGAGAATCGCTTGAACCCGGG"..
 "AGGCGGAGGTTGCAGTGAGCCGAGATCGCGCCACTGCACTCC"..
 "AGCCTGGGCGACAGAGCGAGACTCCGTCTCAAAAA"
local IUB = {{"a",0.27},{"c",0.12},{"g",0.12},{"t",0.27},{"B",0.02},{"D",0.02},{"H",0.02},{"K",0.02},{"M",0.02},{"N",0.02},{"R",0.02},{"S",0.02},{"V",0.02},{"W",0.02},{"Y",0.02}}
local HS = {{"a",0.3029549426680},{"c",0.1979883004921},{"g",0.1975473066391},{"t",0.3015094502008}}
local write = io.write
local function repeat_ (s, n)
    local len, k = #s, 0
    while n > 0 do
        local m = n < 60 and n or 60
        local line = {}
        for i = 1, m do line[i] = string.sub(s, k+1, k+1); k = (k+1) % len end
        write(table.concat(line), "\n")
        n = n - m
    end
end
local function random_ (t, n)
    local acc = 0.0
    for i = 1, #t do acc = acc + t[i][2]; t[i][2] = acc end
    while n > 0 do
        local m = n < 60 and n or 60
        local line = {}
        for i = 1, m do
            local r = rnd(1.0)
            local j = 1
            while t[j][2] < r do j = j + 1 end
            line[i] = t[j][1]
        end
        write(table.concat(line), "\n")
        n = n - m
    end
end
local n = tonumber(arg and arg[1]) or 1000
write(">ONE Homo sapiens alu\n"); repeat_(ALU, 2*n)
write(">TWO IUB ambiguity codes\n"); random_(IUB, 3*n)
write(">THREE Homo sapiens frequency\n"); random_(HS, 5*n)
