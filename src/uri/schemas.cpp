#include <ssengine/uri.h>
#include <map>
#include <ssengine/log.h>
#include "schemas.h"

//TODO: avoid to use static variable here.

//TODO: reimplement this
struct ss_uri_schema_manager{
	ss_uri_schema_manager(){

	}
	~ss_uri_schema_manager(){
		for (auto itor = map.begin(); itor != map.end(); ++itor){
			delete itor->second;
		}
		map.clear();
	}
	std::map<std::string, ss_uri_schema_handler*> map;
};
static ss_uri_schema_manager s_schemas;
//static std::map<std::string, ss_uri_schema_handler*> s_schemas;

void ss_uri_add_schema(const char* schema, ss_uri_schema_handler* handler)
{
	auto itor = s_schemas.map.find(schema);
	if (itor != s_schemas.map.end()){
		delete itor->second;
		s_schemas.map.erase(itor);
	}
	
	s_schemas.map.insert(std::make_pair(std::string(schema), handler));
	
	//s_schemas.map[schema] = handler;
}

struct ss_alias_schema_handler : ss_uri_schema_handler
{
	ss_uri alias;
	bool readOnly;

	ss_alias_schema_handler(ss_uri _alias, bool _readOnly)
		: alias(_alias), readOnly(_readOnly)
	{
		alias.normalize();
	}
	~ss_alias_schema_handler(){

	}

	ss_uri solve(ss_uri uri){
		uri.normalize();
		ss_uri ret = alias.join(uri.path.c_str()+1);
		//ret.normalize();
		return ret;
	}

	// properties
	virtual bool is_local(){
		return alias.is_local();
	}

	// queries
	virtual bool exists(const ss_uri& uri){
		return solve(uri).exists();
	}

	virtual bool is_file(const ss_uri& uri){
		return solve(uri).is_file();
	}

	virtual bool is_directory(const ss_uri& uri){
		return solve(uri).is_directory();
	}

	virtual struct input_stream*  open_for_read(const ss_uri& uri){
		return solve(uri).open_for_read();
	}

	//TODO: remove dir

	// operates
	virtual bool mkdir(const ss_uri& uri){
		if (!readOnly){
			return solve(uri).mkdir();
		}
		return false;
	}
	virtual struct output_stream*  open_for_write(const ss_uri& uri){
		if (!readOnly){
			return solve(uri).open_for_write();
		}
		return NULL;
	}
	virtual struct output_stream*  open_for_append(const ss_uri& uri){
		if (!readOnly){
			return solve(uri).open_for_append();
		}
		return NULL;
	}
};

void ss_uri_add_schema_alias(const char* schema, const ss_uri& alias, bool readOnly){
	if (!readOnly && !alias.exists() && alias.is_local()){
		alias.mkdir();
	}
	if (!alias.is_directory()){
		SS_LOGE("Cannot create directory for schema alias: %s", schema, alias.str().c_str());
	}
	ss_uri_add_schema(schema, new ss_alias_schema_handler(alias, readOnly));
}

void ss_uri_init_schemas(){
	ss_uri_add_schema("file", new ss_file_schema_handler());
}

bool ss_uri::is_local() const{
	auto itor = s_schemas.map.find(schema);
	if (itor == s_schemas.map.end()){
		return false;
	}
	return itor->second->is_local();
}

bool ss_uri::exists() const{
	auto itor = s_schemas.map.find(schema);
	if (itor == s_schemas.map.end()){
		return false;
	}
	return itor->second->exists(*this);
}

bool ss_uri::is_file() const{
	auto itor = s_schemas.map.find(schema);
	if (itor == s_schemas.map.end()){
		return false;
	}
	return itor->second->is_file(*this);
}

bool ss_uri::is_directory() const{
	auto itor = s_schemas.map.find(schema);
	if (itor == s_schemas.map.end()){
		return false;
	}
	return itor->second->is_directory(*this);
}

bool ss_uri::mkdir() const{
	auto itor = s_schemas.map.find(schema);
	if (itor == s_schemas.map.end()){
		return false;
	}
	return itor->second->mkdir(*this);
}

input_stream* ss_uri::open_for_read() const{
	auto itor = s_schemas.map.find(schema);
	if (itor == s_schemas.map.end()){
		return nullptr;
	}
	return itor->second->open_for_read(*this);
}

output_stream* ss_uri::open_for_write() const{
	auto itor = s_schemas.map.find(schema);
	if (itor == s_schemas.map.end()){
		return nullptr;
	}
	return itor->second->open_for_write(*this);
}

output_stream* ss_uri::open_for_append() const{
	auto itor = s_schemas.map.find(schema);
	if (itor == s_schemas.map.end()){
		return nullptr;
	}
	return itor->second->open_for_append(*this);
}


bool ss_uri_exists(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	return u.exists();
}

bool ss_uri_is_file(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	return u.is_file();
}

bool ss_uri_is_directory(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	return u.is_directory();
}

bool ss_uri_mkdir(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	return u.mkdir();
}

input_stream* ss_uri_open_for_read(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	return u.open_for_read();
}

output_stream* ss_uri_open_for_write(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	return u.open_for_write();
}

output_stream* ss_uri_open_for_append(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	return u.open_for_append();
}