#ifndef _COMMON_H
#define _COMMON_H

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stddef.h>
#include <unistd.h>
#include <stdint.h>

#include <lua5.1/lauxlib.h>
#include <lua5.1/lua.h>
#include <lua5.1/lualib.h>

#include "vec.h"
#include "util_vec3f.h"

#include "pt.h"

#ifdef __cplusplus
#define KOKO_API extern "C"
#else 
#define KOKO_API

#endif

struct bounds_t {
    float xmin, xmax, 
        ymin, ymax, zmin, zmax;
};
typedef struct Vec3rgb {
    bool is_set;
    uint8_t r, g, b;
} Vec3rgb;

LUALIB_API int luaopen_table_ext(lua_State *L);

#define Vec3rgb_nil (Vec3rgb){ .is_set=false, .r=0, .g=0, .b=0}
#define Vec3rgb_color(red, green, blue) (Vec3rgb){ .is_set=true, .r=red, .g=green, .b=blue}

#define Vec3f_nil (Vec3f){0, 0, 0}

typedef struct Vec2f { float x, y; } Vec2f;
#define Vec2f_nil (Vec2f) { .x = 0, .y = 0 }

static inline Vec2f
Vec2f_sub(Vec2f v2a, Vec2f v2b) {
    return (Vec2f) { (v2a.x-v2b.x), (v2a.y-v2b.y) }; 
}
#define Vec3f_zero(v3) v3.x = 0, v3.y = 0, v3.z = 0;
static inline Vec3f 
Vec3f_div(Vec3f v3, float r) { 
    return (Vec3f) { v3.x/r, v3.y/r, v3.z/r};
}
static inline Vec3f
Vec3f_add(Vec3f v3a, Vec3f v3b) {
    return (Vec3f) { (v3a.x+v3b.x), (v3a.y+v3b.y), (v3a.z+v3b.z) };
}
static inline Vec3f
Vec3f_sub(Vec3f v3a, Vec3f v3b) {
    return (Vec3f) { (v3a.x-v3b.x), (v3a.y-v3b.y), (v3a.z-v3b.z) };
}
static inline float
Vec3f_length(Vec3f v3) {
    return sqrt(powf(v3.x,2) + powf(v3.y,2) + powf(v3.z,2));
}
static inline void 
dumpstack(lua_State* L) {
    int top = lua_gettop(L);
    for (int i = 1; i <= top; i++) {
        printf("%d\t%s\t", i, luaL_typename(L, i));
        switch (lua_type(L, i)) {
        case LUA_TNUMBER:
            printf("%g\n", lua_tonumber(L, i));
            break;
        case LUA_TSTRING:
            printf("%s\n", lua_tostring(L, i));
            break;
        case LUA_TBOOLEAN:
            printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
            break;
        case LUA_TNIL:
            printf("%s\n", "nil");
            break;
        default:
            printf("%p\n", lua_topointer(L, i));
            break;
        }
    }
}
  
#endif