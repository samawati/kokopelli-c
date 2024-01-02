time = '4:02'
__hours = {}
hour = rectangle(-0.04, 0.04, 1.5, 1.8)
for i=0, 11 do table.insert(__hours, hour:rotate(360/12*i)) end
hours = table.reduce(hour.__add, __hours)
__halfs = {}
half = rectangle(-0.03, 0.03, 1.6, 1.8)
for i=0, 23 do table.insert(__halfs, half:rotate(360/24*i)) end
halfs = table.reduce(half.__add, __halfs)
__quarters= {}
quarter = rectangle(-0.02, 0.02, 1.7, 1.8)
for i=0, 47 do table.insert(__quarters, quarter:rotate(360/48.*i)) end
quarters = table.reduce(quarter.__add, __quarters)
hand_width = 0.1
taper = 0.5
hand = rectangle(-hand_width, hand_width, 0, 1.3):__add(
            circle(0, 1.3, hand_width)):__add(
                circle(0, 0, hand_width)):taper_x_y(0, 0, 1.3, 1, taper)
hands = hand:rotate(-360/60.*2.):__add(
            hand:scale_y(0, 0.7):rotate(-360/12.*4.))
cad:shapes({ hands, hours, halfs, quarters })

