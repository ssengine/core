#include <lua.hpp>
#include "../modules.h"
#include "ssengine/render/drawbatch.h"
#include "ssengine/core.h"
#include "ssengine/render/device.h"
#include "../decoders/decoder.h"

static int ll_drawLine(lua_State *L) {
	float x0 = luaL_checknumber(L, 1);
	float y0 = luaL_checknumber(L, 2);
	float x1 = luaL_checknumber(L, 3);
	float y1 = luaL_checknumber(L, 4);

	ss_core_context *C = ss_lua_get_core_context(L);
	ss_db_draw_line(C, x0, y0, x1, y1);
	
	return 0;
}

static int ll_loadImage(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	ss_init_decoder();
	ss_image_data *data = new ss_image_data();
	if (data == nullptr || ss_decode_image(path, data) != 0) {
		return luaL_error(L, "load image failed. the image is:%s", path);
	}
	ss_core_context *C = ss_lua_get_core_context(L);
	data->gen_texture(C);
	lua_pushlightuserdata(L, data);
	return 1;
}

static int ll_disposeImage(lua_State *L) {
	ss_image_data *data = (ss_image_data*)lua_touserdata(L, 1);
	if (data != nullptr) {
		delete data;
	}
	return 0;
}

//TODO : need a comfortable interface. (subRect, x, y)
static int ll_drawImageRect(lua_State *L) {
	ss_image_data *data = (ss_image_data*)lua_touserdata(L, 1);
	float l = luaL_checknumber(L, 2);
	float t = luaL_checknumber(L, 3);
	float w = luaL_checknumber(L, 4);
	float h = luaL_checknumber(L, 5);
	float tl = luaL_checknumber(L, 6);
	float tt = luaL_checknumber(L, 7);
	float tw = luaL_checknumber(L, 8);
	float th = luaL_checknumber(L, 9);

	ss_core_context *C = ss_lua_get_core_context(L);
	ss_db_draw_image_rect(C, data->texture, t, w, w, h, tl, tt, tw, th);
	return 0;
}

//TODO : need a comfortable interface. (x, y)
static int ll_drawImage(lua_State *L) {
	ss_image_data *data = (ss_image_data*)lua_touserdata(L, 1);

	ss_core_context *C = ss_lua_get_core_context(L);
	ss_db_draw_image_rect(C, data->texture, -1, 1, 2, 2, 0, 0, 1, 1);
	return 0;
}

static int ll_flush(lua_State *L) {
	ss_core_context *C = ss_lua_get_core_context(L);
	ss_db_flush(C);
	return 0;
}

int ss_module_render2d(lua_State *L) {
	luaL_reg render2d_funcs[] = {
		"drawLine", ll_drawLine,
		"loadImage", ll_loadImage,
		"disposeImage", ll_disposeImage,
		"drawImageRect", ll_drawImageRect,
		"drawImage", ll_drawImage,
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