#include "_mesh.h"

struct mesh_t *
mesh_create(struct Mesh_ *ptr, Vec3rgb color) { 
    struct mesh_t *m = calloc(1, sizeof(struct mesh_t));
    m->ptr = ptr;
    m->cached = NULL;
    m->color = (color.is_set) ? color : Vec3rgb_nil;
    vec_init(&m->children);
    return m;
}
void
mesh_free(struct mesh_t *m) { 
    if(m != NULL) {
        if(m->ptr)
            free_mesh(m->ptr);
        if(m->source.expr)
            mathtree_free(m->source.expr);
        vec_deinit(&m->children);
        vec_compact(&m->children);
        free(m);
    }
}
struct Interval_ 
mesh_X(struct mesh_t *m) { 
    if (m->ptr) return m->ptr->X;
    else {
        struct Mesh_ *mesh = vec_first(&m->children)->ptr;
        float lower = mesh->X.lower;
        float upper = mesh->X.upper;
        for(int i=1; i<m->children.length; i++) {
            mesh = m->children.data[i]->ptr;
            lower = fmin(mesh->X.lower, lower);
            upper = fmax(mesh->X.upper, upper);
	    }
        return (struct Interval_) {
            .lower = lower,
            .upper = upper 
        };
    }   
}
struct Interval_ 
mesh_Y(struct mesh_t *m) {
    if (m->ptr) return m->ptr->Y;
    else {
        struct Mesh_ *mesh = vec_first(&m->children)->ptr;
        float lower = mesh->Y.lower;
        float upper = mesh->Y.upper;
        for(int i=1; i<m->children.length; i++) {
            mesh = m->children.data[i]->ptr;
            lower = fmin(mesh->Y.lower, lower);
            upper = fmax(mesh->Y.upper, upper);
	    }
        return (struct Interval_) {
            .lower = lower,
            .upper = upper 
        };
    }   
}
struct Interval_
mesh_Z(struct mesh_t *m) {
    if (m->ptr) return m->ptr->Z;
    else {
        struct Mesh_ *mesh = vec_first(&m->children)->ptr;
        float lower = mesh->Z.lower;
        float upper = mesh->Z.upper;
        for(int i=1; i<m->children.length; i++) {
            mesh = m->children.data[i]->ptr;
            lower = fmin(mesh->Z.lower, lower);
            upper = fmax(mesh->Z.upper, upper);
	    }
        return (struct Interval_) {
            .lower = lower,
            .upper = upper 
        };
    }   
}
uint32_t 
mesh_tcount(struct mesh_t *m) { 
    return m->ptr ? m->ptr->tcount : 0; 
}
uint32_t 
mesh_vcount(struct mesh_t *m) { 
    return m->ptr ? m->ptr->vcount : 0; 
}
float *
mesh_vdata(struct mesh_t *m) { 
    return m->ptr ? m->ptr->vdata : NULL; 
}
uint32_t  *
mesh_tdata(struct mesh_t *m) { 
    return m->ptr ? m->ptr->tdata : NULL; 
}
void 
mesh_save_stl(struct mesh_t *m, const char *filename) {
    save_stl(m->ptr, filename);
}
void 
mesh_save(struct mesh_t *m, const char *filename) {
    save_mesh(filename, m->ptr);
}
void
mesh_leafs(struct mesh_t *m, mesh_ptrs_vec_t *leafs) {
    int i = 0;
    struct mesh_t *c = NULL;
    if(m->children.length > 0) {
        vec_foreach(&m->children, c, i) {
            mesh_leafs(c, leafs);
        }
    } else (void)vec_push(leafs, m->ptr);
}
void 
mesh_refine_math(struct mesh_t *m) {
    struct Interval_ X, Y, Z;
    X = mesh_X(m);
    Y = mesh_Y(m);
    Z = mesh_Z(m);
    float scale = m->source.scale;
    int depth = m->source.depth;
    struct region_t r = region_create (
            X.lower / scale, Y.lower / scale, Z.lower / scale,
            X.upper / scale, Y.upper / scale, Z.upper / scale,
            100.00,
            false,
            depth+1
        );
    struct Region_ subregions[2];
    split(r.r, subregions, 2);
    for( int i=0; i<2; i++) {
        struct asdf_t *asdf = mathtree_asdf(m->source.expr, 
            (struct region_t) { .r = subregions[i], .free_arrays = false, .is_set = true }, 0, m->source.scale, true);
        struct mesh_t *mesh = mathtree_triangulate(asdf);
        mesh->source = (struct source_t) {
            .type = MESH_MATHTREE,
            .expr = mathtree_clone(m->source.expr),
            .depth = m->source.depth+1,
            .scale = m->source.scale
        };
        (void)vec_push(&m->children, mesh);
    }
    free_mesh(m->ptr);
    region_free(r);
    m->ptr = NULL;
}
void 
mesh_refine_asdf(struct mesh_t *m) {
    for(int i =0; i <= 8; i++) {
        const char *filename = m->source.file;
        struct asdf_t *asdf = asdf_load(filename);
        struct mesh_t *mesh = mathtree_triangulate(asdf);
        mesh->source = (struct source_t) {
            .type = MESH_ASDF,
            .file = filename,
            .depth = m->source.depth+1
        };
        (void)vec_push(&m->children, mesh);
    }
    free_mesh(m->ptr);
    m->ptr = NULL;
}
void 
mesh_refine(struct mesh_t *m) {
    if (m->cached == NULL) {
        m->cached = "./cached_file";
        mesh_save(m, m->cached);
    }
    if (m->source.type == MESH_MATHTREE) {
        mesh_refine_math(m);
    } else if (m->source.type == MESH_ASDF) { 
       mesh_refine_asdf(m);
    }
}
struct mesh_t *
mesh_collapse_tree(struct mesh_t *m) {
    struct Interval_ X, Y, Z;
    X = mesh_X(m);
    Y = mesh_Y(m);
    Z = mesh_Z(m);
    float scale = m->source.scale;
    int depth = m->source.depth;
    struct region_t r = region_create (
            X.lower / scale, Y.lower / scale, Z.lower / scale,
            X.upper / scale, Y.upper / scale, Z.upper / scale,
            100.00,
            false,
            depth
        );
    struct asdf_t *asdf = mathtree_asdf(m->source.expr,
                r, 0,  m->source.scale, true);
    region_free(r);
    return mathtree_triangulate(asdf);
}
struct mesh_t *
mesh_merge(mesh_ptrs_vec_t *ptrs) {
    return mesh_create(merge_meshes(ptrs->length, (const struct Mesh_ **)ptrs->data), Vec3rgb_nil);
}