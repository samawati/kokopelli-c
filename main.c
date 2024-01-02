#include "glcanvas.h"

int  main(int argc, char** argv) {
    
    //struct image_t *i = libfab_image_load("screenshot.png");
    //libfab_image_save(i, "out1.png");

    struct glcanvas_t *w = glcanvas_create();
    glcanvas_run(w);
    return 0;
}