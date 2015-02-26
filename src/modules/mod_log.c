#include "../modules.h"
#include <ssengine/log.h>

#include <lua.h>
#include <lauxlib.h>

static int dolog(int level, lua_State* L){
	const char* str = luaL_checkstring(L, 1);
	ss_log(level, "%s", str);
	return 0;
}

#define DEFINE_LOG_FUNC(name,level) \
	static int name(lua_State*L){	\
		return dolog(level, L);		\
	}

DEFINE_LOG_FUNC(log_trace, SS_LL_TRACE);
DEFINE_LOG_FUNC(log_info, SS_LL_INFO);
DEFINE_LOG_FUNC(log_warn, SS_LL_WARN);
DEFINE_LOG_FUNC(log_error, SS_LL_ERROR);
DEFINE_LOG_FUNC(log_fatal, SS_LL_FATAL);

static luaL_Reg log_funcs[] = {
	{ "trace",	log_trace },
	{ "info",	log_info },
	{ "warn",	log_warn },
	{ "error",	log_error },
	{ "fatal",	log_fatal },
	{ NULL, NULL }
};

int ss_module_log(lua_State* L){
#if LUA_VERSION_NUM >= 502
    luaL_newlib(L, log_funcs);
    lua_pushvalue(L, -1); /* make "log" global */
    lua_setglobal(L, "log");
#else
	luaL_register(L, "log", log_funcs);
#endif
	return 1;
}