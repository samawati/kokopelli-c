#ifndef  LIBFAB_MESH_H
#define LIBFAB_MESH_H

#include "_asdf.h"
#include "_image.h"
#include "_mathtree.h"
#include "_region.h"

#include "util_vec3f.h"
#include "formats_mesh.h"
#include "vec.h"

#include "common.h"

typedef vec_t(struct mesh_t *) mesh_vec_t;
typedef vec_t(struct Mesh_ *) mesh_ptrs_vec_t;

struct mesh_t {
    struct Mesh_ *ptr;
    mesh_vec_t children;
    struct source_t {
        enum { MESH_NONE, MESH_ASDF, MESH_MATHTREE } type;
        const char *file;
        float depth, scale;
        struct mathtree_t *expr;
    } source;
    Vec3rgb color;
    const char *cached;
};

KOKO_API struct mesh_t *mesh_create(struct Mesh_ *ptr, Vec3rgb color);
KOKO_API void mesh_free(struct mesh_t *m);
KOKO_API struct Interval_ mesh_X(struct mesh_t *m);
KOKO_API struct Interval_ mesh_Y(struct mesh_t *m);
KOKO_API struct Interval_ mesh_Z(struct mesh_t *m) ;
KOKO_API uint32_t mesh_tcount(struct mesh_t *m);
KOKO_API uint32_t mesh_vcount(struct mesh_t *m) ;
KOKO_API float *mesh_vdata(struct mesh_t *m);
KOKO_API uint32_t  *mesh_tdata(struct mesh_t *m);
KOKO_API void mesh_save_stl(struct mesh_t *m, const char *filename);
KOKO_API void mesh_save(struct mesh_t *m, const char *filename) ;
KOKO_API void mesh_refine_math(struct mesh_t *m);
KOKO_API void mesh_refine_asdf(struct mesh_t *m);
KOKO_API void mesh_refine(struct mesh_t *m);
KOKO_API struct mesh_t *mesh_collapse_tree(struct mesh_t *m);
KOKO_API struct mesh_t *mesh_merge(mesh_ptrs_vec_t *ptrs);
KOKO_API void mesh_leafs(struct mesh_t *m, mesh_ptrs_vec_t *leafs);

#endif