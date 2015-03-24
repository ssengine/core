#include <lua.hpp>
#include "../modules.h"
#include "ssengine/render/drawbatch.h"
#include "ssengine/core.h"
#include "ssengine/render/device.h"
#include "../core.h"
#include "ssengine/render/resources.h"

static int ll_draw_line(lua_State *L) {
	float x0 = (float)luaL_checknumber(L, 1);
    float y0 = (float)luaL_checknumber(L, 2);
    float x1 = (float)luaL_checknumber(L, 3);
    float y1 = (float)luaL_checknumber(L, 4);

	ss_core_context *C = ss_lua_get_core_context(L);
	ss_db_draw_line(C, x0, y0, x1, y1);

	return 0;
}

static int ll_flush(lua_State *L) {
	ss_core_context *C = ss_lua_get_core_context(L);
	ss_db_flush(C);
	return 0;
}

int ss_module_render2d(lua_State *L) {
	luaL_reg render2d_funcs[] = {
		"drawLine", ll_draw_line,
		"flush", ll_flush,
		NULL, NULL
	};

#if LUA_VERSION_NUM >= 502
	luaL_newlib(L, render2d_funcs);
#else
	luaL_register(L, "render2d", render2d_funcs);
#endif
	return 1;
}