#include <ssengine/macros.h>
#include <vector>
#include <map>
#include <string>
#include <stack>

//TODO: make me thread-safe.

//TODO: do cache with dirty check.

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

	void eval();
};

//TODO: avoid to use static variable here.
static std::map<std::string, ss_macro> s_map_macros;

void ss_macro::eval(){
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
				auto itor = s_map_macros.find(top);
				if (itor == s_map_macros.end() || itor->second.evaluting){
					s.top().push_back('{');
					s.top().append(top);
					s.top().push_back('}');
				} else {
					itor->second.eval();
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

void ss_macro_define(const char* name, const char* val){
	s_map_macros[name] = ss_macro(val);
}

void ss_macro_undefine(const char* name){
	s_map_macros.erase(name);
}

int ss_macro_isdef(const char* name){
	return s_map_macros.find(name) != s_map_macros.end();
}

void ss_macro_eval(const char* name){
	auto itor = s_map_macros.find(name);
	if (itor != s_map_macros.end()){
		itor->second.eval();
	}
}

size_t ss_macro_get_length(const char* name){
	auto itor = s_map_macros.find(name);
	if (itor == s_map_macros.end()){
		return 0;
	}
	return itor->second.cache.length();
}

int ss_macro_get_content(const char* name, char* buf, size_t sz){
	auto itor = s_map_macros.find(name);
	if (itor == s_map_macros.end()){
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

const std::string & ss_macro_get_content(const char* name){
	auto itor = s_map_macros.find(name);
	if (itor == s_map_macros.end()){
		static std::string empty;
		return empty;
	}
	return itor->second.cache;
}

int SS_CORE_API ss_macro_get_integer(const char* name){
	auto itor = s_map_macros.find(name);
	if (itor == s_map_macros.end()){
		static std::string empty;
		return 0;
	}
	int val = 0;
	sscanf(itor->second.cache.c_str(), "%d", &val);
	return val;
}
