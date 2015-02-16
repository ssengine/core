#include <ssengine/uri.h>
#include <map>

//TODO: avoid to use static variable here.
static std::map<std::string, ss_uri_schema_handler> s_schemas;

void ss_uri_add_schema(const char* schema, ss_uri_schema_handler handler)
{
	s_schemas[schema] = handler;
}

extern struct ss_uri_schema_handler file_schema_handler;

void ss_uri_init_schemas(){
	ss_uri_add_schema("file", file_schema_handler);
}

bool ss_uri_exists(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	auto itor = s_schemas.find(u.schema);
	if (itor == s_schemas.end()){
		return false;
	}
	return itor->second.exists(u);
}

bool ss_uri_is_file(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	auto itor = s_schemas.find(u.schema);
	if (itor == s_schemas.end()){
		return false;
	}
	return itor->second.is_file(u);
}

bool ss_uri_is_directory(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	auto itor = s_schemas.find(u.schema);
	if (itor == s_schemas.end()){
		return false;
	}
	return itor->second.is_directory(u);
}

bool ss_uri_mkdir(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	auto itor = s_schemas.find(u.schema);
	if (itor == s_schemas.end()){
		return false;
	}
	return itor->second.mkdir(u);
}

input_stream* ss_uri_open_for_read(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	auto itor = s_schemas.find(u.schema);
	if (itor == s_schemas.end()){
		return nullptr;
	}
	return itor->second.open_for_read(u);
}

output_stream* ss_uri_open_for_write(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	auto itor = s_schemas.find(u.schema);
	if (itor == s_schemas.end()){
		return nullptr;
	}
	return itor->second.open_for_write(u);
}

output_stream* ss_uri_open_for_append(const char* uri){
	ss_uri u = ss_uri::parse(uri);
	auto itor = s_schemas.find(u.schema);
	if (itor == s_schemas.end()){
		return nullptr;
	}
	return itor->second.open_for_append(u);
}