#include <lua.hpp>
#include "../modules.h"
#include "ssengine/core.h"
#include "../core.h"
#include "ssengine/render/resources.h"

static int resource_meta_gc(lua_State *L){
    ss_resource_ref*& res = ss_lua_check_resource_ref(L, 1);
    ss_resource_release(ss_lua_get_core_context(L), res);
    res = nullptr;
    return 0;
}

static int resource_meta_load(lua_State *L){
    ss_resource_ref* res = ss_lua_check_resource_ref(L, 1);
    lua_pushinteger(L, ss_resource_load(ss_lua_get_core_context(L), res));
    return 1;
}

void ss_lua_push_resource_ref(lua_State* L, struct ss_resource_ref* res){
    ss_resource_ref** ptr = reinterpret_cast<ss_resource_ref**>(lua_newuserdata(L, sizeof(ss_resource_ref*)));
    *ptr = res;
    ss_resource_addref(ss_lua_get_core_context(L), res);

    if (luaL_newmetatable(L, "Resource") == 1){
        luaL_reg resource_methods[] = {
            { "load", resource_meta_load },
            { "__gc", resource_meta_gc }, 
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
    lua_setmetatable(L, -2);
}

static int resource_get(lua_State *L){
    lua_pushvalue(L, 1);
    lua_gettable(L, lua_upvalueindex(1));
    if (!lua_isuserdata(L, -1)){
        luaL_error(L, "Unsupported resource type %s", lua_tostring(L, 1));
    }
    resource_loader_func func = (resource_loader_func)lua_touserdata(L, -1);
    
    auto C = ss_lua_get_core_context(L);
    ss_resource_ref* res = func(C, luaL_checkstring(L, 2));
    ss_lua_push_resource_ref(L, res);
    ss_resource_release(C, res);
    return 1;
}

extern "C" SS_CORE_API ss_resource_ref* ss_texture2d_resource(ss_core_context* C, const char* uri);

int ss_module_resource(lua_State *L) {
    luaL_reg resource_funcs[] = {
        { "get", resource_get },
        { NULL, NULL }
    };

    // Create module table.
    lua_createtable(L, 0, 2);

    // Create resource type map
    lua_createtable(L, 0, 1);
    {
        // Initialize built-in resource loaders.
        lua_pushlightuserdata(L, (resource_loader_func)ss_texture2d_resource);
        lua_setfield(L, -2, "texture2d");
    }
    
    for (size_t i = 0; resource_funcs[i].func != nullptr; ++i){
        lua_pushvalue(L, -1);
        lua_pushcclosure(L, resource_funcs[i].func, 1);
        lua_setfield(L, -3, resource_funcs[i].name);
    }

    lua_setfield(L, -2, "loaders");

    return 1;
}