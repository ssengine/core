#include "core.h"
#include "render/draw_batch.h"
#include <ssengine/resource.h>
#include <ssengine/render/drawbatch.h>
#include <ssengine/macros.h>
#include <ssengine/log.h>
#include <algorithm>

//define DllMain for windows
#ifdef WIN32
#include <Windows.h>
#endif

wchar_t* char2wchar_t(const char* str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	if (len == 0){
		return NULL;
	}
	wchar_t* buf = (wchar_t*)malloc(sizeof(wchar_t)*(len + 1));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, buf, len);

	return buf;
}

char* wchar_t2char(const wchar_t* str)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	if (len == 0){
		return NULL;
	}
	char* buf = (char*)malloc(sizeof(char)*(len + 1));
	WideCharToMultiByte(CP_UTF8, 0, str, -1, buf, len, NULL, NULL);

	return buf;
}

ss_core_context* ss_create_context(){
	ss_core_context* C = new ss_core_context();
	return C;
}

void ss_destroy_context(ss_core_context* C){
	delete C;
}

ss_core_context::ss_core_context()
    : L(nullptr), renderer(0), draw_batch(nullptr), image_decoder(nullptr)
{
	_ss_uri_init_schemas(this);
	_ss_init_script_context(this);

    _ss_init_image_decoder(this);

	//TODO: define core macros

	ss_open_device(this, SS_DT_SOFTWARE);
}

ss_core_context::~ss_core_context(){
	while (resource_with_device_type.size() > 0){
		ss_close_device(this, resource_with_device_type.begin()->first);
	}

	_ss_destroy_script_context(this);
	_ss_release_schemas(this);

    _ss_dispose_image_decoder(this);
}

ss_render_device*  ss_get_render_device(ss_core_context* C){
	return C->renderer;
}

void ss_set_render_device(ss_core_context* C, ss_render_device* device){
	//TODO: use null device if there's no device.
	if (C->draw_batch){
		delete C->draw_batch;
	}
	C->renderer = device;
	if (device){
        C->draw_batch = new ss_draw_batch_impl(device);
	}
}

ss_draw_batch* ss_core_get_draw_batch(ss_core_context* C){
    return C->draw_batch;
}

#ifdef WIN32
int ss_load_plugin(ss_core_context*C, const char* name){
    //TODO: when ss_load_plugins is called multi times with same name, only load the first time.

    std::string macroName = name;
    macroName.insert(0, "PLUGINS(");
    macroName.append(")");

    std::string macroType = (macroName + "(type)");
    ss_macro_eval(C, macroType.c_str());
    std::string type = ss_macro_get_content(C, macroType.c_str());

    std::string macroPath = (macroName + "(path)");
    ss_macro_eval(C, macroPath.c_str());
    std::string path = ss_macro_get_content(C, macroPath.c_str());

    if (path.empty()){
        path = std::string(name) + ".dll";
    }
    std::wstring wpath = string2wstring(path);

    //TODO: UnloadLibrary mod when ss_context_destroy(C);
    if (type == "native"){
        HMODULE mod = LoadLibraryW(wpath.c_str());
        if (mod == NULL){
            SS_LOGE("Failed to load plugin `%s`", name);
            return -1;
        }

        ss_main_func_type func = (ss_main_func_type)GetProcAddress(mod, "ss_main");
        if (func == NULL){
            lua_CFunction luaentry = (lua_CFunction)GetProcAddress(mod, (std::string("luaopen_") + name).c_str());
            if (luaentry == NULL){
                SS_LOGE("Cannot locate ss_main or luaopen_%s on `%s`", name, path.c_str());
                return -2;
            }
            std::string sname = name;
            std::replace(sname.begin(), sname.end(), '_', '.');
            ss_lua_preload_module(C->L, sname.c_str(), luaentry);
        }
        else {
            func(C);
        }
    }
    else {
        SS_LOGE("Unsupported plugin type `%s` for `%s`", type.c_str(), name);
        return -3;
    }
    return 0;
}
#else
int ss_load_plugin(ss_core_context*C, const char* name){
    // Not implemented yet.
    return -1;
}
#endif