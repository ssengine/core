#pragma once

#include <ssengine/ssengine.h>

#include <ssengine/core.h>

#ifdef WIN32
#define EXPORT __declspec(dllexport)
#else
#endif

#include <map>
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

struct ss_core_context{
	std::map<std::string, ss_macro> map_macros;
	std::map<std::string, ss_uri_schema_handler*> uri_schemas;
	lua_State*		L;
	ss_render_device* renderer;
	ss_core_context();
	~ss_core_context();
};

void _ss_uri_init_schemas(ss_core_context* C);
void _ss_init_script_context(ss_core_context* C);
void _ss_release_schemas(ss_core_context* C);
void _ss_destroy_script_context(ss_core_context* C);
