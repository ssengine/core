#include "stage2d.h"

#include <lua.hpp>
#include "../modules.h"
#include "../core.h"

inline struct ss_node2d*& ss_lua_check_node2d_root(lua_State* L, int pos){
    return *reinterpret_cast<struct ss_node2d**>(luaL_checkudata(L, pos, "node2d.Root"));
}

static int node2d_root_meta_gc(lua_State *L){
    ss_node2d*& res = ss_lua_check_node2d_root(L, 1);
    if (res != nullptr){
        delete res;
    }
    res = nullptr;
    return 0;
}

static void ss_lua_push_node2d_root(lua_State *L, ss_node2d* root){
    ss_node2d** ptr = reinterpret_cast<ss_node2d**>(lua_newuserdata(L, sizeof(ss_resource_ref*)));
    *ptr = root;

    if (luaL_newmetatable(L, "node2d.Root") == 1){
        luaL_reg mt_methods[] = {
            { "__gc", node2d_root_meta_gc },
            { NULL, NULL }
        };

        for (size_t i = 0; mt_methods[i].func != nullptr; ++i){
            lua_pushcfunction(L, mt_methods[i].func);
            lua_setfield(L, -2, mt_methods[i].name);
        }
    }
    lua_setmetatable(L, -2);
}

static ss_node2d* ss_lua_get_node2d(lua_State*L, int pos){
    if (lua_islightuserdata(L, 1)){
        return (ss_node2d*)lua_touserdata(L, 1);
    }
    else {
        return ss_lua_check_node2d_root(L, 1);
    }
}

static ss_node2d* ss_lua_get_node2d_child(lua_State*L, int pos){
    if (lua_islightuserdata(L, 1)){
        ss_node2d* self = (ss_node2d*)lua_touserdata(L, 1);
        if (!self->get_parent()){
            luaL_error(L, "Argument %d should not be a root node.", pos);
        }
        return self;
    }
    luaL_error(L, "Argument %d should be a light userdata.", pos);
    return NULL;
}

static int node2d_create_root(lua_State *L){
    ss_lua_push_node2d_root(L, new ss_node2d());
    return 1;
}

static int node2d_get_root(lua_State *L){
    ss_node2d*& res = ss_lua_check_node2d_root(L, 1);
    lua_pushlightuserdata(L, res);
    return 1;
}

static int node2d_new_child(lua_State *L){
    ss_node2d* parent = ss_lua_get_node2d(L, 1);
    ss_node2d* child = new ss_node2d();
    parent->insert_last(child);
    lua_pushlightuserdata(L, child);
    return 1;
}

static int node2d_remove(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d_child(L, 1);
    self->remove();
    delete self;
    return 0;
}

static int node2d_detach(lua_State *L){
    ss_node2d* self = (ss_node2d*)ss_lua_get_node2d_child(L, 1);
    self->remove();
    ss_lua_push_node2d_root(L, self);   
    return 1;
}

static int node2d_calc(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d(L, 1);
    self->calc();
    return 1;
}

static int node2d_prev(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d_child(L, 1);
    ss_node2d* ret = self->get_prev();
    if (ret)
        lua_pushlightuserdata(L, ret);
    else
        lua_pushnil(L);
    return 1;
}

static int node2d_next(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d_child(L, 1);
    ss_node2d* ret = self->get_next();
    if (ret)
        lua_pushlightuserdata(L, ret);
    else
        lua_pushnil(L);
    return 1;
}

static int node2d_first_child(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d(L, 1);
    ss_node2d* ret = self->get_first_child();
    if (ret)
        lua_pushlightuserdata(L, ret);
    else
        lua_pushnil(L);
    return 1;
}

static int node2d_last_child(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d(L, 1);
    ss_node2d* ret = self->get_last_child();
    if (ret)
        lua_pushlightuserdata(L, ret);
    else
        lua_pushnil(L);
    return 1;
}

static int node2d_setx(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d(L, 1);
    self->set_x((float)luaL_checknumber(L, 2));
    return 0;
}

static int node2d_sety(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d(L, 1);
    self->set_y((float)luaL_checknumber(L, 2));
    return 0;
}

static int node2d_setxy(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d(L, 1);
    self->set_displace((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3));
    return 0;
}

static int node2d_set_rot(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d(L, 1);
    self->set_rotation((float)luaL_checknumber(L, 2));
    return 0;
}

static int node2d_set_scale(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d(L, 1);
    self->set_scale((float)luaL_checknumber(L, 2));
    return 0;
}

static int node2d_getx(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d(L, 1);
    lua_pushnumber(L, self->get_x());
    return 1;
}

static int node2d_gety(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d(L, 1);
    lua_pushnumber(L, self->get_y());
    return 1;
}

static int node2d_getxy(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d(L, 1);
    lua_pushnumber(L, self->get_x());
    lua_pushnumber(L, self->get_y());
    return 2;
}

static int node2d_get_rot(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d(L, 1);
    lua_pushnumber(L, self->get_rotation());
    return 1;
}

static int node2d_get_scale(lua_State *L){
    ss_node2d* self = ss_lua_get_node2d(L, 1);
    lua_pushnumber(L, self->get_scale());
    return 1;
}

int ss_module_node2d(lua_State *L) {
    luaL_reg node2d_funcs[] = {
        { "createRoot", node2d_create_root },
        { "getRoot", node2d_get_root },
        { "newChild", node2d_new_child},
        { "remove", node2d_remove },
        { "detach", node2d_detach },
        { "calc", node2d_calc },
        { "next", node2d_next },
        { "prev", node2d_prev },
        { "firstChild", node2d_first_child },
        { "lastChild", node2d_last_child },
        { "setx", node2d_setx },
        { "sety", node2d_sety },
        { "setxy", node2d_setxy },
        { "setRotation", node2d_set_rot },
        { "setScale", node2d_set_scale },
        { "getx", node2d_getx },
        { "gety", node2d_gety },
        { "getxy", node2d_getxy },
        { "getRotation", node2d_get_rot },
        { "getScale", node2d_get_scale },
        { NULL, NULL }
    };
#if LUA_VERSION_NUM >= 502
    luaL_newlib(L, render2d_funcs);
#else
    luaL_register(L, "node2d", node2d_funcs);
#endif
    return 1;
}

