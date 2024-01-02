#ifndef LIBFAB_RENDER_H
#define LIBFAB_RENDER_H

#include "util_region.h"

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

KOKO_API struct image_t *make_flat_image(struct fabvars_t *cad, struct mathtree_t *t, float scale);
KOKO_API struct mesh_t *make_mesh(struct fabvars_t *cad, struct mathtree_t *t);
KOKO_API struct image_t *make_image(struct fabvars_t *cad, struct mathtree_t *t, float zmin, float zmax, float scale);
KOKO_API void make_images(struct fabvars_t *cad, vec_image_t *images, float scale);

#endif