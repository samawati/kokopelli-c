#ifndef LIBFAB_SHAPE_H
#define LIBFAB_SHAPE_H

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

typedef enum text_align_t { TEXT_LEFT, TEXT_RIGHT, TEXT_CENTERED } text_align_t;

LUALIB_API struct fabvars_t *koko_fabvars_create(lua_State *L);

LUALIB_API int luaopen_mathtree(lua_State *L);
LUALIB_API int luaopen_shape(lua_State * L);
LUALIB_API int luaopen_cad(lua_State *L);
LUALIB_API int luaopen_text(lua_State * L);

#endif