#include <ssengine/macros.h>
#include <vector>
#include <map>
#include <string>
#include <stack>

#include "core.h"

//TODO: make me thread-safe.

//TODO: do cache with dirty check.

void ss_macro::eval(ss_core_context* C){
	this->evaluting = true;

	std::stack<std::string> s;
	s.push("");

	for (size_t i = 0; i < expr.length(); i++){
		char ch = expr[i];
		switch (ch){
		case '{':{
			s.push(std::string());
		}
			break;
		case '}':{
			if (s.size() > 1){
				std::string top = s.top();
				s.pop();
				auto itor = C->map_macros.find(top);
				if (itor == C->map_macros.end() || itor->second.evaluting){
					s.top().push_back('{');
					s.top().append(top);
					s.top().push_back('}');
				} else {
					itor->second.eval(C);
					s.top().append(itor->second.cache);
				}
				break;
			}
		}
		default:
			s.top().push_back(ch);
		}
	}

	while (s.size() > 1){
		std::string top = s.top();
		s.pop();
		s.top().push_back('{');
		s.top().append(top);
	}

	this->cache = s.top();

	this->evaluting = false;
}

void ss_macro_define(ss_core_context* C, const char* name, const char* val){
	C->map_macros[name] = ss_macro(val);
}

void ss_macro_undefine(ss_core_context* C, const char* name){
	C->map_macros.erase(name);
}

int ss_macro_isdef(ss_core_context* C, const char* name){
	return C->map_macros.find(name) != C->map_macros.end();
}

void ss_macro_eval(ss_core_context* C, const char* name){
	auto itor = C->map_macros.find(name);
	if (itor != C->map_macros.end()){
		itor->second.eval(C);
	}
}

size_t ss_macro_get_length(ss_core_context* C, const char* name){
	auto itor = C->map_macros.find(name);
	if (itor == C->map_macros.end()){
		return 0;
	}
	return itor->second.cache.length();
}

int ss_macro_get_content(ss_core_context* C, const char* name, char* buf, size_t sz){
	auto itor = C->map_macros.find(name);
	if (itor == C->map_macros.end()){
		if (buf){
			*buf = 0;
		}
		return 0;
	}
	const std::string& cache = itor->second.cache;
	if (buf && sz < cache.length() + 1){
		return -1;
	}
	
	if (buf){
		cache.copy(buf, cache.length());
	}
	return cache.length();
}

const std::string & ss_macro_get_content(ss_core_context* C, const char* name){
	auto itor = C->map_macros.find(name);
	if (itor == C->map_macros.end()){
		static std::string empty;
		return empty;
	}
	return itor->second.cache;
}

int SS_CORE_API ss_macro_get_integer(ss_core_context* C, const char* name){
	auto itor = C->map_macros.find(name);
	if (itor == C->map_macros.end()){
		static std::string empty;
		return 0;
	}
	int val = 0;
	sscanf(itor->second.cache.c_str(), "%d", &val);
	return val;
}
