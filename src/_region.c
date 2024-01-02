#include "_region.h"

struct region_t
region_create(float xmin, float ymin, float zmin, float xmax, 
        float ymax, float zmax, float scale, bool dummy, float depth) {
    float dx = xmax - xmin;
    float dy = ymax - ymin;
    float dz = zmax - zmin;
    if (isnormal(depth)) {
        scale = 3*(2^6) * (powf(2,(depth/3.))) / (dx+dy+dz);
    }
    float ni = fmax((int)(roundf(dx*scale)), 1);
    float nj = fmax((int)(roundf(dy*scale)), 1);
    float nk = fmax((int)(roundf(dz*scale)), 1);
    struct region_t r = {
        .r = (struct Region_ ){ 
            .imin = 0, .jmin = 0, .kmin = 0,
            .ni = ni, .nj = nj, .nk = nk,
            .voxels = ni*nj*nk,
            .X = NULL, .Y = NULL, .Z = NULL,
            .L = NULL
        },
        .free_arrays = false,
        .is_set = true
    };
    if(!dummy) {
        build_arrays(&r.r, 
            xmin, ymin, zmin, xmax, ymax, zmax);
    } else r.free_arrays = false;
    return r;
}
void 
region_free(struct region_t r) {
    if(r.free_arrays == true) {
        free_arrays(&r.r);
    }
}
struct Region_ *
region_octsect(struct Region_ r, bool all, bool overlap) {
    static struct Region_ L[8];
    uint8_t bits = (overlap) ? 
        octsect_overlap(r, L) : octsect(r, L);
    struct Region_ *__L = calloc(8, sizeof(struct Region_));
    if (all) {
        for(int i = 0; i < 8; i++) {
            if ((bits & (1 << i))) {
                __L[i] = L[i];
            }
        }
    } else {
        for(int i = 0,j=0; i < 8; i++) {
            if (bits & (1 << i)) {
                __L[j++] = L[i];
            }
        }
    }
    return __L;
}
void
region_split_xy(struct Region_ r,  struct Region_ L[8], int *count) {
    *count = split_xy(r, L, *count);
}