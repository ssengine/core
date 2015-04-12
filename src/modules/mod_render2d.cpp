#include <lua.hpp>
#include "../modules.h"
#include "ssengine/render/drawbatch.h"
#include "ssengine/core.h"
#include "ssengine/render/device.h"
#include "../core.h"
#include "ssengine/render/resources.h"

static int render2d_flush(lua_State *L) {
	ss_core_context *C = ss_lua_get_core_context(L);
	ss_db_flush(C);
	return 0;
}

static int render2d_context_meta_gc(lua_State *L){
    render2d_context* ptr = ss_lua_check_render2d_context(L, 1);
    ptr->~render2d_context();
    return 0;
}


static int render2d_context_draw_image(lua_State *L){
    render2d_context* RC = ss_lua_check_render2d_context(L, 1);

    auto& ref = ss_lua_check_resource_reference<ss_texture2d>(L, 2);
    ss_texture2d* res = ref->get();

    // Ignore texture that was not loaded.
    if (res != NULL){
        ss_db_draw_image_rect(RC->C, RC->matrix_stack.top(), res,
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
            (float)luaL_checknumber(L, 5),
            (float)luaL_checknumber(L, 6),
            (float)luaL_checknumber(L, 7),
            (float)luaL_checknumber(L, 8),
            (float)luaL_checknumber(L, 9),
            (float)luaL_checknumber(L, 10)
            );
    }
    return 0;
}

static int render2d_context_draw_line(lua_State *L) {
    render2d_context* RC = ss_lua_check_render2d_context(L, 1);
    float x0 = (float)luaL_checknumber(L, 2);
    float y0 = (float)luaL_checknumber(L, 3);
    float x1 = (float)luaL_checknumber(L, 4);
    float y1 = (float)luaL_checknumber(L, 5);

    ss_db_draw_line(RC->C, RC->matrix_stack.top(), x0, y0, x1, y1);

    return 0;
}

static int render2d_context_matrix_push(lua_State *L){
    render2d_context* RC = ss_lua_check_render2d_context(L, 1);
    RC->matrix_stack.push();
    return 0;
}

static int render2d_context_matrix_pop(lua_State *L){
    render2d_context* RC = ss_lua_check_render2d_context(L, 1);
    RC->matrix_stack.pop();
    return 0;
}

static int render2d_context_load_ortho2d(lua_State *L){
    render2d_context* RC = ss_lua_check_render2d_context(L, 1);
    auto& top = RC->matrix_stack.top();

    float width = (float)luaL_checknumber(L, 2);
    float height = (float)luaL_checknumber(L, 3);
    float x = (float)luaL_checknumber(L, 4);
    float y = (float)luaL_checknumber(L, 5);

    top = ss_matrix::ortho2d(width, height, x, y);

    return 0;
}

int render2d_new_context(lua_State *L){
    render2d_context* ptr = reinterpret_cast<render2d_context*>(lua_newuserdata(L, sizeof(render2d_context)));
    new (ptr)render2d_context();

    ptr->C = ss_lua_get_core_context(L);

    if (luaL_newmetatable(L, "render2d.Context") == 1){
        luaL_reg mt_methods[] = {
            { "__gc", render2d_context_meta_gc },

            // Draw batch methods:
            { "flush", render2d_flush },
            { "drawImage", render2d_context_draw_image },
            { "drawLine", render2d_context_draw_line },

            // Matrix operators:
            { "matrixPush", render2d_context_matrix_push },
            { "matrixPop", render2d_context_matrix_pop },
            { "loadOrtho2D", render2d_context_load_ortho2d },
            { NULL, NULL }
        };

        for (size_t i = 0; mt_methods[i].func != nullptr; ++i){
            lua_pushcfunction(L, mt_methods[i].func);
            lua_setfield(L, -2, mt_methods[i].name);
        }
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);
    return 1;
}

int ss_module_render2d(lua_State *L) {
    luaL_reg render2d_funcs[] = {
        { "flush", render2d_flush },
        { "newContext", render2d_new_context },
        { NULL, NULL }
	};

#if LUA_VERSION_NUM >= 502
	luaL_newlib(L, render2d_funcs);
#else
	luaL_register(L, "render2d", render2d_funcs);
#endif
	return 1;
}
