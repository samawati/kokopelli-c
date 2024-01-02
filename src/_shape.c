#include "_shape.h"

#include "sds.h"

#define MATHTREE_MT "mathtree.mt"
#define FABVARS_MT "fabvars.mt"

static struct mathtree_t *
shapes2d_scale_x(struct mathtree_t *part, float x0, float sx) {
    struct mathtree_t *t, *X = NULL;
    X = mathtree_create(
            (x0 !=0) ? sdscatprintf(sdsempty(), "+f%g/-Xf%gf%g", x0, x0, sx) : 
                        sdscatprintf(sdsempty(),"/Xf%g",sx), false, Vec3rgb_nil);          
    t = mathtree_map(part, X, NULL, NULL);
    mathtree_free(X);
    X = mathtree_create(
            (x0 !=0) ? sdscatprintf(sdsempty(), "+f%g*f%g-Xf%g", x0, sx, x0) :
                        sdscatprintf(sdsempty(),"*Xf%g",sx), false, Vec3rgb_nil);
    t->bounds = mathtree_map_bounds(part, X, NULL, NULL);
    mathtree_free(X);
    return t;
}
static struct mathtree_t *
shapes2d_scale_y(struct mathtree_t *part, float y0, float sy) {
    struct mathtree_t *t, *Y = NULL;
    Y = mathtree_create(
            y0 !=0 ? sdscatprintf(sdsempty(), "+f%g/-Yf%gf%g", y0, y0, sy) : 
                        sdscatprintf(sdsempty(),"/Yf%g",sy), false, Vec3rgb_nil);

    t = mathtree_map(part, NULL, Y, NULL);
    mathtree_free(Y);
    Y = mathtree_create(
            y0 !=0 ? sdscatprintf(sdsempty(), "+f%g*f%g-Yf%g", y0, sy, y0) :
                        sdscatprintf(sdsempty(),"/Yf%g",sy), false, Vec3rgb_nil);
    t->bounds = mathtree_map_bounds(part, NULL, Y , NULL);
    mathtree_free(Y);
    return t;
}
static struct mathtree_t *
shapes2d_scale_xy(struct mathtree_t *part, float x0, float y0, float sxy) {
    struct mathtree_t *t = NULL, *X = NULL, *Y = NULL;
    X = mathtree_create(
            x0 !=0 ? sdscatprintf(sdsempty(), "+f%g/-Xf%gf%g", x0, x0, sxy) : 
                        sdscatprintf(sdsempty(),"/Xf%g",sxy), false, Vec3rgb_nil);
    Y = mathtree_create(
            y0 !=0 ? sdscatprintf(sdsempty(), "+f%g/-Yf%gf%g", y0, y0, sxy) : 
                        sdscatprintf(sdsempty(),"/Yf%g", sxy), false, Vec3rgb_nil);
    t = mathtree_map(part, X, Y, NULL);
    mathtree_free(X);
    mathtree_free(Y);
    X =  mathtree_create(
            y0 !=0 ? sdscatprintf(sdsempty(), "+f%g*f%g-Xf%g", x0, sxy, x0) :
                        sdscatprintf(sdsempty(),"*Xf%g", sxy), false, Vec3rgb_nil);
    Y = mathtree_create(
            y0 !=0 ? sdscatprintf(sdsempty(), "+f%g*f%g-Yf%g", y0, sxy, y0) :
                        sdscatprintf(sdsempty(),"*Yf%g",sxy), false, Vec3rgb_nil);   
    t->bounds = mathtree_map_bounds(part, X, Y, NULL);
    mathtree_free(X);
    mathtree_free(Y);
    return t;
}
static struct mathtree_t *
shapes2d_shear_x_y(struct mathtree_t *part, float y0, float y1, float dx0, float dx1) {
    float dx = dx1 - dx0;
    float dy = y1 - y0;
    struct mathtree_t *t = NULL, *X = NULL;
    X = mathtree_create(
            sdscatprintf(sdsempty(), "--Xf%g/*f%g-Yf%gf%g", dx0, dx, y0, dy), false, Vec3rgb_nil);
    t = mathtree_map(part, X, NULL, NULL);
    mathtree_free(X);
    X =  mathtree_create(
            sdscatprintf(sdsempty(), "++Xf%g/*f%g-Yf%gf%g", dx0, dx, y0, dy), false, Vec3rgb_nil);
    t->bounds = mathtree_map_bounds(part, X, NULL, NULL);
    mathtree_free(X);
    return t;
}
static struct mathtree_t *
shapes2d_taper_x_y(struct mathtree_t *part, float x0, float y0, float y1, float s0, float s1) {
    float dy = y1 - y0;
    float ds = s1 - s0;
    float s0y1 = s0 * y1;
    float s1y0 = s1 * y0;
    struct mathtree_t *t = NULL, *X = NULL;
    X = mathtree_create(
            sdscatprintf(sdsempty(), "+f%g/*-Xf%gf%g-+*Yf%gf%gf%g", 
                x0, x0, dy, ds, s0y1, s1y0), false, Vec3rgb_nil);
    t = mathtree_map(part, X, NULL, NULL);
    mathtree_free(X);
    X = mathtree_create(
            sdscatprintf(sdsempty(), "+f%g*-Xf%g/-+*Yf%gf%gf%gf%g", 
                x0, x0, ds, s0y1, s1y0, dy), false, Vec3rgb_nil);
    t->bounds = mathtree_map_bounds(part, X, NULL, NULL);
    mathtree_free(X);
    return t;
}
static struct mathtree_t *
shapes2d_blend(struct mathtree_t * p0, struct mathtree_t *p1, float amount) {
    if (!p0->shape || !p1->shape) return NULL;
    struct mathtree_t *fillet = NULL, *joint = NULL, *t = NULL;
    fillet = mathtree_create(
        sdscatprintf(sdsempty(),"-+rb%srb%sf%g", p0->math, p1->math, amount), true, Vec3rgb_nil);
    joint = mathtree_add(p0, p1);
    struct bounds_t bounds = joint->bounds;
    t = mathtree_add(joint, fillet);
    t->bounds = bounds;
    mathtree_free(fillet);
    mathtree_free(joint);
    return t;
}
static const struct  
Vec3rgb_named{
    const char *name;
    Vec3rgb color;
} colors[15] = {
    {"red",      {.is_set=true, .r=255, .g=0,   .b=0  }},
    {"blue",     {.is_set=true, .r=0,   .g=0,   .b=255}},
    {"green",    {.is_set=true, .r=0,   .g=255, .b=0  }},
    {"white",    {.is_set=true, .r=255, .g=255, .b=255}},
    {"grey",     {.is_set=true, .r=128, .g=128, .b=128}},
    {"black",    {.is_set=true, .r=0,   .g=0,   .b=0  }},
    {"yellow",   {.is_set=true, .r=255, .g=255, .b=0  }},
    {"cyan",     {.is_set=true, .r=0,   .g=255, .b=255}},
    {"magenta",  {.is_set=true, .r=255, .g=0,   .b=255}},
    {"teal",     {.is_set=true, .r=0,   .g=255, .b=255}},
    {"pink",     {.is_set=true, .r=255, .g=0,   .b=255}},
    {"brown",    {.is_set=true, .r=145, .g=82,  .b=45 }},
    {"tan",      {.is_set=true, .r=125, .g=90,  .b=60 }},
    {"navy",     {.is_set=true, .r=0,   .g=0,   .b=128}},
    {NULL,       {.is_set=false, .r=0,  .g=0,   .b=0  }}
};
static struct mathtree_t *
shapes2d_color(struct mathtree_t *t, const char *color) {
    if(t != NULL && color != NULL) {
        for(int i = 0; colors[i].name != NULL; i++) {
            if(!strcmp(color, colors[i].name))
                t->color = colors[i].color;
        }
    }
    return t;
}
static struct mathtree_t *
shapes2d_circle(float x0, float y0, float r) {
    r = fabs(r);
    sds s = x0 ? sdscatprintf(sdsempty(), 
        "-r+q-Xf%gq", x0) : sdscat(sdsempty(), "-r+qXq");
    struct mathtree_t *t = 
        mathtree_create(y0 ? sdscatprintf(
            s, "-Yf%gf%g", y0, r) : sdscatprintf(s, "Yf%g", r), true, Vec3rgb_nil);
    t->bounds.xmin = x0-r, t->bounds.xmax = x0+r;
    t->bounds.ymin = y0-r, t->bounds.ymax = y0+r;
    return t;
}
static struct mathtree_t * 
shapes2d_triangle(float x0, float y0, float x1, float y1, float x2, float y2) {
    float xm = (x0 + x1 + x2)/3.0;
    float ym = (y0 + y1 + y2)/3.0;
    float angles[3] = {0};
    float a0 = atan2(y0 - ym, x0 - xm);
    float a1 = atan2(y1 - ym, x1 - xm);
    float a2 = atan2(y2 - ym, x2 - xm);
    if ((a1 < a0) && (a1 < a2)) {
        angles[0] = a1, angles[1] = a2, angles[2] = a0;
    } else if ((a2 < a0) && (a2 < a1)) {
        angles[0] = a2, angles[1] = a0, angles[2] = a1;
    }
    if (angles[2] > angles[1]) {
        x0 = x1, y0 = y1; 
        x1 = x0, y1 = y0;
    }
    sds e0 = sdscatprintf(sdsempty(), "-*f%g-Xf%g*f%g-Yf%g", y1-y0, x0, x1-x0, y0);
    sds e1 = sdscatprintf(sdsempty(), "-*f%g-Xf%g*f%g-Yf%g", y2-y1, x1, x2-x1, y1);
    sds e2 = sdscatprintf(sdsempty(), "-*f%g-Xf%g*f%g-Yf%g", y0-y2, x2, x0-x2, y2);
    struct mathtree_t *t = mathtree_create(
        sdscatprintf(sdsempty(), "ni%si%s%s", e0, e1, e2), true, Vec3rgb_nil);
    t->bounds.xmin = fmin(x0, fmin(x1, x2)), t->bounds.xmax = fmax(x0, fmax(x1, x2));
    t->bounds.ymin = fmin(y0, fmin(y1, y2)), t->bounds.ymax = fmax(y0, fmax(y1, y2));
    sdsfree(e0);
    sdsfree(e1);
    sdsfree(e2);
    return t;
}
static struct mathtree_t * 
shapes2d_right_triangle(float x0, float y0, float h) {    
    struct mathtree_t *t = mathtree_create(
        sdscatprintf(sdsempty(),"aa-f%fX-f%fY-X-f%f-Yf%f", x0, y0, x0+h, y0), true, Vec3rgb_nil);
    t->bounds.xmin = x0, t->bounds.xmax = x0 + h;
    t->bounds.ymin = y0, t->bounds.ymax = y0 + h;
    return t;
}
static struct mathtree_t * 
shapes2d_rectangle(float x0, float x1, float y0, float y1) {
    struct mathtree_t *t = mathtree_create(
        sdscatprintf(sdsempty(), "aa-f%gX-Xf%ga-f%gY-Yf%g", x0, x1, y0, y1), true, Vec3rgb_nil);
    t->bounds.xmin = x0, t->bounds.xmax = x1;
    t->bounds.ymin = y0, t->bounds.ymax = y1;
    return t;
}
static struct mathtree_t * 
shapes2d_rounded_rectangle(float x0, float x1, float y0, float y1, float r) {
    r *= fmin(x1 - x0, y1 - y0)/2.0;
    struct mathtree_t *__t[6], *t = NULL;
    __t[0] = shapes2d_rectangle(x0, x1, y0+r, y1-r); //r0
    __t[1] = shapes2d_rectangle(x0+r, x1-r, y0, y1); //r1
    __t[2] = shapes2d_circle(x0+r, y0+r, r); //c0
    __t[3] = shapes2d_circle(x0+r, y1-r, r); //c1
    __t[4] = shapes2d_circle(x1-r, y0+r, r); //c2
    __t[5] = shapes2d_circle(x1-r, y1-r, r); //c3
    for(int i=0; i<6; i++) {
        t = (i<=0) ? __t[i] : 
            mathtree_add(t, __t[i]);
    }
    for(int i=0; i<6; i++) mathtree_free(__t[i]);
    return t;
}
static struct mathtree_t * 
shapes2d_reflect_x(struct mathtree_t *part, float x0) {
    struct mathtree_t *t = NULL, *X = NULL;
    X = mathtree_create(
            x0 !=0 ? sdscatprintf(sdsempty(), "-*f2f%gX", x0) : sdsnew("nX"), false, Vec3rgb_nil);
    t = mathtree_map(part, X, NULL, NULL);
    t->bounds = mathtree_map_bounds(part, X, NULL, NULL);
    mathtree_free(X);
    return t;
}
static struct mathtree_t * 
shapes2d_reflect_y(struct mathtree_t *part, float y0) {
    struct mathtree_t *t = NULL, *Y = NULL;
    Y = mathtree_create(
            y0 !=0 ? sdscatprintf(sdsempty(), "-*f2f%gY", y0) : sdsnew("nY"), false, Vec3rgb_nil);
    t = mathtree_map(part,NULL, Y, NULL);
    t->bounds = mathtree_map_bounds(part, NULL, Y, NULL);
    mathtree_free(Y);
    return t;
}
static struct mathtree_t * 
shapes2d_reflect_xy(struct mathtree_t *part) {
    struct mathtree_t *t, *X = NULL, *Y = NULL;
    Y = mathtree_create(sdsnew("X"), false, Vec3rgb_nil);
    X = mathtree_create(sdsnew("Y"), false, Vec3rgb_nil);
    t = mathtree_map(part, X, Y, NULL);
    t->bounds = mathtree_map_bounds(part, X, Y, NULL);
    mathtree_free(X);
    mathtree_free(Y);
    return t;
}
static struct mathtree_t * 
shapes2d_move(struct mathtree_t *part, float dx, float dy, float dz) {
    struct mathtree_t *t, *X = NULL, *Y = NULL, *Z = NULL;
    X = dx!=0 ? mathtree_create(
            sdscatprintf(sdsempty(), "-Xf%g", dx), false, Vec3rgb_nil) : NULL;
    Y = dy!=0 ? mathtree_create(
            sdscatprintf(sdsempty(), "-Yf%g", dy), false, Vec3rgb_nil) : NULL;
    Z = dz!=0 ? mathtree_create(
            sdscatprintf(sdsempty(), "-Zf%g", dz), false, Vec3rgb_nil) : NULL;
    t = mathtree_map(part, X, Y, Z);
    if (isnormal(mathtree_dx(part))) {
        t->bounds.xmin = part->bounds.xmin + dx, t->bounds.xmax = part->bounds.xmax + dx;
    }
    if (isnormal(mathtree_dy(part))) {
        t->bounds.ymin = part->bounds.ymin + dy, t->bounds.ymax = part->bounds.ymax + dy;
    }
    if (isnormal(mathtree_dz(part))) {
        t->bounds.zmin = part->bounds.zmin + dz, t->bounds.zmax = part->bounds.zmax + dz;
    }
    mathtree_free(X);
    mathtree_free(Y);
    mathtree_free(Z);
    return t;
}
static struct mathtree_t * 
shapes2d_rotate(struct mathtree_t *part, double angle) {
    angle *= M_PI/180;
    double ca = cos(angle); 
    double sa = sin(angle);
    double nsa = -sa;
    struct mathtree_t *t, *X = NULL, *Y = NULL;
    X = mathtree_create(
            sdscatprintf(sdsempty(), "+*f%gX*f%gY", ca, sa), false, Vec3rgb_nil);
    Y = mathtree_create(
            sdscatprintf(sdsempty(), "+*f%gX*f%gY", nsa, ca), false, Vec3rgb_nil);        
    t = mathtree_map(part, X, Y, NULL);
    ca = cos(-angle);
    sa = sin(-angle);
    nsa = -sa;
    t->bounds = mathtree_map_bounds(part, X, Y, NULL);
    mathtree_free(X);
    mathtree_free(Y);
    return t;
}
static struct mathtree_t * 
shapes2d_slot(float x, float y, float width, float height, float angle, float chamfer) {
    struct mathtree_t *t = NULL, 
        *slot = NULL, *inset = NULL, *s0 = NULL, 
            *inset_r = NULL, *s1 = NULL, *s1_r = NULL;
    slot = shapes2d_rectangle(-width/2, width/2, -height, 0);
    inset = shapes2d_triangle(width/2, 0, width/2 + height * chamfer, 
                    0, width/2, -chamfer*height);
    s0 = mathtree_add(slot, inset);
    inset_r = shapes2d_reflect_x(inset, 0);
    s1 = mathtree_add(s0, inset_r);
    s1_r = shapes2d_rotate(s1, angle);
    t = shapes2d_move(s1_r, x, y, 0);
    mathtree_free(slot);
    mathtree_free(inset);
    mathtree_free(s0);
    mathtree_free(inset_r);
    mathtree_free(s1);
    mathtree_free(s1_r);
    return t;
}
static struct mathtree_t * 
shapes2d_tab(float x, float y, float width, float height, float angle, float chamfer) {
    struct mathtree_t *t = NULL, *tab = NULL, *cutout = NULL, 
            *s0 = NULL, *cutout_r = NULL, *s1 = NULL, *s1_r = NULL;
    tab = shapes2d_rectangle(-width/2, width/2, 0, height);
    cutout = shapes2d_triangle(width/2 - chamfer*height, height,
                width/2, height, width/2, height - chamfer*height);
    s0 = mathtree_sub(tab, cutout);
    cutout_r = shapes2d_reflect_x(cutout, 0);
    s1 = mathtree_add(s0,  cutout_r);
    s1_r = shapes2d_rotate(s1, angle);
    t = shapes2d_move(s1_r, x, y, 0);
    mathtree_free(tab);
    mathtree_free(cutout);
    mathtree_free(s0);
    mathtree_free(cutout_r);
    mathtree_free(s1);
    mathtree_free(s1_r);
    return t;
}
static struct mathtree_t * 
shapes3d_extrusion(struct mathtree_t *__t, float z0, float z1) {
    struct mathtree_t *t = mathtree_create(
            sdscatprintf(sdsempty(), "a%sa-f%gZ-Zf%g", 
                    __t->math, z0, z1), true, __t->color);
    t->bounds.xmin = __t->bounds.xmin;
    t->bounds.xmax = __t->bounds.xmax;
    t->bounds.ymin = __t->bounds.ymin;
    t->bounds.ymax = __t->bounds.ymax;
    t->bounds.zmin = z0;
    t->bounds.zmax = z1;
    return t;
}
static struct mathtree_t * 
shapes3d_cylinder(float x0, float y0, float z0, float z1, float r) {
    struct mathtree_t *t = NULL, *c = NULL;
    c = shapes2d_circle(x0, y0, r);
    t = shapes3d_extrusion(c, z0, z1);
    mathtree_free(c);
    return t;
}
static struct mathtree_t * 
shapes3d_sphere(float x0, float y0, float z0, float r) {
    sds X = (x0 != 0) ? sdscatprintf(sdsempty(), "-Xf%g", x0) : sdsnew("X");
    sds Y = (y0 != 0) ? sdscatprintf(sdsempty(), "-Yf%g", y0) : sdsnew("Y");
    sds Z = (z0 != 0) ? sdscatprintf(sdsempty(), "-Zf%g", z0) : sdsnew("Z");
    struct mathtree_t *t = mathtree_create(
        sdscatprintf(sdsempty(), "-r++q%sq%sq%sf%g", X, Y, Z, r), true, Vec3rgb_nil);
    t->bounds.xmin = x0-r, t->bounds.xmax = x0+r;
    t->bounds.ymin = y0-r, t->bounds.ymax = y0+r;
    t->bounds.zmin = z0-r, t->bounds.zmax = z0+r;
    sdsfree(X);
    sdsfree(Y); 
    sdsfree(Z);
    return t;
}
static struct mathtree_t * 
shapes3d_cube(float x0, float x1, float y0, float y1, float z0, float z1) {
    struct mathtree_t *t = NULL, *r = NULL;
    r = shapes2d_rectangle(x0, x1, y0, y1);
    t = shapes3d_extrusion(r, z0, z1);
    mathtree_free(r);
    return t;
}
static struct mathtree_t *
shapes3d_taper_xy_z(struct mathtree_t *part, float x0, float y0, float z0, float z1, float s0, float s1) {
    float dz = z1 - z0;
    struct mathtree_t *t = NULL, *X = NULL, *Y = NULL;
    X = mathtree_create(
            sdscatprintf(sdsempty(), "+f%g/*-Xf%gf%g+*f%g-Zf%g*f%g-f%gZ", 
                x0, x0, dz, s1, z0, s0, z1), false, Vec3rgb_nil);
    Y = mathtree_create(
            sdscatprintf(sdsempty(), "+f%g/*-Yf%gf%g+*f%g-Zf%g*f%g-f%gZ",
                y0, y0, dz, s1, z0, s0, z1), false, Vec3rgb_nil);   
    t = mathtree_map(part, X, Y, NULL);
    mathtree_free(X);
    mathtree_free(Y);
    X = mathtree_create(
            sdscatprintf(sdsempty(), "+/*-Xf%g+*f%g-Zf%g*f%g-f%gZf%gf%g", 
                x0, s1, z0, s0, z1, dz, x0), false, Vec3rgb_nil);
    Y = mathtree_create(
            sdscatprintf(sdsempty(), "+/*-Yf%g+*f%g-Zf%g*f%g-f%gZf%gf%g", 
                y0, s1, z0, s0, z1, dz, y0), false, Vec3rgb_nil);
    t->bounds = mathtree_map_bounds(part, X, Y, NULL);
    mathtree_free(X);
    mathtree_free(Y);
    return t;
}
static struct mathtree_t * 
shapes3d_cone(float x0, float y0, float z0, float z1, float r) {
    struct mathtree_t *t = NULL, *c = NULL;
    c = shapes3d_cylinder(x0, y0, z0, z1, r);
    t = shapes3d_taper_xy_z(c, x0, y0, z0, z1, 1.0, 0.0);
    mathtree_free(c);
    return t;
}
static struct mathtree_t * 
shapes3d_pyramid(float x0, float x1, float y0, float y1, float z0, float z1) {
    struct mathtree_t *t = NULL, *c = NULL;
    c = shapes3d_cube(x0, x1, y0, y1, z0, z1);
    t = shapes3d_taper_xy_z(c, (x0+x1)/2., (y0+y1)/2., z0, z1, 1.0, 0.0);
    mathtree_free(c);
    return t;
}
static struct mathtree_t * 
shapes3d_rotate_x(struct mathtree_t *part, double angle) {
    angle *= M_PI/180;
    double ca = cos(angle);
    double sa = sin(angle);
    double nsa = -sa;
    struct mathtree_t *t = NULL, *Y = NULL, *Z = NULL;
    Y = mathtree_create(
            sdscatprintf(sdsempty(), "+*f%gY*f%gZ",ca, sa), false, Vec3rgb_nil);
    Z = mathtree_create(
            sdscatprintf(sdsempty(), "+*f%gY*f%gZ", nsa, ca), false, Vec3rgb_nil);
    t = mathtree_map(part, NULL, Y, Z);
    ca = cos(-angle);
    sa = sin(-angle);
    nsa = -sa;
    mathtree_free(Y);
    mathtree_free(Z);
    Y = mathtree_create(
            sdscatprintf(sdsempty(), "+*f%gY*f%gZ", ca, sa), false, Vec3rgb_nil);
    Z = mathtree_create(
            sdscatprintf(sdsempty(), "+*f%gY*f%gZ", nsa, ca), false, Vec3rgb_nil);   
    t->bounds = mathtree_map_bounds(part, NULL, Y, Z);
    mathtree_free(Y);
    mathtree_free(Z);
    return t;
}
static struct mathtree_t * 
shapes3d_rotate_y(struct mathtree_t *part, double angle) {
    angle *= M_PI/180;
    double ca = cos(angle);
    double sa = sin(angle);
    double nsa = -sa;
    struct mathtree_t *t = NULL, *X = NULL, *Z = NULL;
    X = mathtree_create(
            sdscatprintf(sdsempty(), "+*f%gX*f%gZ",ca, sa), false, Vec3rgb_nil);
    Z = mathtree_create(
            sdscatprintf(sdsempty(), "+*f%gX*f%gZ", nsa, ca), false, Vec3rgb_nil);   
    t = mathtree_map(part,X, NULL, Z);
    ca = cos(-angle);
    sa = sin(-angle);
    nsa = -sa;
    mathtree_free(X);
    mathtree_free(Z);
    X = mathtree_create(
            sdscatprintf(sdsempty(), "+*f%gX*f%gZ", ca, sa), false, Vec3rgb_nil);
    Z = mathtree_create(
            sdscatprintf(sdsempty(), "+*f%gX*f%gZ", nsa, ca), false, Vec3rgb_nil);   
    t->bounds = mathtree_map_bounds(part, X ,NULL, Z);
    mathtree_free(X);
    mathtree_free(Z);
    return t;
}
static struct mathtree_t * 
shapes3d_reflect_z(struct mathtree_t *part, float z0) {
    struct mathtree_t *t = NULL, *Z = NULL;
    Z = mathtree_create(
            z0 !=0 ? sdscatprintf(sdsempty(), "-*f2f%gZ", z0) : sdsnew("nZ"), false, Vec3rgb_nil);
    t = mathtree_map(part,NULL, NULL, Z);
    t->bounds = mathtree_map_bounds(part, NULL, NULL, Z);
    mathtree_free(Z);
    return t;
}
static struct mathtree_t * 
shapes3d_reflect_xz(struct mathtree_t *part) {
    struct mathtree_t *t = NULL, *X = NULL, *Z = NULL;
    X = mathtree_create(sdsnew("X"), false, Vec3rgb_nil);
    Z = mathtree_create(sdsnew("Z"), false, Vec3rgb_nil);  
    t = mathtree_map(part, X, NULL, Z);
    t->bounds = mathtree_map_bounds(part, X, NULL, Z);
    mathtree_free(X);
    mathtree_free(Z);
    return t;
}
static struct mathtree_t * 
shapes3d_reflect_yz(struct mathtree_t *part) {
    struct mathtree_t *t = NULL, *Y = NULL, *Z = NULL;
    Y = mathtree_create(sdsnew("Y"), false, Vec3rgb_nil);
    Z = mathtree_create(sdsnew("Z"), false, Vec3rgb_nil); 
    t = mathtree_map(part,NULL, Y, Z);
    t->bounds = mathtree_map_bounds(part, NULL, Y, Z);
    mathtree_free(Y);
    mathtree_free(Z);
    return t;
}
static struct mathtree_t * 
shapes3d_scale_z(struct mathtree_t *part, float z0, float sz) {
    struct mathtree_t *t = NULL, *Z = NULL;
    Z = mathtree_create(
            z0 !=0 ? sdscatprintf(sdsempty(),"+f%g/-Zf%gf%g", z0, z0, sz) : 
                sdscatprintf(sdsempty(),"/Zf%g", sz) , false, Vec3rgb_nil);
    t = mathtree_map(part,NULL, NULL, Z);
    mathtree_free(Z);
    Z = mathtree_create(
            z0 !=0 ? sdscatprintf(sdsempty(),"+f%g*f%g-Zf%g", z0, sz, z0) : 
                sdscatprintf(sdsempty(),"Zf%g", sz) , false, Vec3rgb_nil);   
    t->bounds = mathtree_map_bounds(part, NULL, NULL, Z);
    mathtree_free(Z);
    return t;
}
static struct mathtree_t * 
shapes3d_shear_x_z(struct mathtree_t *part, float z0, float z1, float dx0, float dx1) {
    struct mathtree_t *t = NULL, *X = NULL;
    X = mathtree_create(
            sdscatprintf(sdsempty(), "--Xf%g/*f%g-Zf%gf%g",
                    dx0, (dx1-dx0), z0, (z1-z0)), false, Vec3rgb_nil) ;
    t = mathtree_map(part, X, NULL, NULL);
    mathtree_free(X);
    X =  mathtree_create(
            sdscatprintf(sdsempty(), "++Xf%g/*f%g-Zf%gf%g", 
                    dx0, (dx1-dx0), z0, (z1-z0)), false, Vec3rgb_nil);
    t->bounds = mathtree_map_bounds(part, X, NULL, NULL);
    mathtree_free(X);
    return t;
}
static struct mathtree_t * 
shapes3d_revolve_y(struct mathtree_t *part) {
    struct mathtree_t *t = NULL, *X = NULL;
    X = mathtree_create(sdsnew("r+qXqZ"), false, Vec3rgb_nil);
    t = mathtree_map(part, X, NULL, NULL);
    if (part->bounds.xmin !=0 && part->bounds.xmax !=0) {
        t->bounds.xmin = fmin(-fabs(part->bounds.xmin), -fabs(part->bounds.xmax));
        t->bounds.xmax = fmax(fabs(part->bounds.xmin), fabs(part->bounds.xmax));
        t->bounds.ymin = part->bounds.ymin;
        t->bounds.ymax = part->bounds.ymax;
        t->bounds.zmin = t->bounds.xmin;
        t->bounds.zmax = t->bounds.xmax;
    }
    mathtree_free(X);
    return t;
}
static struct mathtree_t * 
shapes3d_revolve_x(struct mathtree_t *part) {
    struct mathtree_t *t = NULL, *Y = NULL;
    Y = mathtree_create(sdsnew("r+qYqZ"), false, Vec3rgb_nil);
    t = mathtree_map(part, NULL, Y, NULL);
    if (part->bounds.xmin !=0 && part->bounds.xmax !=0) {
        t->bounds.xmin = part->bounds.xmin;
        t->bounds.xmax = part->bounds.xmax;
        t->bounds.ymin = fmin(-fabs(part->bounds.ymin), -fabs(part->bounds.ymax));
        t->bounds.ymax = fmax(fabs(part->bounds.ymin), fabs(part->bounds.ymax));
        t->bounds.zmin = t->bounds.ymin;
        t->bounds.zmax = t->bounds.ymax;
    }
    mathtree_free(Y);
    return t;
}
static struct mathtree_t * 
shapes3d_loft(struct mathtree_t *p0, struct mathtree_t *p1, float z0, float z1) {
    if (!p0->shape || !p1->shape) return NULL;
    /*
    (((Z-z1)*(Z-z2)/((z0-z1)*(z0-z2))+
    0.5*(Z-z0)*(Z-z2)/((z1-z0)*(z1-z2)))*(part0)+
    (0.5*(Z-z0)*(Z-z2)/((z1-z0)*(z1-z2))+
    (Z-z0)*(Z-z1)/((z2-z0)*(z2-z1)))*(part1))
    */
	return NULL;
}
static struct mathtree_t *
__to_mathtree(lua_State *L, int index) {
    struct mathtree_t **t = lua_touserdata(L, index);
    if (t != NULL) {
        if (lua_getmetatable(L, index)) {
            lua_getfield(L, LUA_REGISTRYINDEX, MATHTREE_MT);
            if (lua_rawequal(L, -1, -2)) {
                lua_pop(L, 2);
                return *t;
            }
        }
    }
    if (lua_type(L, index) == LUA_TSTRING)
        return mathtree_create(sdsnew(lua_tostring(L, index)), false, Vec3rgb_nil);
    else if (lua_type(L, index) == LUA_TNUMBER)
        return mathtree_constant(lua_tonumber(L, index));
    luaL_typerror(L, index, MATHTREE_MT);
    return NULL;
}
static int
lua_mathtree_add(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_add(
        __to_mathtree(L, 1), 
        __to_mathtree(L, 2));
    *t = __t;
    return 1;
}
static int
lua_mathtree_sub(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_sub(
        __to_mathtree(L, 1), 
        __to_mathtree(L, 2));
    *t = __t;
    return 1;
}
static int
lua_mathtree_and(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_and(
        __to_mathtree(L, 1), 
        __to_mathtree(L, 2));
    *t = __t;
    return 1;
} 
static int
lua_mathtree_or(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_or(
        __to_mathtree(L, 1), 
        __to_mathtree(L, 2));
    *t = __t;
    return 1;
} 
static int
lua_mathtree_mul(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_mul( 
        __to_mathtree(L, 1), 
        __to_mathtree(L, 2));
    *t = __t;
    return 1;
} 
static int
lua_mathtree_rdiv(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_rdiv(
        __to_mathtree(L, 1), 
        __to_mathtree(L, 2));
    *t = __t;
    return 1;
} 
static int
lua_mathtree_div(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_div(
        __to_mathtree(L, 1), 
        __to_mathtree(L, 2));
    *t = __t;
    return 1;
} 
static int
lua_mathtree_unm(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_unm(__to_mathtree(L, 1));
    *t = __t;
    return 1;
} 
static int 
lua_mathtree_create(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_create(
        luaL_checkstring(L, 1), lua_toboolean(L, 2), Vec3rgb_nil);
    *t = __t;
    return 1;
}
static int 
lua_mathtree_sqrt(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_sqrt(__to_mathtree(L, 1));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_abs(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_abs(__to_mathtree(L, 1));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_square(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_square(__to_mathtree(L, 1));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_sin(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_sin(__to_mathtree(L, 1));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_cos(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_cos(__to_mathtree(L, 1));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_tan(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_tan(__to_mathtree(L, 1));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_asin(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_asin(__to_mathtree(L, 1));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_atan(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_atan(__to_mathtree(L, 1));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_acos(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = mathtree_acos(__to_mathtree(L, 1));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_X_global(lua_State *L) {
    struct mathtree_t **X = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__X = mathtree_X();
    *X = __X;
    lua_setglobal(L, "X");
    return 0;
}
static int 
lua_mathtree_Y_global(lua_State *L) {
    struct mathtree_t **Y = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__Y = mathtree_Y();
    *Y = __Y;
    lua_setglobal(L, "Y");
    return 0;
}
static int 
lua_mathtree_Z_global(lua_State *L) {
    struct mathtree_t **Z = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__Z = mathtree_Z();
    *Z = __Z;
    lua_setglobal(L, "Z");
    return 0;
}
static int 
lua_mathtree_shape(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = __to_mathtree(L, 1);
    __t->shape = lua_toboolean(L, 2);
    *t = __t;
    return 1;
}
static int 
lua_mathtree_bounds(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = __to_mathtree(L, 1);
    __t->bounds = __to_mathtree(L, 2)->bounds;
    *t = __t;
    return 1;
}
static int 
lua_mathtree_color(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_color(
        __to_mathtree(L, 1), luaL_checkstring(L, 2));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_move(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_move(
        __to_mathtree(L, 1), 
            luaL_checknumber(L, 2), 
                luaL_checknumber(L, 3), 
                    luaL_checknumber(L, 4));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_reflect_yz(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes3d_reflect_yz(
        __to_mathtree(L, 1));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_reflect_xy(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_reflect_xy(
        __to_mathtree(L, 1));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_tab(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_tab(
        luaL_checknumber(L, 1), 
            luaL_checknumber(L, 2), 
                luaL_checknumber(L, 3), 
                    luaL_checknumber(L, 4),
                luaL_checknumber(L, 5),
            luaL_checknumber(L, 6));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_slot(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_slot(
        luaL_checknumber(L, 1), 
            luaL_checknumber(L, 2), 
                luaL_checknumber(L, 3), 
                    luaL_checknumber(L, 4),
                luaL_checknumber(L, 5),
            luaL_checknumber(L, 6));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_rotate(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_rotate(
        __to_mathtree(L, 1), 
            luaL_checknumber(L, 2));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_reflect_x(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_reflect_x(
        __to_mathtree(L, 1), 
            luaL_checknumber(L, 2));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_reflect_y(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_reflect_y(
        __to_mathtree(L, 1), 
            luaL_checknumber(L, 2));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_scale_x(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_scale_x(
        __to_mathtree(L, 1)
        , luaL_checknumber(L, 2)
        , luaL_checknumber(L, 3));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_scale_y(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_scale_y(
        __to_mathtree(L, 1)
        , luaL_checknumber(L, 2)
        , luaL_checknumber(L, 3));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_scale_xy(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_scale_xy(
        __to_mathtree(L, 1)
        , luaL_checknumber(L, 2)
        , luaL_checknumber(L, 3)
        , luaL_checknumber(L, 4));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_shear_x_y(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_shear_x_y(
        __to_mathtree(L, 1)
        , luaL_checknumber(L, 2)
        , luaL_checknumber(L, 3)
        , luaL_checknumber(L, 4)
        , luaL_checknumber(L, 5));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_taper_x_y(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_taper_x_y(
        __to_mathtree(L, 1)
        , luaL_checknumber(L, 2)
        , luaL_checknumber(L, 3)
        , luaL_checknumber(L, 4)
        , luaL_checknumber(L, 5)
        , luaL_checknumber(L, 6));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_extrusion(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes3d_extrusion(
        __to_mathtree(L, 1)
         , luaL_checknumber(L, 2)
         , luaL_checknumber(L, 3));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_blend(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_blend(
        __to_mathtree(L, 1)
        , __to_mathtree(L, 2)
        , luaL_checknumber(L, 3));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_rotate_x(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes3d_rotate_x(
        __to_mathtree(L, 1)
        , luaL_checknumber(L, 2));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_rotate_y(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes3d_rotate_y(
        __to_mathtree(L, 1)
        , luaL_checknumber(L, 2));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_reflect_z(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes3d_reflect_z(
        __to_mathtree(L, 1)
        , luaL_checknumber(L, 2));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_reflect_xz(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes3d_reflect_xz(
        __to_mathtree(L, 1));
    *t = __t;
    return 1;
}

static int 
lua_mathtree_scale_z(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes3d_scale_z(
        __to_mathtree(L, 1)
         , luaL_checknumber(L, 2)
         , luaL_checknumber(L, 3));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_shear_x_z(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes3d_shear_x_z(
        __to_mathtree(L, 1)
         , luaL_checknumber(L, 2)
         , luaL_checknumber(L, 3)
         , luaL_checknumber(L, 4)
         , luaL_checknumber(L, 5));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_taper_xy_z(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes3d_taper_xy_z(
        __to_mathtree(L, 1)
         , luaL_checknumber(L, 2)
         , luaL_checknumber(L, 3)
         , luaL_checknumber(L, 4)
         , luaL_checknumber(L, 5)
         , luaL_checknumber(L, 6)
         , luaL_checknumber(L, 7));
    *t = __t;
    return 1;
}
static int 
lua_mathtree_revolve_y(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes3d_revolve_y(
        __to_mathtree(L, 1));
    *t = __t;        
    return 1;
}
static int 
lua_mathtree_revolve_x(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes3d_revolve_x(
        __to_mathtree(L, 1));
   *t = __t;
   return 1;
}
static int 
lua_mathtree_loft(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes3d_loft(
        __to_mathtree(L, 1) , __to_mathtree(L, 2)
        , luaL_checknumber(L, 3)
        , luaL_checknumber(L, 4)
        );
    *t = __t;
    return 1;
}
static int 
lua_mathtree__gc(lua_State *L) {
    mathtree_free(__to_mathtree(L, 1));
    return 0;
}
static int 
lua_mathtree__tostring(lua_State *L) {
    struct mathtree_t *t = __to_mathtree(L, 1);
    lua_pushfstring(L, "%s:(%p) - %s", MATHTREE_MT, t, t->math);
    return 1;
}
static const 
luaL_Reg lua_mathtree_userdata_lib[] = {
    { "__sub", lua_mathtree_sub},
    { "__and", lua_mathtree_and},
    { "__or", lua_mathtree_or},
    { "__mul", lua_mathtree_mul},
    { "__div", lua_mathtree_div},
    { "__rdiv", lua_mathtree_rdiv},
    { "__unm", lua_mathtree_unm},
    { "__add", lua_mathtree_add},
    { "__gc",  lua_mathtree__gc},
    { "__tostring", lua_mathtree__tostring},
    { "shape", lua_mathtree_shape},
    { "bounds",lua_mathtree_bounds},
    { "color", lua_mathtree_color },
    { "move", lua_mathtree_move},
    { "reflect_yz", lua_mathtree_reflect_yz},
    { "tab", lua_mathtree_tab},
    { "slot", lua_mathtree_slot},
    { "rotate", lua_mathtree_rotate},
    { "reflect_x", lua_mathtree_reflect_x},
    { "reflect_y", lua_mathtree_reflect_y},
    { "reflect_xy", lua_mathtree_reflect_xy},
    { "scale_x", lua_mathtree_scale_x},
    { "scale_y", lua_mathtree_scale_y},
    { "scale_xy", lua_mathtree_scale_xy},
    { "shear_x_y", lua_mathtree_shear_x_y},
    { "taper_x_y", lua_mathtree_taper_x_y},
    { "blend", lua_mathtree_blend},
    { "extrusion", lua_mathtree_extrusion},
    { "rotate_x", lua_mathtree_rotate_x},
    { "rotate_y", lua_mathtree_rotate_y},
    { "reflect_z", lua_mathtree_reflect_z},
    { "reflect_xz", lua_mathtree_reflect_xz},
    { "scale_z", lua_mathtree_scale_z},
    { "shear_x_z", lua_mathtree_shear_x_z},
    { "taper_xy_z", lua_mathtree_taper_xy_z},
    { "revolve_y", lua_mathtree_revolve_y},
    { "revolve_x", lua_mathtree_revolve_x},
    { "loft", lua_mathtree_loft},    
    { NULL, NULL }
};
static const 
luaL_Reg lua_mathtree_libs[] = {
    { "create", lua_mathtree_create },
    { "sqrt",   lua_mathtree_sqrt   },
    { "abs",    lua_mathtree_abs    },
    { "square", lua_mathtree_square },
    { "sin",    lua_mathtree_sin    },
    { "cos",    lua_mathtree_cos    },
    { "tan",    lua_mathtree_tan    },
    { "asin",   lua_mathtree_asin   },
    { "atan",   lua_mathtree_atan   },
    { "acos",   lua_mathtree_acos   },
    { NULL, NULL }
};

LUALIB_API int 
luaopen_mathtree(lua_State * L) {
    luaL_newmetatable(L, MATHTREE_MT);
    luaL_register(L, 0, lua_mathtree_userdata_lib);
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -2);
    lua_rawset(L, -3);
    luaL_register(L, "mathtree", lua_mathtree_libs);
    lua_settop(L, 0);
    lua_mathtree_X_global(L);
    lua_mathtree_Y_global(L);
    lua_mathtree_Z_global(L);
    lua_settop(L, 0);
    return 0;
}

static int 
lua_shape_circle(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_circle(luaL_checknumber(L, 1), 
                luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    *t = __t;
   return 1;
}
static int 
lua_shape_triangle(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_triangle(
            luaL_checknumber(L, 1), 
                luaL_checknumber(L, 2), 
                    luaL_checknumber(L, 3),
                    luaL_checknumber(L, 4), 
                luaL_checknumber(L, 5), 
            luaL_checknumber(L, 6));
    *t = __t;
     return 1;
}
static int 
lua_shape_right_triangle(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);    
    struct mathtree_t *__t = shapes2d_right_triangle(
            luaL_checknumber(L, 1), 
                luaL_checknumber(L, 2), 
            luaL_checknumber(L, 3));
    *t = __t;
    return 1;
}
static int 
lua_shape_rectangle(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_rectangle(
        luaL_checknumber(L, 1), 
            luaL_checknumber(L, 2), 
                luaL_checknumber(L, 3), 
            luaL_checknumber(L, 4) );
    *t = __t;
    return 1;
}
static int 
lua_shape_rounded_rectangle(lua_State *L){
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = shapes2d_rounded_rectangle(
        luaL_checknumber(L, 1), 
            luaL_checknumber(L, 2), 
                luaL_checknumber(L, 3), 
                    luaL_checknumber(L, 4),
                luaL_checknumber(L, 5) );
    *t = __t;
    return 1;
}
static int 
lua_shape_cylinder(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
  struct mathtree_t *__t = shapes3d_cylinder(
        luaL_checknumber(L, 1), 
            luaL_checknumber(L, 2), 
                luaL_checknumber(L, 3), 
                    luaL_checknumber(L, 4),
                luaL_checknumber(L, 5));
    *t = __t;
    return 1;
}
static int 
lua_shape_sphere(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
  struct mathtree_t *__t = shapes3d_sphere(
        luaL_checknumber(L, 1), 
            luaL_checknumber(L, 2), 
                luaL_checknumber(L, 3), 
            luaL_checknumber(L, 4));
    *t = __t;
    return 1;
}
static int 
lua_shape_cube(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
  struct mathtree_t *__t = shapes3d_cube(
        luaL_checknumber(L, 1), 
            luaL_checknumber(L, 2), 
                luaL_checknumber(L, 3), 
            luaL_checknumber(L, 4),
        luaL_checknumber(L, 5),
    luaL_checknumber(L, 6));
    *t = __t;
    return 1;
}
static int 
lua_shape_cone(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
  struct mathtree_t *__t = shapes3d_cone(
        luaL_checknumber(L, 1), 
            luaL_checknumber(L, 2), 
                luaL_checknumber(L, 3), 
            luaL_checknumber(L, 4),
        luaL_checknumber(L, 5));
    *t = __t;
    return 1;
}
static int 
lua_shape_pyramid(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
  struct mathtree_t *__t = shapes3d_pyramid(
        luaL_checknumber(L, 1), 
            luaL_checknumber(L, 2), 
                luaL_checknumber(L, 3), 
            luaL_checknumber(L, 4),
        luaL_checknumber(L, 5),
    luaL_checknumber(L, 5));
    *t = __t;
    return 1;
}

static const
luaL_Reg lua_shape_libs[] = {
    { "circle", lua_shape_circle},
    { "triangle", lua_shape_triangle},
    { "right_triangle", lua_shape_right_triangle},
    { "rectangle", lua_shape_rectangle},
    { "rounded_rectangle", lua_shape_rounded_rectangle},
    { "cylinder", lua_shape_cylinder},
    { "sphere", lua_shape_sphere},
    { "cube", lua_shape_cube},
    { "cone", lua_shape_cone},
    { "pyramid", lua_shape_pyramid},
    { NULL, NULL }
};
LUALIB_API int 
luaopen_shape(lua_State * L) {
    const luaL_Reg *lib = lua_shape_libs;
    for (; lib->name; lib++) {
        lua_register(L, lib->name, lib->func);
    }
    return 0;
}
struct fabvars_t *
koko_fabvars_create(lua_State *L) {
    struct fabvars_t *f = lua_newuserdata(L, sizeof(struct fabvars_t));
    f->render_mode = RENDER_2D;
    f->mm_per_unit = 25.4;
    f->border = 0.05;
    luaL_getmetatable(L, FABVARS_MT);
    lua_setmetatable(L, -2);
    lua_setglobal(L, "cad");
    vec_init(&f->_shapes);
    return f;
}
static int
lua_fabvars_shapes(lua_State *L) {
    struct fabvars_t *cad = luaL_checkudata(L, 1, FABVARS_MT);
    switch (lua_type(L, 2)) {
        case LUA_TTABLE : {
            lua_pushnil(L);
            while (lua_next(L, 2)) {
                (void)vec_push(&cad->_shapes, __to_mathtree(L, -1)); 
                lua_pop(L, 1);
            } break;
        } 
        default: {
            (void)vec_push(&cad->_shapes, __to_mathtree(L, 2));
            break;
        }
    }
    return 0;
}
static int
lua_fabvars_render_mode(lua_State *L) {
    struct fabvars_t *f = luaL_checkudata(L, 1, FABVARS_MT);
    f->render_mode = luaL_checknumber(L, 2);
    return 0;
}
static int
lua_fabvars_mm_per_unit(lua_State *L) {
    struct fabvars_t *f = luaL_checkudata(L, 1, FABVARS_MT);
    f->mm_per_unit = luaL_checknumber(L, 2);
    return 0;
}
static int
lua_fabvars_border(lua_State *L) {
    struct fabvars_t *f = luaL_checkudata(L, 1, FABVARS_MT);
    f->border = luaL_checknumber(L, 2);
    return 0;
}
static const 
luaL_reg lua_fabvars_userdata_lib[] = {
  {"shapes", lua_fabvars_shapes },
  {"border" , lua_fabvars_border},
  {"render_mode", lua_fabvars_render_mode },
  {"mm_per_unit", lua_fabvars_mm_per_unit },
  { NULL, NULL }
};
LUALIB_API int 
luaopen_cad(lua_State *L) {
    luaL_newmetatable(L, FABVARS_MT);
    luaL_register(L, 0, lua_fabvars_userdata_lib);
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -2);
    lua_rawset(L, -3);
    lua_settop(L, 0);
    return 0;
}
static struct glyph_t {
    struct mathtree_t *shape;
    float width;
} _glyphs[255];
static void
glyph_0(void) {
    static struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = shapes2d_triangle(0, 0, 0.35, 1, 0.1, 0);
    s1 = mathtree_add(s0, shapes2d_triangle(0.1, 0, 0.35, 1, 0.45, 1));
    s2 = mathtree_add(s1, shapes2d_triangle(0.35, 1, 0.45, 1, 0.8, 0));
    s3 = mathtree_add(s2, shapes2d_triangle(0.7, 0, 0.35, 1, 0.8, 0));
    s4 = mathtree_add(s3, shapes2d_rectangle(0.2, 0.6, 0.3, 0.4));
    _glyphs['A'] = (struct glyph_t){ 
        .shape = s4,
        .width = 0.8 
    };
    mathtree_free(s0);
    mathtree_free(s1);
    mathtree_free(s2);
    mathtree_free(s3);
}
static void
glyph_1(void) {
    static struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = shapes2d_circle(0.25, 0.275, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.25, 0.275, 0.175));
    s2 = shapes2d_shear_x_y(s1, 0, 0.35, 0, 0.1);
    s3 = mathtree_add(s2, shapes2d_rectangle(0.51, 0.61, 0, 0.35));
    s4 = shapes2d_move(s3, -0.05, 0, 0);
    _glyphs['a' ] = (struct glyph_t){ 
        .shape = s4,
        .width = 0.58 
    };
    mathtree_free(s0);
    mathtree_free(s1);
    mathtree_free(s2);
    mathtree_free(s3);
}
static void
glyph_2(void) {
    static struct mathtree_t *s0, *s1, *s2,
        *s3, *s4, *s5, *s6, *s7, *s8;
    s0 = shapes2d_circle(0.3, 0.725, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.3, 0.725, 0.175));
    s2 = mathtree_add(s1, shapes2d_circle(0.3, 0.275, 0.275));
    s3 = mathtree_sub(s2, shapes2d_circle(0.3, 0.275, 0.175));
    s4 = mathtree_and(s3, shapes2d_rectangle(0.3, 1, 0, 1));
    s5 = mathtree_add(s4, shapes2d_rectangle(0, 0.1, 0, 1));
    s6 = mathtree_add(s5, shapes2d_rectangle(0.1, 0.3, 0, 0.1));
    s7 = mathtree_add(s6, shapes2d_rectangle(0.1, 0.3, 0.45, 0.55));
    s8 = mathtree_add(s7, shapes2d_rectangle(0.1, 0.3, 0.9, 1));
    _glyphs['B'] = (struct glyph_t){ 
        .shape = s8,
        .width = 0.575
    };
    mathtree_free(s0);
    mathtree_free(s1);
    mathtree_free(s2);
    mathtree_free(s3);
    mathtree_free(s4);
    mathtree_free(s5);
    mathtree_free(s6);
    mathtree_free(s7);
}
static void
glyph_3(void) {
    static struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = shapes2d_circle(0.25, 0.275, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.25, 0.275, 0.175));
    s2 = mathtree_and(s1, mathtree_add(shapes2d_rectangle(0.25, 1, 0, 0.275), shapes2d_rectangle(0, 1, 0.275, 1)));
    s3 = mathtree_add(s2, shapes2d_rectangle(0, 0.1, 0, 1));
    s4 = mathtree_add(s3, shapes2d_rectangle(0.1, 0.25, 0, 0.1));
    _glyphs['b'] = (struct glyph_t){ 
        .shape = s4,
        .width = 0.525
    };
    mathtree_free(s0);
    mathtree_free(s1);
    mathtree_free(s2);
    mathtree_free(s3);
}
static void
glyph_4(void) {
    static struct mathtree_t *s0, *s1, *s2, *s3, *s4, *s5;
    s0 = mathtree_sub(shapes2d_circle(0.3, 0.7, 0.3), shapes2d_circle(0.3, 0.7, 0.2));
    s1 = mathtree_add(s0, mathtree_sub(shapes2d_circle(0.3, 0.3, 0.3), shapes2d_circle(0.3, 0.3, 0.2)));
    s2 = mathtree_sub(s1, shapes2d_rectangle(0, 0.6, 0.3, 0.7));
    s3 = mathtree_sub(s2, shapes2d_triangle(0.3, 0.5, 1, 1.5, 1, -0.5));
    s4 = mathtree_sub(s3, shapes2d_rectangle(0.3, 0.6, 0.2, 0.8));
    s5 = mathtree_add(s4, shapes2d_rectangle(0, 0.1, 0.3, 0.7));
    _glyphs['C'] = (struct glyph_t){ 
        .shape = s5,
        .width = 0.57
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
    mathtree_free(s4); 
}
static void
glyph_5(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = shapes2d_circle(0.275, 0.275, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.275, 0.275, 0.175));
    s2 = mathtree_sub(s1, shapes2d_triangle(0.275, 0.275, 0.55, 0.55, 0.55, 0));
    _glyphs['c'] = (struct glyph_t){    
        .shape = s2,
        .width = 0.48
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
}
static void
glyph_6(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = mathtree_sub(shapes2d_circle(0.1, 0.5, 0.5), 
                shapes2d_circle(0.1, 0.5, 0.4));
    s1 = mathtree_and(s0, shapes2d_rectangle(0, 1, 0, 1));
    s2 = mathtree_add(s1, shapes2d_rectangle(0, 0.1, 0, 1));
    _glyphs['D'] = (struct glyph_t){      
        .shape = s2,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1);
}
static void
glyph_7(void) {
    struct mathtree_t *s0;
    s0 = shapes2d_reflect_x(_glyphs['b'].shape, _glyphs['b'].width/2);
    _glyphs['d'] = (struct glyph_t){    
        .shape = s0,
        .width = _glyphs['b'].width
    };
}
static void
glyph_8(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = shapes2d_rectangle(0, 0.1, 0, 1);
    s1 = mathtree_add(s0, shapes2d_rectangle(0.1, 0.6, 0.9, 1));
    s2 = mathtree_add(s1, shapes2d_rectangle(0.1, 0.6, 0, 0.1));
    s3 = mathtree_add(s2, shapes2d_rectangle(0.1, 0.5, 0.45, 0.55));
    _glyphs['E'] = (struct glyph_t){ 
        .shape = s3,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2); 
}
static void
glyph_9(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = shapes2d_circle(0.275, 0.275, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.275, 0.275, 0.175));
    s2 = mathtree_sub(s1, shapes2d_triangle(0.1, 0.275, 0.75, 0.275, 0.6, 0));
    s3 = mathtree_add(s2, shapes2d_rectangle(0.05, 0.55, 0.225, 0.315));
    s4 = mathtree_and(s3, shapes2d_circle(0.275, 0.275, 0.275));
    _glyphs['e'] = (struct glyph_t){        
        .shape = s4,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3);
}
static void
glyph_10(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = shapes2d_rectangle(0, 0.1, 0, 1);
    s1 = mathtree_add(s0, shapes2d_rectangle(0.1, 0.6, 0.9, 1));
    s2 = mathtree_add(s1, shapes2d_rectangle(0.1, 0.5, 0.45, 0.55));
    _glyphs['F'] = (struct glyph_t){        
        .shape = s2,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
}
static void
glyph_11(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = mathtree_sub(shapes2d_circle(0.4, 0.75, 0.25), 
            shapes2d_circle(0.4, 0.75, 0.15));
    s1 = mathtree_and(s0, shapes2d_rectangle(0, 0.4, 0.75, 1));
    s2 = mathtree_add(s1, shapes2d_rectangle(0, 0.4, 0.45, 0.55));
    s3 = mathtree_add(s2, shapes2d_rectangle(0.15, 0.25, 0, 0.75));
    _glyphs['f'] = (struct glyph_t){  
        .shape = s3,
        .width = 0.4
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2); 
}
static void
glyph_12(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = shapes2d_circle(0.275, -0.1, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.275, -0.1, 0.175));
    s2 = mathtree_and(s1, shapes2d_rectangle(0, 0.55, -0.375, -0.1));
    s3 = mathtree_add(s2, mathtree_sub(shapes2d_circle(0.275, 0.275, 0.275), 
                shapes2d_circle(0.275, 0.275, 0.175)));
    s4 = mathtree_add(s3, shapes2d_rectangle(0.45, 0.55, -0.1, 0.55));
    _glyphs['g'] = (struct glyph_t){  
        .shape = s4,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2); 
    mathtree_free(s3);  
}
static void
glyph_13(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4, *s5;
    s0 = mathtree_sub(shapes2d_circle(0.3, 0.7, 0.3),
            shapes2d_circle(0.3, 0.7, 0.2));
    s1 = mathtree_add(s0, mathtree_sub(shapes2d_circle(0.3, 0.3, 0.3),
            shapes2d_circle(0.3, 0.3, 0.2)));
    s2 = mathtree_sub(s1, shapes2d_rectangle(0, 0.6, 0.3, 0.7));
    s3 = mathtree_add(s2, shapes2d_rectangle(0, 0.1, 0.3, 0.7));
    s4 = mathtree_add(s3, shapes2d_rectangle(0.5, 0.6, 0.3, 0.4));
    s5 = mathtree_add(s4, shapes2d_rectangle(0.3, 0.6, 0.4, 0.5));
    _glyphs['G'] = (struct glyph_t){  
        .shape = s5,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
    mathtree_free(s4);
}
static void
glyph_14(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = shapes2d_rectangle(0, 0.1, 0, 1);
    s1 = mathtree_add(s0, shapes2d_rectangle(0.5, 0.6, 0, 1));
    s2 = mathtree_add(s1, shapes2d_rectangle(0.1, 0.5, 0.45, 0.55));
    _glyphs['H'] = (struct glyph_t){ 
        .shape = s2,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1);
}
static void
glyph_15(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = shapes2d_circle(0.275, 0.275, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.275, 0.275, 0.175));
    s2 = mathtree_and(s1, shapes2d_rectangle(0, 0.55, 0.275, 0.55));
    s3 = mathtree_add(s2, shapes2d_rectangle(0, 0.1, 0, 1));
    s4 = mathtree_add(s3, shapes2d_rectangle(0.45, 0.55, 0, 0.275));
    _glyphs['h'] = (struct glyph_t){   
        .shape = s4,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
}
static void
glyph_16(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = shapes2d_rectangle(0, 0.5, 0, 0.1);
    s1 = mathtree_add(s0, shapes2d_rectangle(0, 0.5, 0.9, 1));
    s2 = mathtree_add(s1, shapes2d_rectangle(0.2, 0.3, 0.1, 0.9));
    _glyphs['I'] = (struct glyph_t){    
        .shape = s2,
        .width = 0.5
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
}
static void
glyph_17(void) {
    struct mathtree_t *s0, *s1;
    s0 = shapes2d_rectangle(0.025, 0.125, 0, 0.55);
    s1 = mathtree_add(s0, shapes2d_circle(0.075, 0.7, 0.075));
    _glyphs['i'] = (struct glyph_t){    
        .shape = s1,
        .width = 0.15
    };
    mathtree_free(s0);  
}
static void
glyph_18(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = shapes2d_circle(0.275, 0.275, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.275, 0.275, 0.175));
    s2 = mathtree_and(s1, shapes2d_rectangle(0, 0.55, 0, 0.275));
    s3 = mathtree_add(s2, shapes2d_rectangle(0.45, 0.55, 0.275, 1));
    _glyphs['J'] = (struct glyph_t){   
        .shape = s3,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);
}
static void
glyph_19(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = shapes2d_circle(0.0, -0.1, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.0, -0.1, 0.175));
    s2 = mathtree_and(s1, shapes2d_rectangle(0, 0.55, -0.375, -0.1));
    s3 = mathtree_add(s2, shapes2d_rectangle(0.175, 0.275, -0.1, 0.55));
    s4 = mathtree_add(s3, shapes2d_circle(0.225, 0.7, 0.075));
    _glyphs['j'] = (struct glyph_t){ 
        .shape = s4,
        .width = 0.3
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
}
static void
glyph_20(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = shapes2d_rectangle(0, 0.6, 0, 1);
    s1 = mathtree_sub(s0, shapes2d_triangle(0.1, 1, 0.5, 1, 0.1, 0.6));
    s2 = mathtree_sub(s1, shapes2d_triangle(0.5, 0, 0.1, 0, 0.1, 0.4));
    s3 = mathtree_sub(s2, shapes2d_triangle(0.6, 0.95, 0.6, 0.05, 0.18, 0.5));
    _glyphs['K'] = (struct glyph_t){           
        .shape = s3,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2); 
}
static void
glyph_21(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = shapes2d_rectangle(0, 0.5, 0, 1);
    s1 = mathtree_sub(s0, shapes2d_triangle(0.1, 1, 0.5, 1, 0.1, 0.45));
    s2 = mathtree_sub(s1, shapes2d_triangle(0.36, 0, 0.1, 0, 0.1, 0.25));
    s3 = mathtree_sub(s2, shapes2d_triangle(0.6, 1, 0.5, 0.0, 0.18, 0.35));
    s4 = mathtree_sub(s3, shapes2d_triangle(0.1, 1, 0.6, 1, 0.6, 0.5));
    _glyphs['k'] = (struct glyph_t){           
        .shape = s4,
        .width = 0.5
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
}
static void
glyph_22(void) {
    struct mathtree_t *s0, *s1;
    s0 = shapes2d_rectangle(0, 0.6, 0, 0.1);
    s1 = mathtree_add(s0, shapes2d_rectangle(0, 0.1, 0, 1));
    _glyphs['L'] = (struct glyph_t){           
        .shape = s1,
        .width = 0.6
    };
    mathtree_free(s0); 
}
static void
glyph_23(void) {
    struct mathtree_t *s0;
    s0 = shapes2d_rectangle(0.025, 0.125, 0, 1);
    _glyphs['l'] = (struct glyph_t){           
        .shape = s0,
        .width = 0.15
    };
}
static void
glyph_24(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4, *s5;
    s0 = shapes2d_rectangle(0, 0.1, 0, 1);
    s1 = mathtree_add(s0, shapes2d_rectangle(0.7, 0.8, 0, 1));
    s2 = mathtree_add(s1, shapes2d_triangle(0, 1, 0.1, 1, 0.45, 0));
    s3 = mathtree_add(s2, shapes2d_triangle(0.45, 0, 0.35, 0, 0, 1));
    s4 = mathtree_add(s3, shapes2d_triangle(0.7, 1, 0.8, 1, 0.35, 0));
    s5 = mathtree_add(s4, shapes2d_triangle(0.35, 0, 0.8, 1, 0.45, 0));
    _glyphs['M'] = (struct glyph_t){          
        .shape = s5,
        .width = 0.8
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
    mathtree_free(s4);
}
static void
glyph_25(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4, *s5;
    s0 = mathtree_sub(shapes2d_circle(0.175, 0.35, 0.175),
            shapes2d_circle(0.175, 0.35, 0.075));
    s1 = mathtree_add(s0, mathtree_sub(shapes2d_circle(0.425, 0.35, 0.175), 
            shapes2d_circle(0.425, 0.35, 0.075)));
    s2 = mathtree_and(s1, shapes2d_rectangle(0, 0.65, 0.35, 0.65));
    s3 = mathtree_add(s2, shapes2d_rectangle(0, 0.1, 0, 0.525));
    s4 = mathtree_add(s3, shapes2d_rectangle(0.25, 0.35, 0, 0.35));
    s5 = mathtree_add(s4, shapes2d_rectangle(0.5, 0.6, 0, 0.35));
    _glyphs['m'] = (struct glyph_t){          
        .shape = s5,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
    mathtree_free(s4);
}
static void
glyph_26(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = shapes2d_rectangle(0, 0.1, 0, 1);
    s1 = mathtree_add(s0, shapes2d_rectangle(0.5, 0.6, 0, 1));
    s2 = mathtree_add(s1, shapes2d_triangle(0, 1, 0.1, 1, 0.6, 0));
    s3 = mathtree_add(s2, shapes2d_triangle(0.6, 0, 0.5, 0, 0, 1));
    _glyphs['N'] = (struct glyph_t){         
        .shape = s3,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2); 
}
static void
glyph_27(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = shapes2d_circle(0.275, 0.275, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.275, 0.275, 0.175));
    s2 = mathtree_and(s1, shapes2d_rectangle(0, 0.55, 0.325, 0.55));
    s3 = mathtree_add(s2, shapes2d_rectangle(0, 0.1, 0, 0.55));
    s4 = mathtree_add(s3, shapes2d_rectangle(0.45, 0.55, 0, 0.325));
    _glyphs['n'] = (struct glyph_t){          
        .shape = s4,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
}
static void
glyph_28(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = mathtree_sub(shapes2d_circle(0.3, 0.7, 0.3), 
            shapes2d_circle(0.3, 0.7, 0.2));
    s1 = mathtree_add(s0, mathtree_sub(shapes2d_circle(0.3, 0.3, 0.3),
            shapes2d_circle(0.3, 0.3, 0.2)));
    s2 = mathtree_sub(s1, shapes2d_rectangle(0, 0.6, 0.3, 0.7));
    s3 = mathtree_add(s2, shapes2d_rectangle(0, 0.1, 0.3, 0.7));
    s4 = mathtree_add(s3, shapes2d_rectangle(0.5, 0.6, 0.3, 0.7));
    _glyphs['O'] = (struct glyph_t){          
        .shape = s4,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
}
static void
glyph_29(void) {
    struct mathtree_t *s0, *s1;
    s0 = shapes2d_circle(0.275, 0.275, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.275, 0.275, 0.175));
    _glyphs['o'] = (struct glyph_t){         
        .shape = s1,
        .width = 0.55
    };
    mathtree_free(s0);  
}
static void
glyph_30(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4, *s5;
    s0 = shapes2d_circle(0.3, 0.725, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.3, 0.725, 0.175));
    s2 = mathtree_and(s1, shapes2d_rectangle(0.3, 1, 0, 1));
    s3 = mathtree_add(s2, shapes2d_rectangle(0, 0.1, 0, 1));
    s4 = mathtree_add(s3, shapes2d_rectangle(0.1, 0.3, 0.45, 0.55));
    s5 = mathtree_add(s4, shapes2d_rectangle(0.1, 0.3, 0.9, 1));
    _glyphs['P'] = (struct glyph_t){         
        .shape = s5,
        .width = 0.575
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
    mathtree_free(s4);
}
static void
glyph_31(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = shapes2d_circle(0.275, 0.275, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.275, 0.275, 0.175));
    s2 = mathtree_add(s1, shapes2d_rectangle(0, 0.1, -0.375, 0.55));
    _glyphs['p'] = (struct glyph_t){           
        .shape = s2,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
}
static void
glyph_32(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4, *s5, *s6;
    s0 = mathtree_sub(shapes2d_circle(0.3, 0.7, 0.3), shapes2d_circle(0.3, 0.7, 0.2));
    s1 = mathtree_add(s0, mathtree_sub(shapes2d_circle(0.3, 0.3, 0.3),shapes2d_circle(0.3, 0.3, 0.2)));
    s2 = mathtree_sub(s1, shapes2d_rectangle(0, 0.6, 0.3, 0.7));
    s3 = mathtree_add(s2, shapes2d_rectangle(0, 0.1, 0.3, 0.7));
    s4 = mathtree_add(s3, shapes2d_rectangle(0.5, 0.6, 0.3, 0.7));
    s5 = mathtree_add(s4, shapes2d_triangle(0.5, 0.1, 0.6, 0.1, 0.6, 0));
    s6 = mathtree_add(s5, shapes2d_triangle(0.5, 0.1, 0.5, 0.3, 0.6, 0.1));
    _glyphs['Q'] = (struct glyph_t){           
        .shape = s6,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
    mathtree_free(s4);
    mathtree_free(s5); 
}
static void
glyph_33(void) {
    struct mathtree_t *s0, *s1;
    s0 = mathtree_sub(shapes2d_circle(0.275, 0.275, 0.275),
        shapes2d_circle(0.275, 0.275, 0.175));
    s1 = mathtree_add(s0, shapes2d_rectangle(0.45, 0.55, -0.375, 0.55));
    _glyphs['q'] = (struct glyph_t){          
        .shape = s1,
        .width = 0.55
    };
    mathtree_free(s0);  
}
static void
glyph_34(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4, *s5, *s6, *s7;
    s0 = shapes2d_circle(0.3, 0.725, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.3, 0.725, 0.175));
    s2 = mathtree_and(s1, shapes2d_rectangle(0.3, 1, 0, 1));
    s3 = mathtree_add(s2, shapes2d_rectangle(0, 0.1, 0, 1));
    s4 = mathtree_add(s3, shapes2d_rectangle(0.1, 0.3, 0.45, 0.55));
    s5 = mathtree_add(s4, shapes2d_rectangle(0.1, 0.3, 0.9, 1));
    s6 = mathtree_add(s5, shapes2d_triangle(0.3, 0.5, 0.4, 0.5, 0.575, 0));
    s7 = mathtree_add(s6, shapes2d_triangle(0.475, 0.0, 0.3, 0.5, 0.575, 0));
    _glyphs['R'] = (struct glyph_t){          
        .shape = s7,
        .width = 0.575
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
    mathtree_free(s4);
    mathtree_free(s5);
    mathtree_free(s6);
}
static void
glyph_35(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = mathtree_sub(shapes2d_circle(0.55, 0, 0.55),
            shapes2d_scale_x(shapes2d_circle(0.55, 0, 0.45), 0.55, 0.8));
    s1 = mathtree_and(s0, shapes2d_rectangle(0, 0.55, 0, 0.55));
    s2 = shapes2d_scale_x(s1, 0, 0.7);
    s3 = mathtree_add(s2, shapes2d_rectangle(0, 0.1, 0, 0.55));
    _glyphs['r'] = (struct glyph_t){           
        .shape = s3,
        .width = 0.385
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2); 
}
static void
glyph_36(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = shapes2d_circle(0.275, 0.725, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.275, 0.725, 0.175));
    s2 = mathtree_sub(s1, shapes2d_rectangle(0.275, 0.55, 0.45, 0.725));
    s3 = mathtree_add(s2, shapes2d_reflect_x(shapes2d_reflect_y(s2, 0.5), .275));
    _glyphs['S'] = (struct glyph_t){         
        .shape = s3,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
}
static void
glyph_37(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = shapes2d_circle(0.1625, 0.1625, 0.1625);
    s1 = mathtree_sub(s0, shapes2d_scale_x(shapes2d_circle(0.165, 0.165, 0.0625), 0.165, 1.5));
    s2 = mathtree_sub(s1, shapes2d_rectangle(0, 0.1625, 0.1625, 0.325));
    s3 = mathtree_add(s2, shapes2d_reflect_x(shapes2d_reflect_y(s2, 0.275), 0.1625));
    s4 = shapes2d_scale_x(s3, 0, 1.5);
    _glyphs['s'] = (struct glyph_t){          
        .shape = s4,
        .width = 0.4875
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3);
}
static void
glyph_38(void) {
    struct mathtree_t *s0;
    s0 = mathtree_add(shapes2d_rectangle(0, 0.6, 0.9, 1), 
        shapes2d_rectangle(0.25, 0.35, 0, 0.9));
    _glyphs['T'] = (struct glyph_t){          
        .shape = s0,
        .width = 0.6
    };
}
static void
glyph_39(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = mathtree_sub(shapes2d_circle(0.4, 0.25, 0.25),
        shapes2d_circle(0.4, 0.25, 0.15));
    s1 = mathtree_and(s0, shapes2d_rectangle(0, 0.4, 0, 0.25));
    s2 = mathtree_add(s1, shapes2d_rectangle(0, 0.4, 0.55, 0.65));
    s3 = mathtree_add(s2, shapes2d_rectangle(0.15, 0.25, 0.25, 1));
    _glyphs['t'] = (struct glyph_t){  
        .shape = s3,
        .width = 0.4
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);
}
static void
glyph_40(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = mathtree_sub(shapes2d_circle(0.3, 0.3, 0.3),
             shapes2d_circle(0.3, 0.3, 0.2));
    s1 = mathtree_and(s0, shapes2d_rectangle(0, 0.6, 0, 0.3));
    s2 = mathtree_add(s1, shapes2d_rectangle(0, 0.1, 0.3, 1));
    s3 = mathtree_add(s2, shapes2d_rectangle(0.5, 0.6, 0.3, 1));
    _glyphs['U'] = (struct glyph_t){   
        .shape = s3,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);
}
static void
glyph_41(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = mathtree_sub(shapes2d_circle(0.275, 0.275, 0.275),
            shapes2d_circle(0.275, 0.275, 0.175));
    s1 = mathtree_and(s0, shapes2d_rectangle(0, 0.55, 0, 0.275));
    s2 = mathtree_add(s1, shapes2d_rectangle(0, 0.1, 0.275, 0.55));
    s3 = mathtree_add(s2, shapes2d_rectangle(0.45, 0.55, 0, 0.55));
    _glyphs['u'] = (struct glyph_t){ 
        .shape = s3,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2); 
}
static void
glyph_42(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = shapes2d_triangle(0, 1, 0.1, 1, 0.35, 0);
    s1 = mathtree_add(s0, shapes2d_triangle(0.35, 0, 0.25, 0, 0, 1));
    s2 = mathtree_add(s1, shapes2d_reflect_x(s1, 0.3));
    _glyphs['V'] = (struct glyph_t){ 
        .shape = s2,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
}
static void
glyph_43(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = shapes2d_triangle(0, 0.55, 0.1, 0.55, 0.35, 0);
    s1 = mathtree_add(s0, shapes2d_triangle(0.35, 0, 0.25, 0, 0, 0.55));
    s2 = mathtree_add(s1, shapes2d_reflect_x(s1, 0.3));
    _glyphs['v'] = (struct glyph_t){ 
        .shape = s2,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
}
static void
glyph_44(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = shapes2d_triangle(0, 1, 0.1, 1, 0.25, 0);
    s1 = mathtree_add(s0, shapes2d_triangle(0.25, 0, 0.15, 0, 0, 1));
    s2 = mathtree_add(s1, shapes2d_triangle(0.15, 0, 0.35, 1, 0.45, 1));
    s3 = mathtree_add(s2, shapes2d_triangle(0.45, 1, 0.25, 0, 0.15, 0));
    s4 = mathtree_add(s3, shapes2d_reflect_x(s3, 0.4));
    _glyphs['W'] = (struct glyph_t){ 
        .shape = s4,
        .width = 0.8
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
}
static void
glyph_45(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = shapes2d_triangle(0, 0.55, 0.1, 0.55, 0.25, 0);
    s1 = mathtree_add(s0, shapes2d_triangle(0.25, 0, 0.15, 0, 0, 0.55));
    s2 = mathtree_add(s1, shapes2d_triangle(0.15, 0, 0.35, 0.5, 0.45, 0.5));
    s3 = mathtree_add(s2, shapes2d_triangle(0.45, 0.5, 0.25, 0, 0.15, 0));
    s4 = mathtree_add(s3, shapes2d_reflect_x(s3, 0.4));
    _glyphs['w'] = (struct glyph_t){ 
        .shape = s4,
        .width = 0.8
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
}
static void
glyph_46(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = shapes2d_triangle(0, 1, 0.125, 1, 0.8, 0);
    s1 = mathtree_add(s0, shapes2d_triangle(0.8, 0, 0.675, 0, 0, 1));
    s2 = mathtree_add(s1, shapes2d_reflect_x(s1, 0.4));
    _glyphs['X'] = (struct glyph_t){ 
        .shape = s2,
        .width = 0.8
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
}
static void
glyph_47(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = shapes2d_triangle(0, 0.55, 0.125, 0.55, 0.55, 0);
    s1 = mathtree_add(s0, shapes2d_triangle(0.55, 0, 0.425, 0, 0, 0.55));
    s2 = mathtree_add(s1, shapes2d_reflect_x(s1, 0.275));
    _glyphs['x'] = (struct glyph_t){ 
        .shape = s2,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
}
static void
glyph_48(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = shapes2d_triangle(0, 1, 0.1, 1, 0.45, 0.5);
    s1 = mathtree_add(s0, shapes2d_triangle(0.45, 0.5, 0.35, 0.5, 0, 1));
    s2 = mathtree_add(s1, shapes2d_reflect_x(s1, 0.4));
    s3 = mathtree_add(s2, shapes2d_rectangle(0.35, 0.45, 0, 0.5));
    _glyphs['Y'] = (struct glyph_t){ 
        .shape = s3,
        .width = 0.8
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);
}
static void
glyph_49(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = shapes2d_triangle(0, 0.55, 0.1, 0.55, 0.325, 0);
    s1 = mathtree_add(s0, shapes2d_triangle(0.325, 0, 0.225, 0, 0, 0.55));
    s2 = mathtree_add(s1, mathtree_add(shapes2d_reflect_x(s1, 0.275), 
            shapes2d_move(shapes2d_reflect_x(s1, 0.275), -0.225, -0.55, 0)));
    s3 = mathtree_and(s2, shapes2d_rectangle(0, 0.55, -0.375, 0.55));
    _glyphs['y'] = (struct glyph_t){ 
        .shape = s3,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2); 
}
static void
glyph_50(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = shapes2d_rectangle(0, 0.6, 0, 1);
    s1 = mathtree_sub(s0, shapes2d_triangle(0, 0.1, 0, 0.9, 0.45, 0.9));
    s2 = mathtree_sub(s1, shapes2d_triangle(0.6, 0.1, 0.15, 0.1, 0.6, 0.9));
    _glyphs['Z'] = (struct glyph_t){ 
        .shape = s2,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
}
static void
glyph_51(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = shapes2d_rectangle(0, 0.6, 0, 0.55);
    s1 = mathtree_sub(s0, shapes2d_triangle(0, 0.1, 0, 0.45, 0.45, 0.45));
    s2 = mathtree_sub(s1, shapes2d_triangle(0.6, 0.1, 0.15, 0.1, 0.6, 0.45));
    _glyphs['z'] = (struct glyph_t){ 
        .shape = s2,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
}
static void
glyph_52(void) {
    struct mathtree_t *s0;
    s0 = mathtree_constant(1);
    s0->bounds = (struct bounds_t){ .xmin=0, .xmax=0, .ymin=0, .ymax=0, .zmin=NAN, .zmax=NAN };
    s0->shape = true;
    s0->bounds.xmin = 0, s0->bounds.xmax = 0.55
    ,s0->bounds.ymin = 0, s0->bounds.ymax = 1;
    _glyphs[' '] = (struct glyph_t){ 
        .shape = s0,
        .width = 0.55
    };
}
static void
glyph_53(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4;
    s0 = shapes2d_circle(0.075, 0.075, 0.075);
    s1 = shapes2d_scale_y(s0, 0.075, 3);
    s2 = mathtree_and(s1, shapes2d_rectangle(0.0, 0.15, -0.15, 0.075));
    s3 = mathtree_sub(s2, shapes2d_triangle(0.075, 0.075, 0.0, -0.15, -0.5, 0.075));
    s4 = mathtree_add(s3, shapes2d_circle(0.1, 0.075, 0.075));
    _glyphs[','] = (struct glyph_t){ 
        .shape = s4,
        .width = 0.175
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
}
static void
glyph_54(void) {
    struct mathtree_t *s0;
    s0 = shapes2d_circle(0.075, 0.075, 0.075);
    _glyphs['.'] = (struct glyph_t){ 
        .shape = s0,
        .width = 0.15
    };
}
static void
glyph_55(void) {
    struct mathtree_t *s0;
    s0 = shapes2d_rectangle(0, 0.1, 0.55, 0.8);
    _glyphs['\''] = (struct glyph_t){ 
        .shape = s0,
        .width = 0.1
    };
}
static void
glyph_56(void) {
    struct mathtree_t *s0;
    s0 = mathtree_add(shapes2d_rectangle(0, 0.1, 0.55, 0.8), 
                shapes2d_rectangle(0.2, 0.3, 0.55, 0.8));
    _glyphs['"'] = (struct glyph_t){ 
        .shape = s0,
        .width = 0.3
    };
}
static void
glyph_57(void) {
    struct mathtree_t *s0;
    s0 = mathtree_add(shapes2d_circle(0.075, 0.15, 0.075),
             shapes2d_circle(0.075, 0.45, 0.075));
    _glyphs[':'] = (struct glyph_t){ 
        .shape = s0,
        .width = 0.15
    };
}
static void
glyph_58(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4, *s5; 
    s0 = shapes2d_circle(0.075, 0.15, 0.075);
    s1 = shapes2d_scale_y(s0, 0.15, 3);
    s2 = mathtree_and(s1, shapes2d_rectangle(0.0, 0.15, -0.075, 0.15));
    s3 = mathtree_sub(s2, shapes2d_triangle(0.075, 0.15, 0.0, -0.075, -0.5, 0.15));
    s4 = mathtree_add(s3, shapes2d_circle(0.075, 0.45, 0.075));
    s5 = mathtree_add(s4, shapes2d_circle(0.1, 0.15, 0.075));
    _glyphs[';'] = (struct glyph_t){ 
        .shape = s5,
        .width = 0.15
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
    mathtree_free(s4); 
}
static void
glyph_59(void) {
    struct mathtree_t *s0, *s1;
    s0 = shapes2d_rectangle(0.025, 0.125, 0.3, 1);
    s1 = mathtree_add(s0, shapes2d_circle(0.075, 0.075, 0.075));
    _glyphs['!'] = (struct glyph_t){ 
        .shape = s1,
        .width = 0.1
    };
    mathtree_free(s0);
}
static void
glyph_60(void) {
    struct mathtree_t *s0;
    s0 = shapes2d_rectangle(0.05, 0.4, 0.35, 0.45);
    _glyphs['-'] = (struct glyph_t){ 
        .shape = s0,
        .width = 0.45
    };
}
static void
glyph_61(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = mathtree_sub(shapes2d_circle(0, 0.4, 0.6), 
        shapes2d_scale_x(shapes2d_circle(0, 0.4, 0.5), 0, 0.7));
    s1 = mathtree_and(s0, shapes2d_rectangle(0, 0.6, -0.2, 1));
    s2 = shapes2d_scale_x(s1, 0, 1/2.);
    _glyphs[')'] = (struct glyph_t){ 
        .shape = s2,
        .width = 0.3
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
}
static void
glyph_62(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = mathtree_sub(shapes2d_circle(0.6, 0.4, 0.6), 
        shapes2d_scale_x(shapes2d_circle(0.6, 0.4, 0.5), 0.6, 0.7));
    s1 = mathtree_and(s0, shapes2d_rectangle(0, 0.6, -0.2, 1));
    s2 = shapes2d_scale_x(s1, 0, 1/2.);
    _glyphs['('] = (struct glyph_t){ 
        .shape = s2,
        .width = 0.3
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
}
static void
glyph_63(void) {
    struct mathtree_t *s0, *s1, *s2;  
    s0 = shapes2d_rectangle(0, 0.3, 0, 1);
    s1 = mathtree_sub(s0, shapes2d_circle(0, 1, 0.2));
    s2 = mathtree_sub(s1, shapes2d_rectangle(0, 0.2, 0, 0.7));
    _glyphs['1'] = (struct glyph_t){ 
        .shape = s2,
        .width = 0.3
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
}
static void
glyph_64(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4, *s5;  
    s0 = shapes2d_circle(0.275, .725, .275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.275, 0.725, 0.175));
    s2 = mathtree_sub(s1, shapes2d_rectangle(0, 0.55, 0, 0.725));
    s3 = mathtree_add(s2, shapes2d_rectangle(0, 0.55, 0, 0.1));
    s4 = mathtree_add(s3, shapes2d_triangle(0, 0.1, 0.45, 0.775, 0.55, 0.725));
    s5 = mathtree_add(s4, shapes2d_triangle(0, 0.1, 0.55, 0.725, 0.125, 0.1));
    _glyphs['2'] = (struct glyph_t){ 
        .shape = s5,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
    mathtree_free(s4); 
}
static void
glyph_65(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4;  
    s0 = shapes2d_circle(0.3, 0.725, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.3, 0.725, 0.175));
    s2 = mathtree_add(s1, shapes2d_circle(0.3, 0.275, 0.275));
    s3 = mathtree_sub(s2, shapes2d_circle(0.3, 0.275, 0.175));
    s4 = mathtree_sub(s3, shapes2d_rectangle(0, 0.275, 0.275, 0.725));
    _glyphs['3'] = (struct glyph_t){ 
        .shape = s4,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
}
static void
glyph_66(void) {
    struct mathtree_t *s0, *s1, *s2, *s3; 
    s0 = shapes2d_triangle(-0.10, 0.45, 0.4, 1, 0.4, 0.45);
    s1 = mathtree_add(s0, shapes2d_rectangle(0.4, 0.5, 0, 1));
    s2 = mathtree_sub(s1, shapes2d_triangle(0.4, 0.85, 0.4, 0.55, 0.1, 0.55));
    s3 = mathtree_and(s2, shapes2d_rectangle(0, 0.5, 0, 1));
    _glyphs['4'] = (struct glyph_t){ 
        .shape = s3,
        .width = 0.5
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
}
static void
glyph_67(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, *s4;    
    s0 = mathtree_sub(shapes2d_circle(0.325, 0.325, 0.325), shapes2d_circle(0.325, 0.325, 0.225));
    s1 = mathtree_sub(s0, shapes2d_rectangle(0, 0.325, 0.325, 0.65));
    s2 = mathtree_add(s1, shapes2d_rectangle(0, 0.325, 0.55, 0.65));
    s3 = mathtree_add(s2, shapes2d_rectangle(0, 0.1, 0.55, 1));
    s4 = mathtree_add(s3, shapes2d_rectangle(0.1, 0.65, 0.9, 1));
    _glyphs['5'] = (struct glyph_t){ 
        .shape = s4,
        .width = 0.65
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
}
static void
glyph_68(void) {
    struct mathtree_t *s0, *s1, *s2, *s3, 
        *s4, *s5, *s6, *s7;
    s0 = mathtree_sub(shapes2d_circle(0.275, 0.725, 0.275), 
                shapes2d_scale_y(shapes2d_circle(0.275, 0.725, 0.175), .725, 1.2));
    s1 = mathtree_and(s0, shapes2d_rectangle(0, 0.55, 0.725, 1));
    s2 = mathtree_sub(s1, shapes2d_triangle(0.275, 0.925, 0.55, 0.9, 0.55, 0.725));
    s3 = shapes2d_scale_y(s2, 1, 2);
    s4 = shapes2d_scale_x(s3, 0, 1.1);
    s5 = mathtree_sub(s4, shapes2d_rectangle(0.275, 0.65, 0., 0.7));
    s6 = mathtree_add(s5, shapes2d_rectangle(0, 0.1, 0.275, 0.45));
    s7 = mathtree_add(s6, mathtree_sub(shapes2d_circle(0.275, 0.275, 0.275), 
                shapes2d_circle(0.275, 0.275, 0.175)));
    _glyphs['6'] = (struct glyph_t){ 
        .shape = s7,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1); 
    mathtree_free(s2);  
    mathtree_free(s3); 
    mathtree_free(s4);  
    mathtree_free(s5);
    mathtree_free(s6);     
}
static void
glyph_69(void) {
    struct mathtree_t *s0, *s1, *s2;
    s0 = shapes2d_rectangle(0, 0.6, 0.9, 1);
    s1 = mathtree_add(s0, shapes2d_triangle(0, 0, 0.475, 0.9, 0.6, 0.9));
    s2 = mathtree_add(s1, shapes2d_triangle(0, 0, 0.6, 0.9, 0.125, 0));
    _glyphs['7'] = (struct glyph_t){ 
        .shape = s2,
        .width = 0.6
    };
    mathtree_free(s0);  
    mathtree_free(s1);      
}
static void
glyph_70(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = shapes2d_circle(0.3, 0.725, 0.275);
    s1 = mathtree_sub(s0, shapes2d_circle(0.3, 0.725, 0.175));
    s2 = mathtree_add(s1, shapes2d_circle(0.3, 0.275, 0.275));
    s3 = mathtree_sub(s2, shapes2d_circle(0.3, 0.275, 0.175));
    _glyphs['8'] = (struct glyph_t){ 
        .shape = s3,
        .width = 0.55
    };
    mathtree_free(s0);  
    mathtree_free(s1);   
    mathtree_free(s2); 
}
static void
glyph_71(void) {
    struct mathtree_t *s0;
    s0 = shapes2d_reflect_x(shapes2d_reflect_y(_glyphs['6'].shape, 0.5), _glyphs['6'].width/2);
    _glyphs['9'] = (struct glyph_t){ 
        .shape = s0,
        .width = _glyphs['6'].width
    };
}
static void
glyph_72(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = shapes2d_circle(0.5, 0.5, 0.4);
    s1 = shapes2d_scale_x(s0, 0.5, powf(0.7,0.5));
    s2 = mathtree_sub(shapes2d_circle(0.5, 0.5, 0.5), s1);
    s3 = shapes2d_scale_x(s2, 0, 0.7);
    _glyphs['0'] = (struct glyph_t){ 
        .shape = s3,
        .width = 0.7
    };
    mathtree_free(s0);  
    mathtree_free(s1);   
    mathtree_free(s2);   
}
static void
glyph_73(void) {
    struct mathtree_t *s0, *s1;
    s0 = shapes2d_rectangle(0., 0.5, 0.45, 0.55);
    s1 = mathtree_add(s0, shapes2d_rectangle(0.2, 0.3, 0.25, 0.75));
    _glyphs['+'] = (struct glyph_t){ 
        .shape = s1,
        .width = 0.55
    };
    mathtree_free(s0);    
}
static void
glyph_74(void) {
    struct mathtree_t *s0, *s1;
    s0 = shapes2d_triangle(0, 0, 0.425, 1, 0.55, 1);
    s1 = mathtree_add(s0, shapes2d_triangle(0, 0, 0.55, 1, 0.125, 0));
    _glyphs['/'] = (struct glyph_t){ 
        .shape = s1,
        .width = 0.55
    };
    mathtree_free(s0);
}
static void
glyph_75(void) {
    struct mathtree_t *s0, *s1, *s2, *s3;
    s0 = mathtree_sub(shapes2d_circle(0.275, 0.725, 0.275), shapes2d_circle(0.275, 0.725, 0.175));
    s1 = mathtree_sub(s0, shapes2d_rectangle(0, 0.275, 0.45, 0.725));
    s2 = mathtree_add(s1, shapes2d_rectangle(0.225, 0.325, 0.3, 0.55));
    s3 = mathtree_add(s2, shapes2d_circle(0.275, 0.075, 0.075));
    _glyphs['?'] = (struct glyph_t){ 
        .shape = s3,
        .width = 0.525
    };
    mathtree_free(s0);
    mathtree_free(s1);
    mathtree_free(s2);
}
struct glyph_funcs_t {
    void (*glyph)(void);
} glyph_funcs[77] = {
    { glyph_0 },
    { glyph_1 },
    { glyph_2 },
    { glyph_3 },
    { glyph_4 },
    { glyph_5 },
    { glyph_6 },
    { glyph_7 },
    { glyph_8 },
    { glyph_9 },
    { glyph_10 },
    { glyph_11 },
    { glyph_12 },
    { glyph_13 },
    { glyph_14 },
    { glyph_15 },
    { glyph_16 },
    { glyph_17 },
    { glyph_18 },
    { glyph_19 },
    { glyph_20 },
    { glyph_21 },
    { glyph_22 },
    { glyph_23 },
    { glyph_24 },
    { glyph_25 },
    { glyph_26 },
    { glyph_27 },
    { glyph_28 },
    { glyph_29 },
    { glyph_30 },
    { glyph_31 },
    { glyph_32 },
    { glyph_33 },
    { glyph_34 },
    { glyph_35 },
    { glyph_36 },
    { glyph_37 },
    { glyph_38 },
    { glyph_39 },
    { glyph_40 },
    { glyph_41 },
    { glyph_42 },
    { glyph_43 },
    { glyph_44 },
    { glyph_45 },
    { glyph_46 },
    { glyph_47 },
    { glyph_48 },
    { glyph_49 },
    { glyph_50 },
    { glyph_51 },
    { glyph_52 },
    { glyph_53 },
    { glyph_54 },
    { glyph_55 },
    { glyph_56 },
    { glyph_57 },
    { glyph_58 },
    { glyph_59 },
    { glyph_60 },
    { glyph_61 },
    { glyph_62 },
    { glyph_63 },
    { glyph_64 },
    { glyph_65 },
    { glyph_66 },
    { glyph_67 },
    { glyph_68 },
    { glyph_69 },
    { glyph_70 },
    { glyph_71 },
    { glyph_72 },
    { glyph_73 },
    { glyph_74 },
    { glyph_75 },
    { NULL }
};
static struct mathtree_t *
koko_text(const char *text, float x, float y, int height, const char *align) {
    int c;
    float dx = 0, dy = -1;
    height = (height <= 0) ? 1 : height;
    struct mathtree_t *text_shape = NULL, *line_shape = NULL;
    for(int i=0; i<strlen(text)+1; ++i) {
        if(((c = text[i]) != '\n') && c != '\0') {
            if(_glyphs[c].shape) {
                struct mathtree_t *chr_math = 
                    shapes2d_move(_glyphs[c].shape, dx, dy, 0);
                line_shape = (line_shape == NULL) ? chr_math:
                    mathtree_add(line_shape, chr_math);
                dx += _glyphs[c].width + 0.1;
            }
            continue;
        }
        dx -= 0.1;
        if (line_shape != NULL) {
            if (align[0] == 'L') {  
            } else if (align[0] == 'C') {
                line_shape = shapes2d_move(line_shape, -dx/2, 0, 0);
            } else if (align[0] == 'R') {
                line_shape = shapes2d_move(line_shape, -dx, 0, 0);
            }
            text_shape = (!text_shape) ? line_shape : mathtree_add(text_shape, line_shape);
        }
        dy -= 1.55;
        dx = 0;
        line_shape = NULL;
    }
    dy += 1.55;
    if (!text_shape) return NULL;
    if (align[1] == 'T') {
    } else if (align[1] == 'B') {
        text_shape = shapes2d_move(text_shape, 0, -dy, 0);
    } else if (align[1] == 'C') {
        text_shape = shapes2d_move(text_shape, 0, -dy/2, 0);
    }
    if (height != 1) {
        text_shape = shapes2d_scale_xy(text_shape, 0, 0, height);
        dx *= height;
        dy *= height;
    }
    return shapes2d_move(text_shape, x, y, 0);
}
static int 
lua_text(lua_State *L) {
    struct mathtree_t **t = lua_newuserdata(L, sizeof(struct mathtree_t *));
    luaL_getmetatable(L, MATHTREE_MT);
    lua_setmetatable(L, -2);
    struct mathtree_t *__t = koko_text(
           luaL_checkstring(L, 1)
         , luaL_checknumber(L, 2)
         , luaL_checknumber(L, 3)
         , luaL_checknumber(L, 4)
         , luaL_checkstring(L, 5));
    *t = __t;
    return 1;
}
LUALIB_API int 
luaopen_text(lua_State * L) {
    for(int i=0; glyph_funcs[i].glyph != NULL; i++)
        glyph_funcs[i].glyph();
    lua_register(L, "text", lua_text);
    return 0;
}
