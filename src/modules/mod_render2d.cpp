#include <lua.hpp>
#include "../modules.h"
#include "ssengine/render/drawbatch.h"
#include "ssengine/core.h"
#include "ssengine/render/device.h"
#include "../decoders/decoder.h"
#include "../core.h"
#include "ssengine/render/resources.h"

#define get_image(L, index) (ss_userdata_image*)lua_touserdata(L, index)

static int ll_load_image(lua_State *L) {
	ss_userdata_image *img = get_image(L, 1);
	ss_resource_ref_impl *ref = wrap(reinterpret_cast<ss_resource_ref*>(img->pointer));
	const char *uri = ref->uri.c_str();

	ss_init_decoder();
	if (ss_decode_image(uri, img) != 0) {
		return luaL_error(L, "load image failed. the image is:%s", uri);
	}
	ss_core_context *C = ss_lua_get_core_context(L);
	img->gen_texture(C);
	return 0;
}

static int ll_unload_image(lua_State *L) {
	ss_userdata_image *img = get_image(L, 1);
	ss_resource_ref *ref = reinterpret_cast<ss_resource_ref*>(img->pointer);
	ss_core_context *C = ss_lua_get_core_context(L);
	ref->prototype->unload(C, ref);
	return 0;
}

static int ll_draw_image(lua_State *L) {
	ss_userdata_image *img = get_image(L, 1);
	ss_resource_ref *ref = reinterpret_cast<ss_resource_ref*>(img->pointer);
	ss_texture2d *texture = (ss_texture2d*)ref->ptr;
	if (texture == nullptr) {
		return luaL_error(L, "draw image failed cause by nil texture, maybe you should load first!");
	}
	ss_core_context *C = ss_lua_get_core_context(L);
	ss_db_draw_image_rect(C, texture, -1, 1, 2, 2, 0, 0, 1, 1);
	return 0;
}

static int ll_create_image(lua_State *L) {
	const char *uri = luaL_checkstring(L, 1);
	ss_core_context *C = ss_lua_get_core_context(L);
	ss_texture_resource_ref *texture = ss_texture_resource(C, uri);

	ss_userdata_image *img = (ss_userdata_image*)lua_newuserdata(L, sizeof(ss_userdata_image));
	img->init();
	img->pointer = texture;
	
	luaL_newmetatable(L, "image_meta");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	
	lua_pushstring(L, "load");
	lua_pushcfunction(L, ll_load_image);
	lua_rawset(L, -3);

	lua_pushstring(L, "unload");
	lua_pushcfunction(L, ll_unload_image);
	lua_rawset(L, -3);

	lua_pushstring(L, "draw");
	lua_pushcfunction(L, ll_draw_image);
	lua_rawset(L, -3);

	lua_setmetatable(L, -2);

	return 1;
}

static int ll_draw_line(lua_State *L) {
	float x0 = luaL_checknumber(L, 1);
	float y0 = luaL_checknumber(L, 2);
	float x1 = luaL_checknumber(L, 3);
	float y1 = luaL_checknumber(L, 4);

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
		"createImage", ll_create_image,
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