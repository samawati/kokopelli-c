arch = cylinder(0,0,-1,0,.1):reflect_yz():move(0,0,-.2)
door = cube(-.1,.1,-.7,0,-.6,-.2):__add(arch)
battlement = cube(-.4,-.2,-.05,.05,.1,.2):__add(
                cube(-.1,.1,-.05,.05,.1,.2)):__add(
                    cube(.2,.4,-.05,.05,.1,.2))
walls = cube(-.6,.6,-.6,.6,-.5,.1):__add(
    cube(-.35,.35,-.35,.35,-1,1)):__sub(door):__add(
        battlement:move(0,-.55,0)):__add(battlement:move(0,.55,0))
battlement = battlement:reflect_xy()
walls = walls:__add(battlement:move(-0.55,0,0)):__add(battlement:move(.55,0,0))
tower = cylinder(0,0,-.5,.5,.2) + cone(0,0,.5,.75,.3)
towers = tower:move(.6,.6,0):__add(
            tower:move(-.6,.6,0)):__add(
                tower:move(.6,-.6,0)):__add(
                    tower:move(-.6,-.6,0))
base = cube(-.9,.9,-.9,.9,-.7,-.5)

cad:shapes({ base, walls, towers })

