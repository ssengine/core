#include "core.h"

#include <lua.hpp>

#include "modules.h"
#include <assert.h>

#include <ssengine/macros.h>
#include <ssengine/log.h>

void ss_lua_preload_module(lua_State *L, const char* name, lua_CFunction func){
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, name);
	lua_pop(L, 2);
}

extern "C" int ss_module_uri_loader(lua_State *L);

static void ss_init_lua_libs(lua_State *L){
	luaL_openlibs(L);
	ss_lua_preload_module(L, "log", ss_module_log);
	
	lua_pushcfunction(L, ss_module_uri_loader);
	lua_call(L, 0, 0);

}

static int s_tag_core_context = 0;

void _ss_init_script_context(ss_core_context* C){
	lua_State* L = C->L = luaL_newstate();
	
	lua_pushlightuserdata(L, &s_tag_core_context);
	lua_pushlightuserdata(L, C);
	lua_rawset(L, LUA_REGISTRYINDEX);

	ss_init_lua_libs(L);
}

void _ss_destroy_script_context(ss_core_context* C){
	lua_close(C->L);
	C->L = NULL;
}

lua_State* ss_get_script_context(ss_core_context* C){
	return C->L;
}

ss_core_context* ss_lua_get_core_context(lua_State* L){
	lua_pushlightuserdata(L, &s_tag_core_context);
	lua_rawget(L, LUA_REGISTRYINDEX);
	void* ret = lua_touserdata(L, -1);
	lua_pop(L, 1);
	return (ss_core_context*)(ret);
}

void ss_run_script_from_macro(ss_core_context* C, const char* name, int nargs, int nrets){
	lua_State* L = C->L;
	
	ss_macro_eval(C, name);
	if (luaL_loadstring(L, ss_macro_get_content(C, name).c_str()) != 0){
		// with Error
		SS_LOGE("%s", lua_tostring(L, -1));
		lua_pop(L, 1);
		return;
	}
	if (nargs){
		lua_insert(L, -nargs - 1);
	}
	ss_safe_call(L, nargs, nrets);
}

static int tag_error_handler = 0;

void ss_safe_call(lua_State* L, int nargs, int nrets){
	// get error handler
	lua_pushlightuserdata(L, &tag_error_handler);
	lua_rawget(L, LUA_REGISTRYINDEX);

	int base = lua_gettop(L) - nargs - 1;

	if (lua_isnil(L, -1)){
		// call without a handler(error logged).
		lua_pop(L, 1);
		if (lua_pcall(L, nargs, nrets, 0) != 0){
			// with errors
			SS_LOGE("%s", lua_tostring(L, -1));
			lua_pop(L, 1);
			lua_settop(L, base + nrets);
		}
	}
	else {
		lua_insert(L, -nargs - 2);
		if (lua_pcall(L, nargs, nrets, 0) != 0){
			// Clear all return values from error handler.
			lua_settop(L, base);
			lua_settop(L, base + nrets);
		}
	}
}
