-- fib, naive recursion
local function fib (n) if n < 2 then return 1 else return fib(n-1) + fib(n-2) end end
print(fib(tonumber(arg and arg[1]) or 30))
