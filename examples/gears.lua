P  = 14.
PA = 20
N  = 32 

R = N/P/2
RB = R*math.cos(math.rad(PA)) 
a = 1/P 
d = 1.157/P
RO = R+a
RR = R-d

r_2 = mathtree.square(X):__add(mathtree.square(Y))
r = mathtree.sqrt(r_2)
t = math.sqrt(math.pow(R, 2)/math.pow(RB, 2)-1)
rot = math.atan2(math.sin(t)-t*math.cos(t), math.cos(t)+t*math.sin(t)) + math.pi/2/N
tooth = mathtree.sqrt(r_2:__sub(math.pow(RB,2))):__sub(
    (mathtree.atan(Y:__div(X)):__add(mathtree.acos(r:__rdiv(RB)))):__mul(RB)):__sub(RB*rot):shape(true)
tooth = tooth:__and(tooth:reflect_y(0))
teeth = nil
if (N % 2) then
    tooth = tooth:__and(X:__unm())
    __teeth = {}
    for i=0, N do table.insert(__teeth, tooth:rotate(i*360./N)) end
    teeth = table.reduce(tooth.__add, __teeth)
else
    __teeth = {}
    for i=0, N/2 do table.insert(__teeth, tooth:rotate(i*360./N)) end 
    teeth = table.reduce(tooth.__add, __teeth)
end
teeth = teeth:__add(
    circle(0, 0, RR)):__and(
        circle(0, 0, RO)):__sub(
            circle(0, 0, RR*0.9)):bounds(circle(0, 0, RO)):extrusion(-0.1, 0.1):color("red")

ribs = rectangle(-0.002*N, 0.002*N, -RR*0.95, RR*0.95)
__ribs = {}
for i=0, 3 do
    table.insert(__ribs, ribs:rotate(i*120))
end
ribs = table.reduce(ribs.__add, __ribs)
ribs = ribs:__add(
    circle(0, 0, 0.4)):__sub(
        circle(0,0,0.25)):__sub(
            rectangle(-0.06, 0.06, 0, 0.3)):extrusion(-0.08, 0.08):color('green');
base = circle(0, 0, RR*0.95):__sub(
    circle(0, 0, 0.35)):__sub(
        rectangle(-0.06, 0.06, 0, 0.3)):extrusion(-0.04, 0.04):color('blue')
cad:shapes({teeth, ribs, base})
