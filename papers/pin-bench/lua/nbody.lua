-- n-body (CLBG)
local PI = 3.141592653589793
local SM = 4*PI*PI
local DPY = 365.24
local function body (x,y,z,vx,vy,vz,m) return {x=x,y=y,z=z,vx=vx*DPY,vy=vy*DPY,vz=vz*DPY,m=m*SM} end
local b = {
 body(0,0,0,0,0,0,1),
 body(4.84143144246472090e+00,-1.16032004402742839e+00,-1.03622044471123109e-01,1.66007664274403694e-03,7.69901118419740425e-03,-6.90460016972063023e-05,9.54791938424326609e-04),
 body(8.34336671824457987e+00,4.12479856412430479e+00,-4.03523417114321381e-01,-2.76742510726862411e-03,4.99852801234917238e-03,2.30417297573763929e-05,2.85885980666130812e-04),
 body(1.28943695621391310e+01,-1.51111514016986312e+01,-2.23307578892655734e-01,2.96460137564761618e-03,2.37847173959480950e-03,-2.96589568540237556e-05,4.36624404335156298e-05),
 body(1.53796971148509165e+01,-2.59193146099879641e+01,1.79258772950371181e-01,2.68067772490389322e-03,1.62824170038242295e-03,-9.51592254519715870e-05,5.15138902046611451e-05)}
local NB = #b
local function energy ()
    local e = 0.0
    for i = 1, NB do
        local bi = b[i]
        e = e + 0.5*bi.m*(bi.vx*bi.vx + bi.vy*bi.vy + bi.vz*bi.vz)
        for j = i+1, NB do
            local bj = b[j]
            local dx, dy, dz = bi.x-bj.x, bi.y-bj.y, bi.z-bj.z
            e = e - bi.m*bj.m/math.sqrt(dx*dx + dy*dy + dz*dz)
        end
    end
    return e
end
local function advance (dt)
    for i = 1, NB do
        local bi = b[i]
        for j = i+1, NB do
            local bj = b[j]
            local dx, dy, dz = bi.x-bj.x, bi.y-bj.y, bi.z-bj.z
            local d2 = dx*dx + dy*dy + dz*dz
            local mag = dt/(d2*math.sqrt(d2))
            local bim, bjm = bi.m*mag, bj.m*mag
            bi.vx = bi.vx - dx*bjm; bi.vy = bi.vy - dy*bjm; bi.vz = bi.vz - dz*bjm
            bj.vx = bj.vx + dx*bim; bj.vy = bj.vy + dy*bim; bj.vz = bj.vz + dz*bim
        end
    end
    for i = 1, NB do
        local bi = b[i]
        bi.x = bi.x + dt*bi.vx; bi.y = bi.y + dt*bi.vy; bi.z = bi.z + dt*bi.vz
    end
end
local n = tonumber(arg and arg[1]) or 1000
local px, py, pz = 0.0, 0.0, 0.0
for i = 1, NB do local bi = b[i]; px = px + bi.vx*bi.m; py = py + bi.vy*bi.m; pz = pz + bi.vz*bi.m end
b[1].vx, b[1].vy, b[1].vz = -px/SM, -py/SM, -pz/SM
print(string.format("%.9f", energy()))
for _ = 1, n do advance(0.01) end
print(string.format("%.9f", energy()))
