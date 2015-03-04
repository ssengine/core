#pragma once

#if defined(__cplusplus)
extern "C"{
#endif

#include <lua.h>

int ss_module_log(lua_State* L);
int ss_module_render2d(lua_State* L);

#if defined(__cplusplus)
}
#endif