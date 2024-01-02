#include "glcanvas.h"

static GLuint 
shader_compile(const char *s, GLenum type) {
    GLint compile_status;
    GLuint handle = glCreateShader(type);
    glShaderSource(handle, 1, &s, 0);
    glCompileShader(handle);
    glGetShaderiv(handle, GL_COMPILE_STATUS, &compile_status);
    if(compile_status == GL_FALSE) {
        GLchar messages[256];
        glGetShaderInfoLog(handle, sizeof(messages), 0, &messages[0]);
        fprintf(stderr, "%s\n", messages);
        exit(EXIT_FAILURE);
    }
    return handle;
}
static GLuint 
shader_link(const GLuint vert, const GLuint frag) {
    GLint compile_status;  
    GLuint handle = glCreateProgram();
    glAttachShader(handle, vert);
    glAttachShader(handle, frag);
    glLinkProgram(handle);                                      
    glGetProgramiv(handle, GL_LINK_STATUS, &compile_status);                               
    if(compile_status == GL_FALSE) {
        GLchar messages[256];
        glGetProgramInfoLog(handle, sizeof(messages), 0, &messages[0]);
        fprintf(stderr, "%s\n", messages);
        exit(EXIT_FAILURE);
    }
    return handle;
}
static void 
compile_shaders(struct glcanvas_t *g) {
    const char *mesh_vs =
        "#version 120\n"
        "attribute vec3 vertex_position;\n"
        "attribute vec3 vertex_normal;\n"
        "varying vec3 normal;\n"
        "void main() {\n"
            "gl_Position = gl_ModelViewProjectionMatrix *\n"
                            "vec4(vertex_position, 1.0);\n"
            "normal = normalize(gl_NormalMatrix*vertex_normal);\n"
        "}\n";

    const char *shaded_fs =
        "#version 120\n"
        "varying vec3 normal;\n"
        "uniform vec4 color;\n"
        "void main() {\n"
            "gl_FragColor = vec4(0.1 + 0.9*normal[2]*color[0],\n"
                                "0.1 + 0.9*normal[2]*color[1],\n"
                                "0.1 + 0.9*normal[2]*color[2],\n"
                                "color[3]);\n"
        "}\n";

    const char *wire_fs =
        "#version 120\n"
        "varying vec3 normal;\n"
        "uniform vec4 color;\n"
        "void main() {\n"
            "float B = normal[2] < 0.0 ? 0.2 : normal[2]*0.8+0.2;\n"
            "gl_FragColor = vec4(B*color[0], B*color[1], B*color[2], color[3]);\n"
        "}\n";

    const char *norm_fs =
        "#version 120\n"
        "varying vec3 normal;\n"
        "void main() {\n"
           "gl_FragColor = vec4(normal[0]/2 + 0.5,\n"
                                "normal[1]/2 + 0.5,\n"
                                "normal[2]/2 + 0.5, 1.0);\n"
        "}\n";

    g->plain_shader = shader_link(shader_compile(mesh_vs, GL_VERTEX_SHADER),
        shader_compile(shaded_fs, GL_FRAGMENT_SHADER));
    g->wire_shader = shader_link(shader_compile(mesh_vs, GL_VERTEX_SHADER),
        shader_compile(wire_fs, GL_FRAGMENT_SHADER));
    g->norm_shader = shader_link(shader_compile(mesh_vs, GL_VERTEX_SHADER),
        shader_compile(norm_fs, GL_FRAGMENT_SHADER));

    const char *flat_vs =
        "#version 120\n"
        "attribute vec3 vertex_position;\n"
        "void main() {\n"
            "gl_Position = gl_ModelViewProjectionMatrix *\n"
                            "vec4(vertex_position, 1.0);\n"
        "}\n";
   
    const char *flat_fs =
        "#version 120\n"
        "uniform vec4 color;\n"
        "void main() {\n"
            "gl_FragColor = color;\n"
        "}\n";
   
    g->flat_shader = shader_link(shader_compile(flat_vs, GL_VERTEX_SHADER),
        shader_compile(flat_fs, GL_FRAGMENT_SHADER));

    const char *path_vs =
        "#version 120\n"
        "attribute vec3  vertex_position;\n"
        "attribute float vertex_color;\n"
        "varying float color;\n"
        "void main() {\n"
            "gl_Position = gl_ModelViewProjectionMatrix *\n"
                            "vec4(vertex_position, 1.0);\n"
            "color = vertex_color;\n"
        "}\n";
   
    const char *path_fs =
        "#version 120\n"
        "varying float color;\n"
        "uniform int show_traverses;\n"
        "void main() {\n"
           "if (color == 0.0)\n"
                "gl_FragColor = vec4(0.9, 0.2, 0.2,\n"
                                    "show_traverses > 0.0 ? 0.5 : 0.0);\n"
           "else\n"
                "gl_FragColor = vec4(0.3*color + 0.2,\n"
                                    "0.8*color + 0.2,\n"
                                    "1.0 - color,\n"
                                    "0.9);\n"
        "}\n";
   
    g->path_shader = shader_link(shader_compile(path_vs, GL_VERTEX_SHADER),
        shader_compile(path_fs, GL_FRAGMENT_SHADER));

    const char *tex_vs = 
        "#version 120\n"
        "attribute vec3 vertex_position;\n"
        "attribute vec2 vertex_texcoord;\n"
        "varying vec2 texcoord;\n"
        "void main()\n"
        "{\n"
            "gl_Position = gl_ModelViewProjectionMatrix *\n"
                            "vec4(vertex_position, 1.0);\n"
            "texcoord = vertex_texcoord;\n"
        "}\n";

    const char *tex_fs =
        "#version 120\n"
        "uniform sampler2D texture;\n"
        "varying vec2 texcoord;\n"
        "void main()\n"
        "{\n"
            "vec4 color = texture2D(texture, texcoord);\n"
            "if (color[0] != 0.0 || color[1] != 0.0 || color[2] != 0.0)\n"
                "gl_FragColor = color;\n"
            "else\n"
                "gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);\n"
        "}\n";
    
    g->image_shader = shader_link(shader_compile(tex_vs, GL_VERTEX_SHADER), 
        shader_compile(tex_fs, GL_FRAGMENT_SHADER));
  
}
static void 
glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "glfw Error %d: %s\n", error, description);
    fflush(stderr);
}
static GLuint
glcanvas_get_shader(struct glcanvas_t *g, glcanvas_shader_t type) {
    struct shader_t {
        GLuint shader;
    } shaders[5] = {
        { g->wire_shader  },
        { g->norm_shader  },
        { g->plain_shader },
        { g->path_shader  },
        { g->flat_shader  }
    };
    return (type >0 && type<6) ?
        shaders[type].shader : -1;
}
static Vec3f
glcanvas_deproject(struct glcanvas_t *g, float dx, float dy) {
    double alpha = g->alpha*(M_PI/180);
    double beta = g->beta*(M_PI/180);
    float M[4] = { cos(alpha), sin(alpha), cos(beta), sin(beta)};
    return deproject((Vec3f) { 0.01/g->scale*dx, -0.01/g->scale*dy, 0 }, M);
}
static void
glcanvas_pan_handler(struct glcanvas_t *g, Vec2f delta) {
    g->center = Vec3f_sub(g->center, 
        glcanvas_deproject(g, delta.x, delta.y));
}
static void 
glcanvas_spin_handler(struct glcanvas_t *g, Vec2f delta) {
    g->alpha = fmodf((g->alpha+delta.x), 360);
    g->beta -= delta.y;
    if (g->beta < 0) g->beta = 0;
    else if (g->beta > 180) g->beta = 180;
}
static void
 __glcanvas_resized(GLFWwindow* window, int w, int h) {
    struct glcanvas_t *g = (struct glcanvas_t*)
            glfwGetWindowUserPointer(window);
    g->width = w;
    g->height = h;
    g->redraw = true;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = w/(float)h;
    (aspect > 1) ? glFrustum(-aspect, aspect, -1, 1, 1, 9) :
            glFrustum(-1, 1, -1/aspect, 1/aspect, 1, 9);
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0627451, 0.0627451, 0.0627451, 1);
    glfwSwapBuffers(g->window);
}
static void 
__glcanvas_mousemove(GLFWwindow* window, double x, double y) {
    struct glcanvas_t *g = (struct glcanvas_t*)
            glfwGetWindowUserPointer(window);
    glfwMakeContextCurrent(g->window);
    Vec2f newpos = (Vec2f) { x*2, y*2};
    switch(g->drag_mode) {
        case WINDOW_DRAG_PAN : {
            glcanvas_pan_handler(g, 
                Vec2f_sub(newpos, g->mouse));
            break;
        }
        case WINDOW_DRAG_ROTATE : {
            glcanvas_spin_handler(g, 
                Vec2f_sub(newpos, g->mouse));
            break;
        }
        default: break;
    };
    g->mouse = newpos;
    if (g->drag_mode != WINDOW_DRAG_NONE) {
        g->redraw = true; 
        glfwPostEmptyEvent();
    }
}
static void 
__glcanvas_mousebutton(GLFWwindow* window, int button, int action, int mods) {
    struct glcanvas_t *g = (struct glcanvas_t*)
            glfwGetWindowUserPointer(window);
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_2 ||
            (button == GLFW_MOUSE_BUTTON_1 && (mods & GLFW_MOD_CONTROL))) {
            g->drag_mode = WINDOW_DRAG_ROTATE;
        } else if (button == GLFW_MOUSE_BUTTON_1) {
            g->drag_mode = WINDOW_DRAG_PAN;
        }
    } else {
        g->drag_mode = WINDOW_DRAG_NONE;
    }
    double x, y;
    glfwGetCursorPos(g->window, &x, &y);
    g->mouse = (Vec2f){x*2, y*2};
}
static void 
__glcanvas_mousescroll(GLFWwindow* window, double sx, double sy) {
    struct glcanvas_t *g = (struct glcanvas_t*)
            glfwGetWindowUserPointer(window);
    const float delta = 1.1;
    g->scale *= sy < 0 ? delta : 1/delta;
    g->redraw = true; 
    glfwPostEmptyEvent();
}
static void 
glcanvas_orient(struct glcanvas_t *g) {
    glTranslatef(0, 0, -5);
    glScalef(g->scale, g->scale, g->scale);
    glRotatef(-g->beta,  1, 0, 0);
    glRotatef(g->alpha, 0, 0, 1);
    glTranslatef(-g->center.x, -g->center.y, -g->center.z);
}
static void 
glcanvas_snap_bounds(struct glcanvas_t *g, Vec3f center, float scale) {
    g->center = center;
    g->scale  = scale;
    g->redraw = true; 
    glfwPostEmptyEvent();
}
static void 
glcanvas_draw_axes(struct glcanvas_t *g) {
    glDisable(GL_DEPTH_TEST);
    glScalef(0.5/g->scale, 0.5/g->scale, 0.5/g->scale);
    glLineWidth(2);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(1, 0, 0);
        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 1, 0);
        glColor3f(0.2, 0.2, 1);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 1);
    glEnd();
    glLineWidth(1);
    glEnable(GL_DEPTH_TEST);
}
static void 
glcanvas_draw_image(struct glcanvas_t *g) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPushMatrix();
    glcanvas_orient(g);
    GLuint shader = g->image_shader;
    glUseProgram(shader);
    GLuint a[2] = {
        glGetAttribLocation(
            shader, "vertex_position"),
        glGetAttribLocation(
            shader, "vertex_texcoord") };
    glEnableVertexAttribArray(a[0]);
    glEnableVertexAttribArray(a[1]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g->texture);
    glUniform1i(glGetUniformLocation(shader, "texture"), 0);
    glBindBuffer(GL_ARRAY_BUFFER, g->tex_vbo);
    glVertexAttribPointer(
            a[0], 3, GL_FLOAT, GL_FALSE,
            5 * sizeof(GLfloat), (GLvoid*)0);
    glVertexAttribPointer(
            a[1], 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
            (GLvoid*)(3 * sizeof(GLfloat)));
    glDrawArrays(GL_TRIANGLES,  0, 6);
    glDisableVertexAttribArray(a[0]);
    glDisableVertexAttribArray(a[1]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glPopMatrix();
}
static void
glcanvas_draw_paths(struct glcanvas_t *g) {
    glLineWidth(2);
    glPushMatrix();
    glcanvas_orient(g);
    GLuint shader = g->path_shader;
    glUseProgram(shader);
    GLuint a[2] = {
        glGetAttribLocation(
            shader, "vertex_position"),
        glGetAttribLocation(
            shader, "vertex_color") };
    glEnableVertexAttribArray(a[0]);
    glEnableVertexAttribArray(a[1]);
    glBindBuffer(GL_ARRAY_BUFFER, g->path_vbo);
    glVertexAttribPointer(
        a[0], 3, GL_FLOAT, GL_FALSE, 
        4*sizeof(GLfloat), (GLvoid*)0);
    glVertexAttribPointer(
        a[1], 1, GL_FLOAT, GL_FALSE, 
        4*sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glUniform1i(glGetUniformLocation(shader, "show_traverses"), 1);
    glDrawArrays(GL_LINE_STRIP, 0, sizeof(g->path_vbo)/4);
    glDisableVertexAttribArray(a[0]);
    glDisableVertexAttribArray(a[1]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glPopMatrix();
    glLineWidth(1);
}
static void 
glcanvas_draw_cube(Interval X, Interval Y, Interval Z) {
    glBegin(GL_QUADS);
        glVertex3f(X.lower, Y.lower, Z.upper);
        glVertex3f(X.lower, Y.upper, Z.upper);
        glVertex3f(X.upper, Y.upper, Z.upper);
        glVertex3f(X.upper, Y.lower, Z.upper);
        glVertex3f(X.lower, Y.lower, Z.lower);
        glVertex3f(X.lower, Y.upper, Z.lower);
        glVertex3f(X.upper, Y.upper, Z.lower);
        glVertex3f(X.upper, Y.lower, Z.lower);
        glVertex3f(X.lower, Y.upper, Z.lower);
        glVertex3f(X.lower, Y.upper, Z.upper);
        glVertex3f(X.upper, Y.upper, Z.upper);
        glVertex3f(X.upper, Y.upper, Z.lower);
        glVertex3f(X.lower, Y.lower, Z.lower);
        glVertex3f(X.lower, Y.lower, Z.upper);
        glVertex3f(X.upper, Y.lower, Z.upper);
        glVertex3f(X.upper, Y.lower, Z.lower);
        glVertex3f(X.lower, Y.lower, Z.lower);
        glVertex3f(X.lower, Y.lower, Z.upper);
        glVertex3f(X.lower, Y.upper, Z.upper);
        glVertex3f(X.lower, Y.upper, Z.lower);
        glVertex3f(X.upper, Y.lower, Z.lower);
        glVertex3f(X.upper, Y.lower, Z.upper);
        glVertex3f(X.upper, Y.upper, Z.upper);
        glVertex3f(X.upper, Y.upper, Z.lower);
    glEnd();
}
static void 
glcanvas_draw_rect(float X[2], float Y[2]) {
    glBegin(GL_QUADS);
        glVertex3f(X[0], Y[0], 0);
        glVertex3f(X[1], Y[0], 0);
        glVertex3f(X[1], Y[1], 0);
        glVertex3f(X[0], Y[1], 0);
    glEnd();
}
static void 
glcanvas_draw_bounds(struct glcanvas_t *g) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(0.5, 0.5, 0.5);
    int i;
    struct mesh_t *m = NULL;
    vec_foreach(&g->meshes, m, i) {
        glcanvas_draw_cube(m->ptr->X, m->ptr->Y, m->ptr->Z);
    }
    if (g->image) {
        float X[2] = {g->image->bounds.xmin, g->image->bounds.xmax};
        float Y[2] = {g->image->bounds.ymin, g->image->bounds.ymax};
        glcanvas_draw_rect(X, Y);
    }
}
static void 
glcanvas_draw_import_bounds(struct glcanvas_t *g) {

}
static void 
glcanvas_draw_border(struct glcanvas_t *g) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glfwGetFramebufferSize(g->window, &g->width, &g->height);
    glViewport(0, 0, g->width, g->height);
    float aspect = g->width/(float)g->height;
    float xmin = 0, xmax = 0, ymin = 0, ymax = 0;
    if (aspect > 1) {
        xmin = -aspect, xmax = aspect;
        ymin = -1, ymax = 1;
    } else {
        xmin = -1, xmax = 1;
        ymin = -1/aspect, ymax = 1/aspect;
    }
    glBegin(GL_QUADS);
        glColor3f(g->border.r/255.,
            g->border.g/255.,g->border.b/255.);
        glVertex3f(xmin, ymin, -1);
        glVertex3f(xmin, ymax, -1);
        glVertex3f(xmin+0.01, ymax, -1);
        glVertex3f(xmin+0.01, ymin, -1);
        glVertex3f(xmin, ymax, -1);
        glVertex3f(xmax, ymax, -1);
        glVertex3f(xmax, ymax-0.01, -1);
        glVertex3f(xmin, ymax-0.01, -1);
        glVertex3f(xmax-0.01, ymin, -1);
        glVertex3f(xmax-0.01, ymax, -1);
        glVertex3f(xmax, ymax, -1);
        glVertex3f(xmax, ymin, -1);
        glVertex3f(xmin, ymin+0.01, -1);
        glVertex3f(xmax, ymin+0.01, -1);
        glVertex3f(xmax, ymin, -1);
        glVertex3f(xmin, ymin, -1);
    glEnd();
}
static void 
glcanvas_snap_axis(struct glcanvas_t *g, char *axis) {
    if(!strcmp(axis, "-x")) { g->alpha = 90;  g->beta = 90; }
    else if(!strcmp(axis,"-y")) { g->alpha = 0;   g->beta = 90; }
    else if(!strcmp(axis,"+z")) { g->alpha = 0;   g->beta = 0;  }
    else if(!strcmp(axis,"+x")) { g->alpha = 270; g->beta = 90; }
    else if(!strcmp(axis,"+y")) { g->alpha = 180; g->beta = 90; }
    else if(!strcmp(axis,"-z")) { g->alpha = 0;   g->beta = 180;}
}
static void 
glcanvas_draw_flat(struct glcanvas_t *g, float scale) {
    scale = (scale<=0) ? 1 : scale;
}
static void 
glcanvas_mesh_vbos_free(struct glcanvas_t *g) {
    int i = 0;
    struct glcanvas_vbo_t *vbo = NULL;
    vec_foreach(&g->mesh_vbos, vbo, i) {
        mesh_free(vbo->merged);
        glDeleteBuffers(1, &vbo->vbo_vdata);
        glDeleteBuffers(1, &vbo->vbo_tdata);
        free(vbo); } vec_deinit(&g->mesh_vbos);
    vec_compact(&g->mesh_vbos);
}
static void
glcanvas_reload_vbos(struct glcanvas_t *g) {
    int i = 0, j =0;
    struct Mesh_ *l = NULL;
    struct mesh_t *m = NULL;
    glcanvas_vbo_vec_t mesh_vbos;
    vec_init(&mesh_vbos);
    vec_foreach(&g->meshes, m, i) {
        struct glcanvas_vbo_t *vbo =  
            calloc(1, sizeof(struct glcanvas_vbo_t));
        vbo->m = m;
        (void)vec_push(&mesh_vbos, vbo);
        mesh_ptrs_vec_t leafs;
        vec_init(&leafs);
        mesh_leafs(m, &leafs);
        //vec_push(&g->leafs, &leafs);
        vbo->merged = mesh_merge(&leafs);
        vec_foreach(&leafs, l, j) { 
            vbo->tcounts += l->tcount; } vec_deinit(&leafs);
        vec_compact(&leafs);
        glGenBuffers(1, &vbo->vbo_vdata);
        glBindBuffer(GL_ARRAY_BUFFER, vbo->vbo_vdata);
        glBufferData(GL_ARRAY_BUFFER, vbo->merged->ptr->vcount*4*6,
                    vbo->merged->ptr->vdata, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &vbo->vbo_tdata);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->vbo_tdata);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, vbo->merged->ptr->tcount*4*3,
                    vbo->merged->ptr->tdata, GL_DYNAMIC_DRAW);
    }
    glcanvas_mesh_vbos_free(g);
    g->mesh_vbos = mesh_vbos;
}
static void 
glcanvas_draw_mesh(struct glcanvas_t *g) {
    (g->current_3dshader == WIREFRAME_SHADER) ? 
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) :
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPushMatrix();
    glcanvas_orient(g);
    GLuint shader = glcanvas_get_shader(g, g->current_3dshader);
    glUseProgram(shader);
    GLuint a[2] = {
        glGetAttribLocation(
            shader, "vertex_position"),
        glGetAttribLocation(
            shader, "vertex_normal") };
    glEnableVertexAttribArray(a[0]);
    glEnableVertexAttribArray(a[1]);
    GLuint color_loc = glGetUniformLocation(shader, "color");
    int i;
    struct glcanvas_vbo_t *vbo = NULL;
    vec_foreach(&g->mesh_vbos, vbo, i) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo->vbo_vdata);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->vbo_tdata);
        if (color_loc != -1) {
            Vec3rgb color = (vbo->m->source.expr->color.is_set) ? 
                vbo->m->source.expr->color : (Vec3rgb){.is_set=true, .r=255, .g=255, .b=255};
            glUniform4f(color_loc, color.r/255., color.g/255., color.b/255., 1);
        }
        glVertexAttribPointer(
                a[0], 3, GL_FLOAT, GL_FALSE,
                6 * sizeof(GLfloat), (GLvoid*)0);
        glVertexAttribPointer(
                a[1], 3, GL_FLOAT, GL_FALSE, 
                6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glDrawElements(GL_TRIANGLES, vbo->tcounts*3,
                        GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glDisableVertexAttribArray(a[0]);
    glDisableVertexAttribArray(a[1]);
    glUseProgram(0);
    if(g->draw_bounds == true)
        glcanvas_draw_bounds(g);
    if(g->draw_axes == true)
        glcanvas_draw_axes(g);
    glPopMatrix();
}
static void
glcanvas_meshes_free(struct glcanvas_t *g) {
    int i = 0;
    struct mesh_t *__mesh = NULL;
    vec_foreach(&g->meshes, __mesh, i) 
        { mesh_free(__mesh); } vec_deinit(&g->meshes);
        vec_compact(&g->meshes);
}
static void
glcanvas_images_free(struct glcanvas_t *g) {
    int i = 0;
    struct image_t *img = NULL;
    vec_foreach(&g->images, img, i) 
        { image_free(img); } vec_deinit(&g->images);
    vec_compact(&g->images);
}
static bool
glcanvas_load_images(struct glcanvas_t *g, vec_image_t *images) {
    glcanvas_images_free(g);
    image_free(g->image);
    struct image_t *img = image_merge(*images);
    GLfloat vertices[30] = {
        img->bounds.xmin, img->bounds.ymin, 0, 0, 0,
        img->bounds.xmax, img->bounds.ymin, 0, 1, 0,
        img->bounds.xmax, img->bounds.ymax, 0, 1, 1,
        img->bounds.xmin, img->bounds.ymin, 0, 0, 0,
        img->bounds.xmax, img->bounds.ymax, 0, 1, 1,
        img->bounds.xmin, img->bounds.ymax, 0, 0, 1
    };
    glBindBuffer(GL_ARRAY_BUFFER, g->tex_vbo);
    glBufferData(GL_ARRAY_BUFFER, 
        sizeof(vertices), vertices, GL_STATIC_DRAW);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, g->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0,
                    GL_RGB, GL_UNSIGNED_BYTE, img->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    float dx = image_dx(img);
    float dy = image_dy(img);
    Vec3f center = Vec3f_add((Vec3f){img->bounds.xmin, img->bounds.ymin, 0}, 
            Vec3f_div((Vec3f){dx, dy, 0}, 2));
    float scale = 4/Vec3f_length(Vec3f_div((Vec3f)(Vec3f){dx, dy, 0}, 2));
    if (g->snap == true) {
        glcanvas_snap_bounds(g, center, scale);
        g->snap = false;
    }    
    g->images = *images;
    g->image = img;
    return true;
}
static bool
glcanvas_load_meshes(struct glcanvas_t *g, mesh_vec_t *meshes) {
    glcanvas_meshes_free(g);
    struct mesh_t *m = vec_first(meshes);
    float x_lower = m->ptr->X.lower;
    float y_lower = m->ptr->Y.lower;
    float z_lower = m->ptr->Z.lower;
    float x_upper = m->ptr->X.upper;
    float y_upper = m->ptr->Y.upper;
    float z_upper = m->ptr->Z.upper;
    for(int i =1; i< meshes->length; i++) {
        m = meshes->data[i];
        x_lower = fmin(x_lower, m->ptr->X.lower);
        y_lower = fmin(y_lower, m->ptr->Y.lower);
        z_lower = fmin(z_lower, m->ptr->Z.lower);
        x_upper = fmax(x_upper, m->ptr->X.upper);
        y_upper = fmax(y_upper, m->ptr->Y.upper);
        z_upper = fmax(z_upper, m->ptr->Z.upper);
    }
    Vec3f min_corner = (Vec3f) {x_lower, y_lower, z_lower };
    Vec3f max_corner = (Vec3f) {x_upper, y_upper, z_upper};
    Vec3f center = (Vec3f) { (min_corner.x+max_corner.x)/2, 
        (min_corner.y+max_corner.y)/2, (min_corner.z+max_corner.z)/2 };
    Vec3f _scale = (Vec3f){(min_corner.x-center.x), 
        (min_corner.y-center.y), (min_corner.z-center.z) };
    glcanvas_reload_vbos(g);
    float scale = 4/sqrt(powf(_scale.x,2) + powf(_scale.y,2) + powf(_scale.z,2));
    g->meshes = *meshes;
    if (g->snap == true) {
        glcanvas_snap_bounds(g, center, scale);
        g->snap = false;
    }
    glcanvas_reload_vbos(g);
    return true;
}
static void
glcanvas_draw(struct glcanvas_t *g) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0627451, 0.0627451, 0.0627451, 1);
    (g->current_3dshader == SUB_SHADER) ?
        glcanvas_draw_flat(g, 50) : glcanvas_draw_mesh(g);
    if (g->path_vbo > 0)
        glcanvas_draw_paths(g);
    if (g->image != NULL) 
        glcanvas_draw_image(g);
    if (g->border.is_set)
        glcanvas_draw_border(g);
    glfwSwapBuffers(g->window);
}
static pt_t
glcanvas_rendertask(void * const ctx) {
    int i = 0;
    float scale = 1;
    struct glcanvas_t *g = ctx;
    struct mathtree_t *t = NULL;
    g->border = Vec3rgb_nil;
    vec_foreach(&g->cad->_shapes, t, i) {
        if (!isnormal(mathtree_dz(t)))
            scale = fmax(scale, pow((1e6/mathtree_dx(t)
        *mathtree_dy(t)),0.5));
    }
    vec_image_t images;
    mesh_vec_t meshes;
    vec_init(&images), vec_init(&meshes);
    switch(g->cad->render_mode) {
        case RENDER_2D: //make_images(g->cad, &images, scale); break;
        case RENDER_3D: {
            vec_foreach(&g->cad->_shapes, t, i) {
                if(mathtree_bounded(t)) {
                    (void)vec_push(&meshes, make_mesh(g->cad, t));
                } else if(isnormal(mathtree_dx(t)) 
                        && isnormal(mathtree_dy(t))) {
                    (void)vec_push(&images, make_flat_image(g->cad, t, scale));
                }
            }
            break;
        }
    };
    glfwMakeContextCurrent(g->window);
    if(images.length > 0) {
        g->redraw = glcanvas_load_images(g, &images);
    } else { vec_deinit(&images); vec_compact(&images);}
    if(meshes.length > 0) {
        g->redraw = glcanvas_load_meshes(g, &meshes);
    } else { vec_deinit(&meshes); vec_compact(&meshes);}

    glfwPostEmptyEvent();

    return PT_DONE;
}
struct glcanvas_t *
glcanvas_create(const char *title, Vec2f dimensions) {
    struct glcanvas_t *g = calloc(1, sizeof(struct glcanvas_t));
    g->snap = true;
    g->scale = 1;
    g->alpha = g->beta = 0;
    g->draw_bounds = g->draw_axes = g->halt = false;
    g->redraw = g->snap = true;
    g->current_3dshader = SHADED_SHADER;
    g->pt = protothread_create_maxpt(1);
    g->L = luaL_newstate();
    luaL_openlibs(g->L);
    luaopen_table_ext(g->L);
    luaopen_mathtree(g->L);
    luaopen_shape(g->L);
    luaopen_text(g->L);
    luaopen_cad(g->L);
    g->cad = koko_fabvars_create(g->L);
    glfwInit();
    glfwSetErrorCallback(glfw_error_callback);
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_FLOATING, GL_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GL_FALSE);
    g->window = glfwCreateWindow(dimensions.x, dimensions.y, title, NULL, NULL);
    glfwMakeContextCurrent(g->window);
    glewInit();
    glfwSwapInterval(1);
    compile_shaders(g);
    glGenTextures(1, &g->texture);
    glGenBuffers(1, &g->tex_vbo);
    vec_init(&g->images);
    vec_init(&g->meshes);
    vec_init(&g->mesh_vbos);
    vec_init(&g->leafs);
    g->mouse = Vec2f_nil;
    g->center = Vec3f_nil;
    g->border = Vec3rgb_nil;
    glfwGetFramebufferSize(g->window, &g->width, &g->height);
    glfwSetWindowUserPointer(g->window, g);
    glfwSetFramebufferSizeCallback(g->window, __glcanvas_resized);
    glfwSetCursorPosCallback(g->window, __glcanvas_mousemove);
    glfwSetMouseButtonCallback(g->window, __glcanvas_mousebutton);
    glfwSetScrollCallback(g->window, __glcanvas_mousescroll);
    glEnable(GL_RGBA);
    glEnable(GL_DOUBLEBUFFER);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    __glcanvas_resized(g->window, g->width, g->height);
    return g;
}
bool 
glcanvas_reeval(struct glcanvas_t *g, const char *script) {
    bool eval = false;
    g->path_vbo = -1;
    vec_clear(&g->cad->_shapes);
    glcanvas_meshes_free(g);
    glcanvas_mesh_vbos_free(g);
    image_free(g->image);
    glcanvas_images_free(g);
    g->image = NULL;
    switch(luaL_dostring(g->L, script)) {
        case 0: {
            pt_create(g->pt, &g->pt_thread, glcanvas_rendertask, g);
            protothread_quiesce(g->pt);
            eval = true;
            break;
        }
        default : {
            g->border = Vec3rgb_color(255, 0, 0);
            break;
        }
    };
    g->redraw = true;
    glfwPostEmptyEvent();
    return eval;
}
void 
glcanvas_poll(struct glcanvas_t *g){
    glfwWaitEvents();
    if (g->halt)
        glfwSetWindowShouldClose(g->window, GL_TRUE);
    if(g->redraw == true)
        glcanvas_draw(g);
    g->redraw = false;
}
void 
glcanvas_run(struct glcanvas_t *g){
    while (!glfwWindowShouldClose(g->window)) {
        glcanvas_poll(g);
    }
}
