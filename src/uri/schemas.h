#pragma once

#include <ssengine/uri.h>

struct ss_file_schema_handler
	: ss_uri_schema_handler
{
	// properties
	virtual bool is_local();

	// queries
	virtual bool exists(const ss_uri& uri);
	virtual bool is_file(const ss_uri& uri);
	virtual bool is_directory(const ss_uri& uri);
	virtual struct input_stream*  open_for_read(const ss_uri& uri);

	//TODO: remove dir

	// operates
	virtual bool mkdir(const ss_uri& uri);
	virtual struct output_stream*  open_for_write(const ss_uri& uri);
	virtual struct output_stream*  open_for_append(const ss_uri& uri);
};