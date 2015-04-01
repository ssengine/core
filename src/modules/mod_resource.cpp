#include <lua.hpp>
#include "../modules.h"
#include "ssengine/core.h"
#include "../core.h"
#include "ssengine/render/resources.h"

static int ss_resource_meta_gc(lua_State *L){
    ss_resource_ref*& res = ss_lua_check_resource_ref(L, 1);
    ss_resource_release(ss_lua_get_core_context(L), res);
    res = nullptr;
    return 0;
}

void ss_lua_push_resource_ref(lua_State* L, struct ss_resource_ref* res){
    ss_resource_ref** ptr = reinterpret_cast<ss_resource_ref**>(lua_newuserdata(L, sizeof(ss_resource_ref*)));
    *ptr = res;
    ss_resource_addref(ss_lua_get_core_context(L), res);

    if (luaL_newmetatable(L, "Resource") == 1){
        luaL_reg resource_methods[] = {
            { "__gc", ss_resource_meta_gc }, 
            { NULL, NULL }
        };

        for (size_t i = 0; resource_methods[i].func != nullptr; ++i){
            lua_pushcfunction(L, resource_methods[i].func);
            lua_setfield(L, -2, resource_methods[i].name);
        }
        // set self as "__index" metamethod.
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
    }
}



int ss_module_resource(lua_State *L) {
    luaL_reg resource_funcs[] = {
        { NULL, NULL }
    };

#if LUA_VERSION_NUM >= 502
    luaL_newlib(L, resource_funcs);
#else
    luaL_register(L, "resource", resource_funcs);
#endif

    return 1;
}