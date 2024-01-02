#include "_mathtree.h"

#include "sds.h"

void 
mathtree_free(struct mathtree_t *t) {
    if(t != NULL) {
        if(t->_ptr)
            free_tree(t->_ptr);
        if(t->math)
            sdsfree(t->math);
        free(t);
    }
}
struct mathtree_t * 
mathtree_create(const char *math, bool shape, const Vec3rgb color) {
    struct mathtree_t *t = calloc(1, sizeof(struct mathtree_t ));
    t->math = sdsnew(math);
    t->_str = NULL;
    t->_ptr = NULL;
    t->shape = shape;
    t->bounds.xmin = t->bounds.ymin = t->bounds.zmin 
        = t->bounds.xmax = t->bounds.ymax = t->bounds.zmax = NAN;
    t->color = (color.is_set) ? color : Vec3rgb_nil;
    return t;
} 
struct mathtree_t * 
mathtree_constant(float f) {   
    return mathtree_create(
        sdscatprintf(sdsempty(),"f%g", f), false, Vec3rgb_nil); 
}
struct mathtree_t * 
mathtree_X(void) {
    return mathtree_create("X", false, Vec3rgb_nil); 
}
struct mathtree_t * 
mathtree_Y(void) {
    return mathtree_create("Y", false, Vec3rgb_nil);
}
struct mathtree_t * 
mathtree_Z(void) {
    return mathtree_create("Z", false, Vec3rgb_nil);
}
float
mathtree_dx(struct mathtree_t *t) {
    return t->bounds.xmax - t->bounds.xmin;
}
float
mathtree_dy(struct mathtree_t *t) {
    return t->bounds.ymax - t->bounds.ymin;
}
float
mathtree_dz(struct mathtree_t *t) {
    return t->bounds.zmax - t->bounds.zmin;
}
struct mathtree_t * 
mathtree_min(const char *a, const char *b) {
    return mathtree_create(
        sdscatprintf(sdsnew("i"), "%s%s", a, b), false, Vec3rgb_nil);
}
struct mathtree_t * 
mathtree_max(const char *a, const char *b) {
    return mathtree_create(
        sdscatprintf(sdsnew("a"), "%s%s", a, b), false, Vec3rgb_nil);
}
struct mathtree_t * 
mathtree_pow(const char *a, const char *b) {
    return mathtree_create(
        sdscatprintf(sdsnew("p"), "%s%s", a, b), false, Vec3rgb_nil);
}
struct mathtree_t * 
mathtree_sqrt(struct mathtree_t *t) {
    return mathtree_create(
        sdscat(sdsnew("r"), t->math), false, Vec3rgb_nil);
}
struct mathtree_t * 
mathtree_abs(struct mathtree_t *t) {
    return mathtree_create(
        sdscat(sdsnew("b"), t->math), false, Vec3rgb_nil);
}
struct mathtree_t * 
mathtree_square(struct mathtree_t *t) {
    return mathtree_create(
        sdscat(sdsnew("q"),t->math), false, Vec3rgb_nil);
}
struct mathtree_t * 
mathtree_sin(struct mathtree_t *t) {
    return mathtree_create(
        sdscat(sdsnew("s"), t->math), false, Vec3rgb_nil);
}
struct mathtree_t * 
mathtree_cos(struct mathtree_t *t) {
    return mathtree_create(
        sdscat(sdsnew("c"), t->math), false, Vec3rgb_nil);
}
struct mathtree_t * 
mathtree_tan(struct mathtree_t *t) {
    return mathtree_create(
        sdscat(sdsnew("t"), t->math), false, Vec3rgb_nil);
}
struct mathtree_t * 
mathtree_asin(struct mathtree_t *t) {
    return mathtree_create(
        sdscat(sdsnew("S"), t->math), false, Vec3rgb_nil);
}
struct mathtree_t * 
mathtree_acos(struct mathtree_t *t) {
    return mathtree_create(
        sdscat(sdsnew("C"), t->math), false, Vec3rgb_nil);
}
struct mathtree_t * 
mathtree_atan(struct mathtree_t *t) {
    return mathtree_create(
        sdscat(sdsnew("T"), t->math), false, Vec3rgb_nil);
}
struct mathtree_t *
mathtree_and(struct mathtree_t *a, struct mathtree_t *b) {
    if (a->shape || b->shape) {
        struct mathtree_t *t = mathtree_create(sdscat(sdscat(
            sdsnew("a"), a->math), b->math), true, Vec3rgb_nil);        
        if ((mathtree_dx(a) > 0) && (mathtree_dx(b) > 0)) {
            t->bounds.xmin = fmax(a->bounds.xmin, b->bounds.xmin);
            t->bounds.xmax = fmin(a->bounds.xmax, b->bounds.xmax);
        }
        if ((mathtree_dy(a) > 0) && (mathtree_dy(b) > 0)) {
            t->bounds.ymin = fmax(a->bounds.ymin, b->bounds.ymin);
            t->bounds.ymax = fmin(a->bounds.ymax, b->bounds.ymax);
        }
        if ((mathtree_dz(a) > 0) && (mathtree_dz(b) > 0)) {
            t->bounds.zmin = fmax(a->bounds.zmin, b->bounds.zmin);
            t->bounds.zmax = fmin(a->bounds.zmax, b->bounds.zmax);
        }
        return t;
    } 
    return NULL;
}
struct mathtree_t *
mathtree_clone(struct mathtree_t *t) {
    struct mathtree_t *c = mathtree_create(
        sdsnew(t->math), t->shape, t->color);
    c->bounds = t->bounds;
    if (t->_ptr)
        c->_ptr = clone_tree(t->_ptr);
    return c;
}
struct mathtree_t *
mathtree_add(struct mathtree_t *a, struct mathtree_t *b) {
    if (a->shape || (b && b->shape)) {
        if (!b)
            return mathtree_clone( a);
        struct mathtree_t *t = mathtree_create(
                sdscatprintf(sdsempty(),"i%s%s", a->math, b->math), true, Vec3rgb_nil);
        if ((isnormal(mathtree_dx(a))) 
                && (isnormal(mathtree_dx(b)))) {
            t->bounds.xmin = fmin(a->bounds.xmin, b->bounds.xmin);
            t->bounds.xmax = fmax(a->bounds.xmax, b->bounds.xmax);
        }
        if ((isnormal(mathtree_dx(a))) 
                    && (isnormal(mathtree_dy(b)))) {
            t->bounds.ymin = fmin(a->bounds.ymin, b->bounds.ymin);
            t->bounds.ymax = fmax(a->bounds.ymax, b->bounds.ymax);
        }
        if ((isnormal(mathtree_dz(a))) 
                && (isnormal(mathtree_dz(b)))) {
            t->bounds.zmin = fmin(a->bounds.zmin, b->bounds.zmin);
            t->bounds.zmax = fmax(a->bounds.zmax, b->bounds.zmax);
        }
        return t;
    }
    return mathtree_create(
        sdscatprintf(sdsempty(),"+%s%s", a->math, b->math), false, Vec3rgb_nil);     
}
struct mathtree_t *
mathtree_sub(struct mathtree_t *a, struct mathtree_t *b) {
    if (a->shape || (b && b->shape)) {
        if (!b) return mathtree_clone(a);
        struct mathtree_t *t = mathtree_create(
            sdscatprintf(sdsempty(),"a%sn%s", a->math, b->math), true, Vec3rgb_nil);
        t->bounds.xmin = a->bounds.xmin, t->bounds.xmax = a->bounds.xmax;
        t->bounds.ymin = a->bounds.ymin, t->bounds.ymax = a->bounds.ymax;
        t->bounds.zmin = a->bounds.zmin, t->bounds.zmax = a->bounds.zmax;
        return t;
    }
    return mathtree_create(
        sdscatprintf(sdsempty(),"-%s%s", a->math, b->math), false, Vec3rgb_nil);
}
struct mathtree_t *
mathtree_or(struct mathtree_t *a, struct mathtree_t *b) {
	return NULL;			
}
struct mathtree_t *
mathtree_mul(struct mathtree_t *a, struct mathtree_t *b) {
    return mathtree_create(
        sdscatprintf(sdsempty(),"*%s%s", a->math, b->math), false, Vec3rgb_nil);
}
struct mathtree_t *
mathtree_div(struct mathtree_t *a, struct mathtree_t *b) {
    return mathtree_create(
        sdscatprintf(sdsempty(),"/%s%s", a->math, b->math), false, Vec3rgb_nil);
}
struct mathtree_t *
mathtree_rdiv(struct mathtree_t *a, struct mathtree_t *b) {
    return mathtree_create(
        sdscatprintf(sdsempty(),"/%s%s", b->math, a->math), false, Vec3rgb_nil);
}
struct mathtree_t *
mathtree_unm(struct mathtree_t *a) {
    struct mathtree_t *t = mathtree_create(
        sdscat(sdsnew("n"), a->math), false, Vec3rgb_nil);
    t->shape = a->shape;
    return t;
}
struct mathtree_t *
mathtree_map(struct mathtree_t *t__, struct mathtree_t *X, struct mathtree_t *Y, struct mathtree_t *Z) {
    struct mathtree_t *t = 
        mathtree_create(sdscatprintf(sdsempty(), "m%s%s%s%s",
                    X ? X->math : " ",
                    Y ? Y->math : " ",
                    Z ? Z->math : " ",
                    t__->math ), t__->shape, t__->color);
    return t;
}
struct MathTree_ *
mathtree_ptr(struct mathtree_t *t) {
    if(!t->_ptr) {
        t->_ptr = parse(t->math);
    }
    if(!t->_ptr) printf("FAILED: %s\n", t->math);
    return t->_ptr;
}
struct render16_context_t {
    pt_thread_t pt_thread;
    pt_func_t pt_func;
    struct mathtree_t *clone;
    struct PackedTree_ *packed;
    struct Region_ subregion;
    uint16_t **pixels;
    int halt;
};
static pt_t
render16_thr(void * const ctx) {
    struct render16_context_t *arg = ctx;
    if(!arg->pixels) exit(0);
    render16(arg->packed, arg->subregion, arg->pixels, &arg->halt);
    mathtree_free(arg->clone);
    free_packed(arg->packed);
    free(arg);
    return PT_DONE;
}
struct image_t *
mathtree_render(struct mathtree_t *t, struct region_t r, 
        float resolution, float mm_per_unit, int threads, int interrupt) {
    if(!r.is_set) {
        float xmin = 0, ymin = 0, 
            zmin = 0, xmax =0, ymax = 0, zmax = 0;
        if(isnormal(t->bounds.zmin)) {
            xmin = t->bounds.xmin, ymin = t->bounds.ymin, 
                    zmin = t->bounds.zmin;
        }
        if(isnormal(t->bounds.zmax)) {
            xmax = t->bounds.xmax, ymax = t->bounds.ymax,
                    zmax = t->bounds.zmax;
        }
        r = region_create(
            xmin, ymin, zmin, xmax, ymax, zmax, 
            resolution, false, NAN);
    }
    struct Region_ s[8];
    region_split_xy(r.r, s, &threads);
    struct image_t *img = image_create(r.r.ni, r.r.nj, 1, DT_IU16);
    image_flipped_pixels(img);
    protothread_t const pt = protothread_create_maxpt(threads);
    for(int i=0; i<threads; i++) {
        struct render16_context_t *arg = calloc(1, sizeof(struct render16_context_t));
        arg->clone = mathtree_clone(t);
        arg->packed = make_packed(mathtree_ptr(arg->clone));
        arg->subregion = s[i];
        arg->pixels = (uint16_t **)img->pixels;
        pt_create(pt, &arg->pt_thread, render16_thr, arg);
    }
    protothread_quiesce(pt);
    img->bounds.xmin = r.r.X[0]*mm_per_unit;
    img->bounds.xmax = r.r.X[r.r.ni]*mm_per_unit;
    img->bounds.ymin = r.r.Y[0]*mm_per_unit;
    img->bounds.ymax = r.r.Y[r.r.nj]*mm_per_unit;
    img->bounds.zmin = r.r.Z[0]*mm_per_unit;
    img->bounds.zmax = r.r.Z[r.r.nk]*mm_per_unit;
    protothread_free(pt);
    region_free(r);
    return img;
}
struct bounds_t
mathtree_map_bounds(struct mathtree_t *t, struct mathtree_t *X, 
            struct mathtree_t *Y, struct mathtree_t *Z) {
    struct Interval_ x = isnormal(mathtree_dx(t)) ?
        (struct Interval_){t->bounds.xmin, t->bounds.xmax} : (struct Interval_){NAN, NAN};
    struct Interval_ y =  isnormal(mathtree_dy(t)) ?
        (struct Interval_){t->bounds.ymin, t->bounds.ymax} : (struct Interval_){NAN, NAN};
    struct Interval_ z = isnormal(mathtree_dz(t)) ?
        (struct Interval_){t->bounds.zmin, t->bounds.zmax} : (struct Interval_){NAN, NAN};

    struct Interval_ a = isnormal(mathtree_dx(t)) ?
        (struct Interval_){t->bounds.xmin, t->bounds.xmax} : (struct Interval_){NAN, NAN};
    struct Interval_ b = isnormal(mathtree_dy(t)) ?
        (struct Interval_){t->bounds.ymin, t->bounds.ymax} : (struct Interval_){NAN, NAN};
    struct Interval_ c = isnormal(mathtree_dz(t)) ?
        (struct Interval_){t->bounds.zmin, t->bounds.zmax} : (struct Interval_){NAN, NAN};
    if(X) {
        struct PackedTree_ *X_p = make_packed(mathtree_ptr(X));
        a = eval_i(X_p, x, y, z);
        free_packed(X_p);
    }
    if (Y) {
        struct PackedTree_ *Y_p = make_packed(mathtree_ptr(Y));
        b = eval_i(Y_p, x, y, z);
        free_packed(Y_p);
    }
    if (Z) {
        struct PackedTree_ *Z_p = make_packed(mathtree_ptr(Z));
        c = eval_i(Z_p, x, y, z);
        free_packed(Z_p);
    }
    return (struct bounds_t) {
        .xmin = isnan(a.lower) ? 0 : a.lower, .xmax = isnan(a.upper) ? 0 : a.upper,
        .ymin = isnan(b.lower) ? 0 : b.lower, .ymax = isnan(b.upper) ? 0 : b.upper,
        .zmin = isnan(c.lower) ? 0 : c.lower, .zmax = isnan(c.upper) ? 0 : c.upper
    };
}
struct asdf_branch_context_t {
    pt_thread_t pt_thread;
    pt_func_t pt_func;
    int id, halt;
    struct mathtree_t *clone;
    struct PackedTree_ *packed;
    struct Region_ subregion;
    struct asdf_t *asdf;
    bool merge_leafs;
};
static pt_t
construct_branch_thr(void * const ctx) {
    struct asdf_branch_context_t *arg = ctx;
    arg->asdf->ptr->branches[arg->id] = build_asdf(arg->packed,
            arg->subregion, arg->merge_leafs, &arg->halt);
    mathtree_free(arg->clone);
    free_packed(arg->packed);
    return PT_DONE;
}
struct asdf_t *
mathtree_asdf(struct mathtree_t *t, struct region_t r, float resolution, float mm_per_unit, bool merge_leafs) {
    int i = 0;
    struct asdf_branch_context_t *arg = NULL;
    if (!r.is_set) {
        if(!mathtree_bounded(t)) {
            printf("Unknown render region!");
        } else if(!resolution) {
            printf("Region or resolution must be provided!");
        }
        float xmin = 0, ymin = 0, zmin = 0,
                    xmax = 0, ymax = 0, zmax = 0;
        if(t->bounds.zmin) 
            xmin = t->bounds.xmin, ymin = t->bounds.ymin, 
                        zmin = t->bounds.zmin;
        if(t->bounds.zmin) 
            xmax = t->bounds.xmax, ymax = t->bounds.ymax, 
                        zmax = t->bounds.zmax;
        r = region_create (
            xmin, ymin, zmin ,
            xmax, ymax, zmax,
            resolution,
            false,
            NAN
        );
    }
    struct Region_ *split = region_octsect(r.r, true, false);
    vec_t(struct asdf_branch_context_t *) args;
    vec_init(&args);
    for(int i = 0; i < 8; i++) {
        if (&split[i] != NULL) {
            struct asdf_branch_context_t *arg = 
                calloc(1, sizeof(struct asdf_branch_context_t));
            arg->clone = mathtree_clone(t);
            arg->packed = make_packed(mathtree_ptr(arg->clone));
            arg->subregion = split[i];
            arg->merge_leafs = merge_leafs;
            arg->id = i;
            (void)vec_push(&args, arg);
        }
    }
    struct ASDF_ *__asdf = asdf_root(vec_first(&args)->packed, r.r);
    struct asdf_t *asdf = asdf_create(__asdf, true, t->color);
    pthread_mutex_lock(&asdf->lock); 
    {
        protothread_t const pt = protothread_create();
        vec_foreach(&args, arg, i) {
            arg->asdf = asdf;
            pt_create(pt, &arg->pt_thread, construct_branch_thr, arg);
        }
        protothread_quiesce(pt);
        protothread_free(pt);
        get_d_from_children(asdf->ptr);
        simplify(asdf->ptr, merge_leafs);
    } pthread_mutex_unlock(&asdf->lock); 
    vec_foreach(&args, arg, i) { 
        free(arg); } vec_deinit(&args);
    vec_compact(&args);
    if(mm_per_unit != 0)
        asdf_rescale(asdf, mm_per_unit);
    region_free(r);
    free(split);
    return asdf;
}
struct mesh_t * 
mathtree_triangulate(struct asdf_t *asdf) {
    return 0;
}
bool 
mathtree_bounded(struct mathtree_t *t) {
    return isnormal(mathtree_dy(t)) &&
            isnormal(mathtree_dx(t)) && isnormal(mathtree_dz(t));
}

