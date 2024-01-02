#ifndef  LIBFAB_MATHTREE_H
#define LIBFAB_MATHTREE_H

#include "tree_render.h"
#include "tree_parser.h"
#include "tree_packed.h"
#include "tree_eval.h"
#include "tree.h"

#include "common.h"

#include "_region.h"
#include "_scheduler.h"
#include "_region.h"
#include "_image.h"
#include "_mesh.h"
#include "_asdf.h"
#include "_fabvars.h"
#include "_mathtree.h"
#include "_shape.h"

struct mathtree_t {
    char *math, *_str;
    bool shape;
    Vec3rgb color;
    struct MathTree_ *_ptr;
    struct bounds_t bounds;
};

KOKO_API struct mathtree_t *mathtree_create(const char *math, bool shape, const Vec3rgb color);
KOKO_API void mathtree_free(struct mathtree_t *t);
struct mathtree_t *mathtree_constant(float f);
KOKO_API struct mathtree_t *mathtree_X(void);
KOKO_API struct mathtree_t *mathtree_Y(void);
KOKO_API struct mathtree_t *mathtree_Z(void);
KOKO_API float mathtree_dx(struct mathtree_t *t);
KOKO_API float mathtree_dy(struct mathtree_t *t);
KOKO_API float mathtree_dz(struct mathtree_t *t);
KOKO_API struct mathtree_t *mathtree_min(const char *a, const char *b);
KOKO_API struct mathtree_t *mathtree_max(const char *a, const char *b);
KOKO_API struct mathtree_t *mathtree_pow(const char *a, const char *b);
KOKO_API struct mathtree_t *mathtree_sqrt(struct mathtree_t *t);
KOKO_API struct mathtree_t *mathtree_abs(struct mathtree_t *t);
KOKO_API struct mathtree_t *mathtree_square(struct mathtree_t *t);
KOKO_API struct mathtree_t *mathtree_sin(struct mathtree_t *t);
KOKO_API struct mathtree_t *mathtree_cos(struct mathtree_t *t);
KOKO_API struct mathtree_t *mathtree_tan(struct mathtree_t *t);
KOKO_API struct mathtree_t *mathtree_asin(struct mathtree_t *t);
KOKO_API struct mathtree_t *mathtree_atan(struct mathtree_t *t);
KOKO_API struct mathtree_t *mathtree_acos(struct mathtree_t *t);
KOKO_API struct mathtree_t *mathtree_and(struct mathtree_t *a, struct mathtree_t *b);
KOKO_API struct mathtree_t *mathtree_clone(struct mathtree_t *t);
KOKO_API struct mathtree_t *mathtree_add(struct mathtree_t *a, struct mathtree_t *b);
KOKO_API struct mathtree_t *mathtree_sub(struct mathtree_t *a, struct mathtree_t *b);
KOKO_API struct mathtree_t *mathtree_or(struct mathtree_t *a, struct mathtree_t *b);
KOKO_API struct mathtree_t *mathtree_mul(struct mathtree_t *a, struct mathtree_t *b);
KOKO_API struct mathtree_t *mathtree_div(struct mathtree_t *a, struct mathtree_t *b);
KOKO_API struct mathtree_t *mathtree_rdiv(struct mathtree_t *a, struct mathtree_t *b);
KOKO_API struct mathtree_t *mathtree_unm(struct mathtree_t *a);
KOKO_API struct mathtree_t *mathtree_map(struct mathtree_t *t__, struct mathtree_t *X, struct mathtree_t *Y, struct mathtree_t *Z);
KOKO_API struct MathTree_ *mathtree_ptr(struct mathtree_t *t);
KOKO_API struct bounds_t  mathtree_map_bounds(struct mathtree_t *t,
            struct mathtree_t *X, struct mathtree_t *Y, struct mathtree_t *Z);
KOKO_API struct asdf_t *mathtree_asdf(struct mathtree_t *m, struct region_t r, float resolution, float mm_per_unit, bool merge_leafs);
KOKO_API struct mesh_t * mathtree_triangulate(struct asdf_t *asdf);
KOKO_API struct image_t *mathtree_render(struct mathtree_t *t, struct region_t r, 
        float resolution, float mm_per_unit, int threads, int interrupt);
KOKO_API bool mathtree_bounded(struct mathtree_t *t);

#endif