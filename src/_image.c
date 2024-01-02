#include "_image.h"

struct image_t *
image_create(int w, int h, int channels, image_dtype_t dtype) {
    #define __image_create(t) \
    {\
        if (channels != 1 && channels != 3) return NULL; \
        struct image_t * i = calloc(1, sizeof(struct image_t)); \
        i->data = calloc(w*h*channels, sizeof(t)); \
        i->width = w; \
        i->height = h; \
        i->channels = channels; \
        i->dtype = dtype; \
        return i; \
    };
    switch(dtype) {
        case DT_IU8: __image_create(uint8_t);
        case DT_IU16: __image_create(uint16_t);
        case DT_IU32: __image_create(uint32_t);
        case DT_FLOAT: __image_create(float);
        default: return NULL;
    }
}
void
image_free(struct image_t *i) {
    if(i) {
        if(i->data != NULL) free(i->data);
        if(i->pixels != NULL) free(i->pixels);
        free(i);
        i = NULL;
    }
}
float
image_dx(struct image_t *i) {
    return i->bounds.xmax-i->bounds.xmin;
}
float
image_dy(struct image_t *i) {
    return i->bounds.ymax-i->bounds.ymin;
}
float
image_dz(struct image_t *i) {
    return i->bounds.zmax-i->bounds.zmin;
}
void
image_pixels(struct image_t *src) {
    #define __image_pixels(t) \
    {\
        int h = src->height, w = src->width, c = src->channels; \
        if(src->pixels != NULL)\
            free(src->pixels);\
        src->pixels = calloc(h*c, sizeof(t *)); \
        for(int i = 0; i < h; i++) {\
            src->pixels[h-i-1] = &((typeof(t) *)src->data)[i*w*c];\
        }\
        break;\
    };
    switch(src->dtype) {
        case DT_IU8: __image_pixels(uint8_t);
        case DT_IU16: __image_pixels(uint16_t);
        case DT_IU32: __image_pixels(uint32_t);
        case DT_FLOAT: __image_pixels(float);
        default: return;
    }
}
void
image_flipped_pixels(struct image_t *src) {
    #define __image_flipped_pixels(t) \
    {\
        int h = src->height, w = src->width, c = src->channels;\
        if(src->pixels != NULL)\
            free(src->pixels);\
        src->pixels = calloc(h*c, sizeof(t *)); \
        for(int i = 0; i < h; i++) {\
            src->pixels[i] = &((typeof(t) *)src->data)[i*w*c];\
        }\
        break;\
    };
    switch(src->dtype) {
        case DT_IU8: __image_flipped_pixels(uint8_t);
        case DT_IU16: __image_flipped_pixels(uint16_t);
        case DT_IU32: __image_flipped_pixels(uint32_t);
        case DT_FLOAT: __image_flipped_pixels(float);
        default: return;
    }
}

typedef void (array_func) (void *, void *, int);

static void 
SHORT_to_UBYTE(uint16_t *ip, uint8_t *op, int n) {
    for(int i=0; i<n; i++) {
        op[i] = (uint8_t)ip[i];
    }
}
static void 
SHORT_to_SHORT(uint16_t *ip, uint16_t *op, int n) {
    for(int i=0; i<n; i++) {
        op[i] = (uint16_t)ip[i];
    }
}
static void 
SHORT_to_INT32(uint16_t *ip, uint32_t *op, int n) {
    for(int i=0;i<n;i++,ip+=1,op+=1) {
        op[i] = (uint32_t)ip[i];
    }
}
static void 
SHORT_to_FLOAT(uint16_t *ip, float *op, int n) {
    for(int i=0;i<n;i++,ip+=1,op+=1) {
        op[i] = (float)ip[i];
    }
}
static array_func *__short_to[DT_NTYPES] = {
    (array_func *)SHORT_to_UBYTE, 
    (array_func *)SHORT_to_SHORT,
    (array_func *)SHORT_to_INT32, 
    (array_func *)SHORT_to_FLOAT
};
struct image_t * 
image_copy(struct image_t *src, int channels, image_dtype_t dtype) {
    #define __image_copy(t) \
    {\
        struct image_t *i = calloc(1, sizeof(struct image_t));\
        int w = src->width, h = src->height, c = src->channels, s = w*h*c; \
        i->data = calloc(s, sizeof(t));\
        __short_to[dtype](src->data, i->data, s);\
        i->width = w;\
        i->height = h;\
        i->channels = channels;\
        i->dtype = dtype;\
        i->bounds = src->bounds;\
        i->color = (src->color.is_set) ? src->color : Vec3rgb_nil;\
        return i;\
    };
    switch(dtype) {
        case DT_IU8: __image_copy(uint8_t); break;
        case DT_IU16: __image_copy(uint16_t); break;
        case DT_IU32: __image_copy(uint32_t); break;
        case DT_FLOAT: __image_copy(float); break;
        default: return NULL;
    }
}
struct image_t *
image_merge(vec_image_t images) {
    if(!images.length) return NULL;
    int i = 0;
    float largest = 0,
            resolution = 0;
    struct image_t *src;
    struct image_t *img = vec_first(&images);
    float xmin = img->bounds.xmin;
    float xmax = img->bounds.xmax;
    float ymin = img->bounds.ymin;
    float ymax = img->bounds.ymax;
    for(int i=1; i<images.length; i++) {
        img = images.data[i];
        xmin = fmin(xmin, img->bounds.xmin);
        xmax = fmax(xmax, img->bounds.xmax);
        ymin = fmin(ymin, img->bounds.ymin);
        ymax = fmax(ymax, img->bounds.ymax);
    }
    vec_foreach(&images, src, i) {
        if (src->width > largest) {
            resolution = src->width/image_dx(src);
            largest = src->width;
        } else if (src->height > largest) {
            resolution = src->height/image_dy(src);
            largest = src->height;
        }
    }
    struct image_t *out = image_create(
            (int)((xmax-xmin)*resolution),
            (int)((ymax-ymin)*resolution), 3, DT_IU8);
    out->bounds.xmin = xmin, out->bounds.xmax = xmax;
    out->bounds.ymin = ymin, out->bounds.ymax = ymax;
    out->bounds.zmin = vec_first(&images)->bounds.zmin, out->bounds.zmax = vec_first(&images)->bounds.zmax;
    struct image_t *depth = image_create(out->width, out->height, 1, DT_IU8);
    vec_foreach(&images, src, i) {
        src = image_copy(src, src->channels, DT_IU8);
        int x  = fmax(0, (int)((src->bounds.xmin - out->bounds.xmin)*resolution));
        int ni = fmin(out->width - x, src->width);
        int y  = fmax(0, (int)((src->bounds.ymin - out->bounds.ymin)*resolution));
        int nj = fmin(out->height - y, src->height);
        float R = 1, G = 1, B = 1;
        if(src->color.is_set == true) {
            R = src->color.r/255., G = src->color.g/255., 
                B = src->color.b/255.;
        }
        image_pixels(src);
        image_pixels(depth);
        image_pixels(out);
        depth_blit((const uint8_t **)src->pixels, 
                (uint8_t **)depth->pixels, (uint8_t (**)[3])out->pixels,
            x, y, ni, nj,
            R, G, B
        );
        image_free(src);
    }
    image_free(depth);
    return out;
}
struct image_t *
image_load(const char *filename) {
    float dx, dy, dz;
    int ni, nj;
    load_png_stats(filename, &ni, &nj, &dx, &dy, &dz);
    struct image_t *i = image_create(ni, nj, 1, DT_IU16);
    if (isnan(dx)) {
        printf("Assuming 72 dpi for x resolution.\n");
        i->bounds.xmin = 0;
        i->bounds.xmax = 72*i->width/25.4;
    } else { i->bounds.xmin = 0; i->bounds.xmax = dx; }
    if (isnan(dy)) {
        printf("Assuming 72 dpi for y resolution.\n");
        i->bounds.ymin = 0;
        i->bounds.ymax = 72*i->height/25.4;
    } else {
        i->bounds.ymin = 0;
        i->bounds.ymax = dy;
    }
    if (!isnan(dz)) { i->bounds.zmin = 0; i->bounds.zmax = dz; }
    image_pixels(i);
    load_png(filename, (uint16_t **)i->pixels);
    i->filename = filename;
    return i;
}
void
image_save(struct image_t *i, const char *filename) {
    if (i->channels == 3) {
        //a->wximg.SaveFile(filename, wx.BITMAP_TYPE_PNG)
    } else {
        struct image_t *img = image_copy(i, 1, DT_IU16);
        float bounds[6] = {
            i->bounds.xmin, i->bounds.ymin, i->bounds.zmin,
            i->bounds.xmax, i->bounds.ymax, i->bounds.zmax
        };
        image_flipped_pixels(img);
        save_png16L(filename, 
            i->width, i->height, bounds, (const uint16_t **)img->pixels);
        image_free(img);
    }
}
float
image_pixels_per_mm(struct image_t *i) {
    float dx = image_dx(i);
    float dy = image_dy(i);
    if ((i->width > i->height) && (dx != 0)) {
        return i->width/dx;
    } else if (dy != 0 ) {
        return i->height/dy;
    }
    else return 0;
}