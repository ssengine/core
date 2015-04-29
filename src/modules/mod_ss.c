#include "../modules.h"

#include <ssengine/core.h>
#include <lauxlib.h>

static int modss_load_plugin(lua_State *L){
    int err = ss_load_plugin(ss_lua_get_core_context(L), luaL_checkstring(L, 1));
    if (err){
        luaL_error(L, "Failed to load plugin with name `%s`: %d", lua_tostring(L, 1), err);
    }
    return 0;
}

int ss_module_ss(lua_State* L){
    luaL_Reg ss_funcs[] = {
        { "loadPlugin", modss_load_plugin },
        { NULL, NULL }
    };

#if LUA_VERSION_NUM >= 502
    luaL_newlib(L, ss_funcs);
    lua_pushvalue(L, -1); /* make "log" global */
    lua_setglobal(L, "ss");
#else
    luaL_register(L, "ss", ss_funcs);
#endif
    return 1;
}