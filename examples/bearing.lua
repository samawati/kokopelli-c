local height = 0.3
local wall = 0.1
local OD = 1.0
local ID = 0.25
local BD = 0.25
local Nb = 6

local base = rectangle(ID/2, OD/2, -height/2, height/2):__sub(
                rectangle(ID/2 + wall, OD/2 - wall, -height, height))
local R_r = (OD + ID)/4
local race = rectangle(R_r - BD/2., R_r + BD/2., -height/6., height/6.)
local bearing = base:__sub(race):revolve_y():rotate_x(90)
local __balls = {}
for i=0, Nb-1, 1 do
    table.insert(__balls, sphere(math.cos(i*2*math.pi/Nb)*R_r, 
            math.sin(i*2*math.pi/Nb)*R_r, 0, BD/2.))
end
local cutout = cube(0, 2, -2, 0, -2, 2)
for _ , s in ipairs(__balls) do cad:shapes(s:__sub(cutout)) end
cad:shapes(bearing:__sub(cutout))