#ifndef LIBFAB_FABVARS_H
#define LIBFAB_FABVARS_H

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

typedef vec_t(struct mathtree_t *) mathtree_vec_t;

struct fabvars_t {
    mathtree_vec_t _shapes;
    struct mathtree_t *_shape;
    struct bounds_t bounds;
    enum { RENDER_2D, RENDER_3D } render_mode;
    float border, 
        mm_per_unit;
};

KOKO_API float koko_fabvars_xmin(struct fabvars_t *f);
KOKO_API float koko_fabvars_xmax(struct fabvars_t *f);
KOKO_API float koko_fabvars_dx(struct fabvars_t *f);
KOKO_API float koko_fabvars_ymin(struct fabvars_t *f);
KOKO_API float koko_fabvars_ymax(struct fabvars_t *f);
KOKO_API float koko_fabvars_dy(struct fabvars_t *f);
KOKO_API float koko_fabvars_zmin(struct fabvars_t *f);
KOKO_API float koko_fabvars_zmax(struct fabvars_t *f);
KOKO_API float koko_fabvars_dz(struct fabvars_t *f);
KOKO_API bool koko_fabvars_bounded(struct fabvars_t *f);

#endif