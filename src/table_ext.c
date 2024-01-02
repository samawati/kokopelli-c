#include "common.h"

static int 
lua_table_count_ext (lua_State *L) {
  size_t n = 0;
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_pushnil(L);
  while (lua_next(L, 1) != 0) {
    n++;
    lua_pop(L, 1);
  }
  lua_pushnumber(L, n);
  return 1;
}
static int 
lua_table_filter_ext (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checktype(L, 2, LUA_TFUNCTION);
  lua_newtable(L);
  lua_pushnil(L);
  while (lua_next(L, 1)) {
    lua_pushvalue(L, 2);
    lua_pushvalue(L, -2);
    lua_call(L, 1, 1);
    if (lua_toboolean(L, -1)) {
      lua_pop(L, 1);
      lua_pushvalue(L, -2);
      lua_pushvalue(L, -2);
      lua_rawset(L, 3);
      lua_pop(L, 1);   
    } else lua_pop(L, 2);
  }
  return 1;
}
static int 
lua_table_foreach_ext (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checktype(L, 2, LUA_TFUNCTION);
  lua_pushnil(L);
  while (lua_next(L, 1)) {
    lua_pushvalue(L, 2);
    lua_pushvalue(L, -3);
    lua_pushvalue(L, -3);
    lua_call(L, 2, 1);
    if (!lua_isnil(L, -1))
      return 1;
    lua_pop(L, 2);
  }
  return 0;
}
static int 
lua_table_reduce_ext (lua_State *L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_pushnil(L);
    lua_pushnil(L);
    while (lua_next(L, 2)) {
        if(lua_tonumber(L, -2) == 1) {
            lua_pushvalue(L, -2);
            lua_remove(L, -3);
            lua_remove(L, -3);
        } else {
            lua_pushvalue(L, 1);
            lua_pushvalue(L, -4);
            lua_pushvalue(L, -3);
            lua_call(L, 2, 1);
            lua_remove(L, -4);
            lua_remove(L, -2);
            lua_pushvalue(L, -2);
            lua_remove(L, -3);
        }
    }
    return 1;
}
static const 
luaL_Reg lib[] = {
  {"count", lua_table_count_ext },
  {"filter", lua_table_filter_ext },
  {"foreach", lua_table_foreach_ext },
  {"reduce", lua_table_reduce_ext },
  {NULL, NULL}
};
LUALIB_API int 
luaopen_table_ext (lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
  luaL_checktype(L, -1, LUA_TTABLE);
  lua_getfield(L, -1, "table");
  luaL_checktype(L, -1, LUA_TTABLE);

  luaL_register(L, NULL, lib);
  return 1;
}
