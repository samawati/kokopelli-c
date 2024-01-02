N = 17
T = 0.05

r = rectangle(-1, 1, -1, 1):__sub(
        rectangle(-1+T, 1-T, -1+T, 1-T)):rotate(45):move(0, math.sqrt(2), 0):scale_x(0, 0.5):__add(
                circle(0, 1.5*math.sqrt(2), 0.1)):__add(
                    rectangle(-0.1, 0.1, -0.1, 0.1):rotate(45):move(0, 1.8*math.sqrt(2), 0):rotate(360./N/2))

__r = {}
for i=0, N-1 do   table.insert(__r, r:rotate(i*360./N) ) end
r = table.reduce(r.__add, __r):__add(
    circle(0, 0, 1.1+T/2)):__sub(
        circle(0, 0, 1.1)):__add(
            circle(0, 0, 2*math.sqrt(2)):__sub(
                circle(0, 0, 2*math.sqrt(2)-T/2)))

c = circle(0, 0, 0.55):__sub(
    circle(0, 0, 0.55-T)):__add(
        rectangle(-0.55, 0.55, -T/2, T/2)):move(0, 0.6, 0):scale_x(0, 0.5)

__c = {}
for i=0, math.floor(N/2)-1 do   table.insert(__c, c:rotate(i*360./ math.floor(N/2)) ) end
c = table.reduce(c.__add, __c):__add(
        circle(0, 0, 0.4)):__sub(
            circle(0, 0, 0.4-T/2))

cad:shapes(r:__add(c))