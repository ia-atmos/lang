-- fannkuch-redux (CLBG); runs on Lua 5.4 and LuaJIT (0-based as the C version)
local n = tonumber(arg and arg[1]) or 7
local perm, perm1, count = {}, {}, {}
for i = 0, n-1 do perm1[i] = i end
local r, maxflips, checksum, permcount = n, 0, 0, 0
while true do
    while r ~= 1 do count[r-1] = r; r = r - 1 end
    for i = 0, n-1 do perm[i] = perm1[i] end
    local flips = 0
    local k = perm[0]
    while k ~= 0 do
        local i, j = 0, k
        while i < j do perm[i], perm[j] = perm[j], perm[i]; i = i + 1; j = j - 1 end
        flips = flips + 1
        k = perm[0]
    end
    if flips > maxflips then maxflips = flips end
    if permcount % 2 == 0 then checksum = checksum + flips else checksum = checksum - flips end
    while true do
        if r == n then
            print(checksum); print(string.format("Pfannkuchen(%d) = %d", n, maxflips))
            return
        end
        local p0 = perm1[0]
        for i = 0, r-1 do perm1[i] = perm1[i+1] end
        perm1[r] = p0
        count[r] = count[r] - 1
        if count[r] > 0 then break end
        r = r + 1
    end
    permcount = permcount + 1
end
