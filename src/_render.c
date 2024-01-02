 #include "_render.h"

 struct image_t *
 make_flat_image(struct fabvars_t *cad, struct mathtree_t *t, float scale) {
    float dx = mathtree_dx(t);
    float dy = mathtree_dy(t);
    struct region_t r = region_create(
        t->bounds.xmin-cad->border*dx, t->bounds.ymin-cad->border*dy, 0,
        t->bounds.xmax+cad->border*dx, t->bounds.ymax+cad->border*dy, 0,
    scale, false, NAN);
    struct image_t *img = mathtree_render(t, r, scale, cad->mm_per_unit, 8, 0);
    img->color = t->color;
    region_free(r);
    return img;
 }
struct image_t *
 make_image(struct fabvars_t *cad, struct mathtree_t *t, float zmin, float zmax, float scale) {
    float xmin = cad->bounds.xmin = 0;
    float xmax = cad->bounds.xmax = 1;
    float ymin = cad->bounds.ymin = -1;
    float ymax = cad->bounds.ymax = 1;
    float dx = mathtree_dx(t);
    float dy = mathtree_dy(t);
    if (isnormal(t->bounds.xmin))
        xmin = fmax(xmin, t->bounds.xmin - cad->border*dx);
    if (isnormal(t->bounds.xmax))
        xmax = fmin(xmax, t->bounds.xmax - cad->border*dx);
    if (isnormal(t->bounds.ymin))
        ymin = fmax(ymin, t->bounds.ymin - cad->border*dy);
    if (isnormal(t->bounds.ymax))
        ymax = fmin(ymax, t->bounds.ymax - cad->border*dy);
    struct region_t r = region_create(
        xmin, ymin, zmin, xmax, ymax, zmax, scale, false, NAN);
    struct image_t *img = mathtree_render(t, r, scale, cad->mm_per_unit, 8, 0);
    img->color = t->color;
    region_free(r);
    return img;
}
void
make_images(struct fabvars_t *cad, vec_image_t *images, float scale) {
    int i = 0;
    struct mathtree_t *t = NULL;
    float zmin = isnormal(cad->bounds.zmin) ? cad->bounds.zmin : 0;
    float zmax = isnormal(cad->bounds.zmax) ? cad->bounds.zmax : 0;
    vec_foreach(&cad->_shapes, t, i) {
        (void)vec_push(images, make_image(cad, t, zmin, zmax, scale));
    }
}

struct mesh_t *
make_mesh(struct fabvars_t *cad, struct mathtree_t *t) {
    int depth = 0;
    struct mesh_t *mesh = NULL;
    while (depth <= 4) {
        struct region_t r = region_create (
            t->bounds.xmin - cad->border*mathtree_dx(t),
            t->bounds.ymin - cad->border*mathtree_dy(t),
            t->bounds.zmin - cad->border*mathtree_dz(t),
            t->bounds.xmax + cad->border*mathtree_dx(t),
            t->bounds.ymax + cad->border*mathtree_dy(t),
            t->bounds.zmax + cad->border*mathtree_dz(t),
            100.00,
            false,
            depth
        );
        struct asdf_t *a = mathtree_asdf(t, r, 0, cad->mm_per_unit, true);
        mesh = asdf_triangulate(a, 0);
        asdf_free(a);
        if (mesh_vcount(mesh)) 
            break;
        if(depth < 4)
            mesh_free(mesh);
        region_free(r);
        depth+=1;
    }
    mesh->source = (struct source_t) {
        .type = MESH_MATHTREE,
        .expr = mathtree_clone(t),
        .depth = depth,
        .scale = cad->mm_per_unit
    };
    return mesh;
 }