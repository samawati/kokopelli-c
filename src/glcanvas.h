#ifndef GLCANVAS_H
#define GLCANVAS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "_mesh.h"
#include "_render.h"
#include "_fabvars.h"
#include "_image.h"
#include "_shape.h"

struct glcanvas_vbo_t {
    uint32_t tcounts;
    GLuint vbo_vdata, 
            vbo_tdata;
    struct mesh_t *m, *merged;
};

typedef vec_t(struct glcanvas_vbo_t *) glcanvas_vbo_vec_t;
enum glcanvas_drag_t { WINDOW_DRAG_NONE, WINDOW_DRAG_PAN, WINDOW_DRAG_ROTATE};

typedef enum glcanvas_shader_t {
    WIREFRAME_SHADER,
    NORMALS_SHADER,
    SHADED_SHADER,
    PATH_SHADER,
    SUB_SHADER } glcanvas_shader_t;

struct glcanvas_t {
    pt_thread_t pt_thread;
    pt_func_t pt_func;
    GLFWwindow *window;
    GLuint texture;
    GLuint tex_vbo, path_vbo;
    GLuint image_shader, plain_shader, wire_shader, 
        norm_shader, flat_shader, path_shader;
    lua_State *L;
    Vec2f mouse;
    Vec3f center;
    Vec3rgb border;
    bool snap, clear, redraw, 
        halt, draw_axes, draw_bounds;
    int width, height;
    float scale, alpha, beta;
    vec_image_t images;
    mesh_vec_t meshes;
    glcanvas_vbo_vec_t mesh_vbos;
    vec_t(mesh_vec_t *) leafs;
    enum glcanvas_drag_t drag_mode;
    struct fabvars_t *cad;
    struct image_t *image;
    protothread_t pt;
    int current_3dshader;
};

KOKO_API struct glcanvas_t *glcanvas_create(const char *title, Vec2f dimensions);
KOKO_API bool glcanvas_reeval(struct glcanvas_t *g, const char *script);
KOKO_API void glcanvas_run(struct glcanvas_t *g);
KOKO_API void glcanvas_poll(struct glcanvas_t *g);

#endif