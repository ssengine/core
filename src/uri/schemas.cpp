#include <ssengine/uri.h>
#include <map>
#include <ssengine/log.h>
#include "schemas.h"
#include "../core.h"

//TODO: avoid to use static variable here.

void _ss_release_schemas(ss_core_context* C){
	for (auto itor = C->uri_schemas.begin(); 
			itor != C->uri_schemas.end(); 
			++itor){
		delete itor->second;
	}
	C->uri_schemas.clear();
}

//static ss_uri_schema_manager s_schemas;
//static std::map<std::string, ss_uri_schema_handler*> s_schemas;

void ss_uri_add_schema(ss_core_context* C, const char* schema, ss_uri_schema_handler* handler)
{
	auto itor = C->uri_schemas.find(schema);
	if (itor != C->uri_schemas.end()){
		delete itor->second;
		C->uri_schemas.erase(itor);
	}
	
	C->uri_schemas.insert(std::make_pair(std::string(schema), handler));
	
	//C->uri_schemas[schema] = handler;
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

void ss_uri_add_schema_alias(ss_core_context* C,  const char* schema, const ss_uri& alias, bool readOnly){
	if (!readOnly && !alias.exists() && alias.is_local()){
		alias.mkdir();
	}
	if (!alias.is_directory()){
		SS_LOGE("Cannot create directory for schema alias: %s", schema, alias.str().c_str());
	}
	ss_uri_add_schema(C, schema, new ss_alias_schema_handler(alias, readOnly));
}

void _ss_uri_init_schemas(ss_core_context* C){
	ss_uri_add_schema(C, "file", new ss_file_schema_handler());

#if !defined(NDEBUG)
    const char* internalDir = getenv("SS_DEBUG_INTERNAL_DIR");
    if (internalDir){
        ss_uri_add_schema_alias(C, "internal", ss_uri::from_file(C, internalDir), true);
    }
    else
#endif
    {
        //TODO: Load internal resource from zipped resource.
    }
}

bool ss_uri::is_local() const{
	auto itor = C->uri_schemas.find(schema);
	if (itor == C->uri_schemas.end()){
		return false;
	}
	return itor->second->is_local();
}

bool ss_uri::exists() const{
	auto itor = C->uri_schemas.find(schema);
	if (itor == C->uri_schemas.end()){
		return false;
	}
	return itor->second->exists(*this);
}

bool ss_uri::is_file() const{
	auto itor = C->uri_schemas.find(schema);
	if (itor == C->uri_schemas.end()){
		return false;
	}
	return itor->second->is_file(*this);
}

bool ss_uri::is_directory() const{
	auto itor = C->uri_schemas.find(schema);
	if (itor == C->uri_schemas.end()){
		return false;
	}
	return itor->second->is_directory(*this);
}

bool ss_uri::mkdir() const{
	auto itor = C->uri_schemas.find(schema);
	if (itor == C->uri_schemas.end()){
		return false;
	}
	return itor->second->mkdir(*this);
}

input_stream* ss_uri::open_for_read() const{
	auto itor = C->uri_schemas.find(schema);
	if (itor == C->uri_schemas.end()){
		return nullptr;
	}
	return itor->second->open_for_read(*this);
}

output_stream* ss_uri::open_for_write() const{
	auto itor = C->uri_schemas.find(schema);
	if (itor == C->uri_schemas.end()){
		return nullptr;
	}
	return itor->second->open_for_write(*this);
}

output_stream* ss_uri::open_for_append() const{
	auto itor = C->uri_schemas.find(schema);
	if (itor == C->uri_schemas.end()){
		return nullptr;
	}
	return itor->second->open_for_append(*this);
}


bool ss_uri_exists(ss_core_context* C, const char* uri){
	ss_uri u = ss_uri::parse(C, uri);
	return u.exists();
}

bool ss_uri_is_file(ss_core_context* C, const char* uri){
	ss_uri u = ss_uri::parse(C, uri);
	return u.is_file();
}

bool ss_uri_is_directory(ss_core_context* C, const char* uri){
	ss_uri u = ss_uri::parse(C, uri);
	return u.is_directory();
}

bool ss_uri_mkdir(ss_core_context* C, const char* uri){
	ss_uri u = ss_uri::parse(C, uri);
	return u.mkdir();
}

input_stream* ss_uri_open_for_read(ss_core_context* C, const char* uri){
	ss_uri u = ss_uri::parse(C, uri);
	return u.open_for_read();
}

output_stream* ss_uri_open_for_write(ss_core_context* C, const char* uri){
	ss_uri u = ss_uri::parse(C, uri);
	return u.open_for_write();
}

output_stream* ss_uri_open_for_append(ss_core_context* C, const char* uri){
	ss_uri u = ss_uri::parse(C, uri);
	return u.open_for_append();
}