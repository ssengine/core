#pragma once

#if defined(__cplusplus)
extern "C"{
#endif

#include <lua.h>

// Modules that are preloaded.
int ss_module_log(lua_State* L);
int ss_module_render2d(lua_State* L);
int ss_module_node2d(lua_State* L);

// Modules that are executed.
int ss_module_uri_loader(lua_State *L);

// Modules that are preloaded and required
int ss_module_resource(lua_State *L);

#if defined(__cplusplus)
}
#endif