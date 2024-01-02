taper = 0.75
face = circle(0,0,.5)
eye = circle(0.18, 0.2, 0.1)
mouth  = circle(0, 0, 0.4):__sub(
            circle(0, 0, 0.3)):__sub(
                rectangle(-1, 1, -0.1, 1))
face = face:__sub(eye:__add(
        eye:reflect_x(0))):__sub(
           mouth):taper_x_y(0, -0.5, 0.5, 1-taper, 1+taper)
cad:shapes(face)
