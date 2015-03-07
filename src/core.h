#pragma once

#include <ssengine/ssengine.h>

#include <ssengine/core.h>

#ifdef WIN32
#define EXPORT __declspec(dllexport)
#else
#endif

#include <map>
#include <set>
#include <string>

struct ss_macro{
	std::string expr;
	/*
	bool dirty;
	std::vector<std::string> depends;
	*/
	bool evaluting = false;
	std::string cache;

	ss_macro(){

	}
	ss_macro(std::string _expr)
		:expr(_expr)
	{
	}

	void eval(ss_core_context* C);
};

struct ss_uri_schema_handler;

struct ss_draw_batch;

enum ss_resource_status_mask
{
	SS_RTM_LOADING   = 0x1,
	SS_RTM_CANCELED  = 0x2
};

struct ss_resource_prototype;

struct ss_resource_ref;

struct ss_resource_ref_impl{
	// All these flags should be used in main thread.
	// So no atomic ops required.

	const ss_resource_prototype* const	prototype;
	const int							device_type;
	void*								ptr;

	int									ref_count;
	std::string							uri;
	int									status;

	int									error_code;

	ss_resource_ref_impl(
		const ss_resource_prototype* _pt,
		int							 _dt,
		std::string					 _uri
		) :prototype(_pt), device_type(_dt), uri(_uri),
		ptr(nullptr), ref_count(1),
		error_code(0)
	{

	}

	ss_resource_ref* ref(){
		return reinterpret_cast<ss_resource_ref*>(this);
	}
};

inline ss_resource_ref_impl* wrap(ss_resource_ref* p){
	return reinterpret_cast<ss_resource_ref_impl*>(p);
}

struct ss_core_context{
	std::map<std::string, ss_macro> map_macros;
	std::map<std::string, ss_uri_schema_handler*>	uri_schemas;
	std::map<std::string, ss_resource_ref_impl*>		resource_from_uris;

	// Only save loaded resource with device_type
	std::map<int, std::set<ss_resource_ref_impl*> >		resource_with_device_type;

	std::map<std::string, int>	user_defined_device_type;

	lua_State*		L;
	ss_render_device* renderer;

	ss_draw_batch*	draw_batch;
	ss_core_context();
	~ss_core_context();
};

void _ss_uri_init_schemas(ss_core_context* C);
void _ss_init_script_context(ss_core_context* C);
void _ss_release_schemas(ss_core_context* C);
void _ss_destroy_script_context(ss_core_context* C);
