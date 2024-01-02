#ifndef LIBFAB_IMAGE_H
#define LIBFAB_IMAGE_H

#include "formats_stl.h"
#include "formats_png_image.h"

#include "common.h"

typedef enum { DT_IU8, DT_IU16, DT_IU32, DT_FLOAT, DT_NTYPES, DT_NOTYPE } image_dtype_t;

typedef vec_t(struct image_t *) vec_image_t;

struct image_t {
    const char *filename;
    uint32_t width, height, 
        channels;
    Vec3rgb color;
    struct bounds_t bounds;
    image_dtype_t dtype;
    void *data;
    void **pixels;
};

KOKO_API struct image_t *image_create(int w, int h, int channels, image_dtype_t dtype);
KOKO_API float image_dx(struct image_t *i);
KOKO_API void image_free(struct image_t *i);
KOKO_API float image_dy(struct image_t *i);
KOKO_API float image_dz(struct image_t *i);
KOKO_API void image_pixels(struct image_t *i);
KOKO_API void image_flipped_pixels(struct image_t *i);
KOKO_API struct image_t *image_load(const char *filename);
KOKO_API struct image_t *image_copy(struct image_t *src, int channels, image_dtype_t dtype);
KOKO_API void image_save(struct image_t *i, const char *filename);
KOKO_API float image_pixels_per_mm(struct image_t *i);
KOKO_API struct image_t *image_merge(vec_image_t images);

#endif