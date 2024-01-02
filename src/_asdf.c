#include "_asdf.h"

struct asdf_t *
asdf_create(struct ASDF_ *ptr, bool free, Vec3rgb color) {
    struct asdf_t *a =  calloc(1, sizeof(struct asdf_t));
    a->ptr = ptr;
    a->filename = NULL;
    a->color = (color.is_set) ? color : Vec3rgb_nil;
    a->free = free;
    pthread_mutex_init (&a->lock, NULL);
    return a;
}
struct Interval_ asdf_X(struct asdf_t *a) {return a->ptr->X;}
struct Interval_ asdf_Y(struct asdf_t *a) {return a->ptr->Y;}
struct Interval_ asdf_Z(struct asdf_t *a) {return a->ptr->Z;}
float 
__asdf_interpolate(struct asdf_t *a, 
                float x, float y, float z) {
    struct Interval_ X, Y, Z;
    X = asdf_X(a);
    Y = asdf_Y(a);
    Z = asdf_Z(a);
    return interpolate(
        a->ptr->d, x, y, z,
        X.lower, Y.lower, Z.lower,
        X.upper - X.lower,
        Y.lower - Y.upper,
        Z.lower - Z.upper
    );
}
enum ASDFstate asdf_state(struct asdf_t *a) { return a->ptr->state;}
float *asdf_d(struct asdf_t *a){return a->ptr->d;}
float asdf_xmin(struct asdf_t *a) {return a->ptr->X.lower;}
float asdf_xmax(struct asdf_t *a){return a->ptr->X.upper;}
float asdf_dx(struct asdf_t *a){return asdf_xmax(a) - asdf_xmin(a);}
float asdf_ymin(struct asdf_t *a){return a->ptr->Y.lower;}
float asdf_ymax(struct asdf_t *a){return a->ptr->Y.upper;}
float asdf_dy(struct asdf_t *a){return asdf_ymax(a) - asdf_ymin(a);}
float asdf_zmin(struct asdf_t *a){return a->ptr->Z.lower;}
float asdf_zmax(struct asdf_t *a){return a->ptr->Z.upper;}
float asdf_dz(struct asdf_t *a){return asdf_zmax(a) - asdf_zmin(a);}
int asdf_mm_per_unit(struct asdf_t *a) {return 1; }
void asdf_rescale(struct asdf_t *a, const float scale) { asdf_scale(a->ptr, scale);}
int asdf_depth(struct asdf_t *a) { return get_depth(a->ptr); }
void asdf_dimensions(struct asdf_t *a, int* ni, int* nj, int* nk) {find_dimensions(a->ptr, ni, nj, nk);}
int asdf_cell_count(struct asdf_t *a) {return count_cells(a->ptr);}
int asdf_ram(struct asdf_t *a) {return (asdf_cell_count(a) * sizeof(struct ASDF_));}
void asdf_save(struct asdf_t *a, const char *filename) {asdf_write(a->ptr, filename);}
struct asdf_t *
asdf_load(const char *filename) {
    struct asdf_t *a = asdf_create(asdf_read(filename), true, Vec3rgb_nil);
    a->filename = filename;
    return a;
}
struct asdf_t *
asdf_from_pixels(struct image_t *i, int offset, bool merge_leafs) {
    image_pixels(i);
    struct ASDF_ *asdf = import_lattice((const float **)i->pixels, i->width, i->height,
            offset, 1/image_pixels_per_mm(i), merge_leafs);
    return asdf_create(asdf, true, Vec3rgb_nil);
}
struct asdf_triangulate_context_t {
    pt_thread_t pt_thread;
    pt_func_t pt_func;
    int halt;
    struct asdf_t *asdf;
    mesh_ptrs_vec_t *ptrs;
};
static pt_t
asdf_triangulate_thr(void * const ctx) {
    struct asdf_triangulate_context_t *arg = ctx;
    struct Mesh_ *mesh = triangulate(arg->asdf->ptr, &arg->halt);
    (void)vec_push(arg->ptrs, mesh); 
    free(arg);
    return PT_DONE;
}
struct mesh_t *
asdf_triangulate(struct asdf_t *asdf, int threads) {
    int halt = 0;
    mesh_ptrs_vec_t ptrs;
    vec_init(&ptrs);
    if(threads > 0) { printf("threads%d\n", threads);
        protothread_t const pt = protothread_create_maxpt(threads);
        for(int i=0; i<threads; i++) {
            struct asdf_triangulate_context_t *arg = 
                    calloc(1, sizeof(struct asdf_triangulate_context_t));
            arg->asdf = asdf;
            arg->halt = halt;
            arg->ptrs = &ptrs;
            pt_create(pt, &arg->pt_thread, asdf_triangulate_thr, arg);
        }
        protothread_quiesce(pt);
        protothread_free(pt);
    } else (void)vec_push(&ptrs, triangulate(asdf->ptr, &halt));
    struct mesh_t *m = mesh_merge(&ptrs);
    int i = 0;
    struct Mesh_ *ptr = NULL;
    vec_foreach(&ptrs, ptr, i) 
        { free_mesh(ptr); } vec_deinit(&ptrs);
    vec_compact(&ptrs);
    return m;
}
void
asdf_free(struct asdf_t *a) {
    if((a->free == true) && a->ptr)
        free_asdf(a->ptr);
}