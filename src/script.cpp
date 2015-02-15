#include "core.h"

#include <lua.hpp>

#include "modules.h"
#include <assert.h>

#include <ssengine/macros.h>
#include <ssengine/log.h>

//TODO: avoid to use static variable here.
static lua_State* s_context = NULL;

void ss_preload_module(const char* name, lua_CFunction func){
	lua_State* L = s_context;
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, name);
	lua_pop(L, 2);
}

static void ss_init_lua_libs(lua_State *L){
	luaL_openlibs(L);
	ss_preload_module("log", ss_module_log);
}

lua_State* ss_init_script_context(){
	assert(s_context == NULL);
	s_context = luaL_newstate();

	ss_init_lua_libs(s_context);
	return s_context;
}

void ss_destroy_script_context(){
	assert(s_context != NULL);
	lua_close(s_context);
	s_context = NULL;
}

lua_State* ss_get_script_context(){
	assert(s_context != NULL);
	return s_context;
}

void ss_run_script_from_macro(const char* name, int nargs, int nrets){
	lua_State* L = s_context;
	
	ss_macro_eval(name);
	if (luaL_loadstring(L, ss_macro_get_content(name).c_str()) != 0){
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
		// call without a handler(error ignored).
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
