#include "core.h"
#include "render/draw_batch.h"
#include <ssengine/resource.h>

//define DllMain for windows
#ifdef WIN32
#include <Windows.h>
BOOL WINAPI DllMain(
	_In_  HINSTANCE hinstDLL,
	_In_  DWORD fdwReason,
	_In_  LPVOID lpvReserved
	){
	return TRUE;
}
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
	: L(nullptr), renderer(0), draw_batch(nullptr)
{
	_ss_uri_init_schemas(this);
	_ss_init_script_context(this);

	//TODO: define core macros

	ss_open_device(this, SS_DT_SOFTWARE);
}

ss_core_context::~ss_core_context(){
	while (resource_with_device_type.size() > 0){
		ss_close_device(this, resource_with_device_type.begin()->first);
	}

	_ss_destroy_script_context(this);
	_ss_release_schemas(this);
}

SS_CORE_API ss_render_device*  ss_get_render_device(ss_core_context* C){
	return C->renderer;
}

SS_CORE_API void ss_set_render_device(ss_core_context* C, ss_render_device* device){
	//TODO: use null device if there's no device.
	if (C->draw_batch){
		delete C->draw_batch;
	}
	C->renderer = device;
	if (device){
		C->draw_batch = new ss_draw_batch(device);
	}
}

