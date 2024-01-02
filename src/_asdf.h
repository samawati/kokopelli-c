#ifndef  LIBFAB_ASDF_H
#define LIBFAB_ASDF_H

#include "asdf.h"
#include "asdf_file_io.h"
#include "asdf_import.h"
#include "asdf_triangulate.h"
#include "formats_mesh.h"

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

struct asdf_t {
    bool free;
    Vec3rgb color;
    struct ASDF_ *ptr;
    const char *filename;
    pthread_mutex_t lock;
};

KOKO_API struct asdf_t *asdf_create(struct ASDF_ *ptr, bool free, Vec3rgb color);
KOKO_API struct Interval_ asdf_X(struct asdf_t *a);
KOKO_API struct Interval_ asdf_Y(struct asdf_t *a);
KOKO_API struct Interval_ asdf_Z(struct asdf_t *a);
KOKO_API float  __asdf_interpolate(struct asdf_t *a, float x, float y, float z);
KOKO_API enum ASDFstate asdf_state(struct asdf_t *a);
KOKO_API float *asdf_d(struct asdf_t *a);
KOKO_API float asdf_xmin(struct asdf_t *a);
KOKO_API float asdf_xmax(struct asdf_t *a);
KOKO_API float asdf_dx(struct asdf_t *a);
KOKO_API float asdf_ymin(struct asdf_t *a);
KOKO_API float asdf_ymax(struct asdf_t *a);
KOKO_API float asdf_dy(struct asdf_t *a);
KOKO_API float asdf_zmin(struct asdf_t *a);
KOKO_API float asdf_zmax(struct asdf_t *a);
KOKO_API float asdf_dz(struct asdf_t *a);
KOKO_API int asdf_mm_per_unit(struct asdf_t *a);
KOKO_API void asdf_rescale(struct asdf_t *a, const float scale);
KOKO_API int asdf_depth(struct asdf_t *a);
KOKO_API void asdf_dimensions(struct asdf_t *a, int* ni, int* nj, int* nk);
KOKO_API int asdf_cell_count(struct asdf_t *a);
KOKO_API int asdf_ram(struct asdf_t *a);
KOKO_API void asdf_save(struct asdf_t *a, const char *filename);
KOKO_API struct asdf_t *asdf_load(const char *filename);
KOKO_API struct asdf_t *asdf_from_pixels(struct image_t *i, int offset, bool merge_leafs);
KOKO_API void asdf_free(struct asdf_t *a);
KOKO_API struct mesh_t *asdf_triangulate(struct asdf_t *asdf, int threads);

#endif