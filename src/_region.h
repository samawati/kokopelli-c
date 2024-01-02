#ifndef LIBFAB_REGION_H
#define LIBFAB_REGION_H

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>

#include "common.h"

#include "util_region.h"
#include "vec.h"

typedef vec_t(struct Region_ *) region_vec_t;

struct region_t {
    bool is_set;
    struct Region_ r;
    bool free_arrays;
};

KOKO_API struct region_t region_create(float xmin, float ymin, float zmin, float xmax, 
        float ymax, float zmax, float scale, bool dummy, float depth);
KOKO_API struct Region_ *region_octsect(struct Region_ r, bool all, bool overlap);
KOKO_API void region_split_xy(struct Region_ r, struct Region_ L[8], int *count);
KOKO_API void region_free(struct region_t r);

#endif