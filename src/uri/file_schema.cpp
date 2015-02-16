#include <ssengine/uri.h>
#include "schemas.h"

#ifdef WIN32
#include <sys/stat.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

//TODO: support file that more than 2G
//TODO: support chinese file name(use UNICODE API)
//TODO: use uri.to_path() instead of path.c_str()+1

bool ss_file_schema_handler::is_local(){
	return true;
}

bool ss_file_schema_handler::exists(const ss_uri& uri){
	struct stat st;
	return stat(uri.path.c_str() + 1, &st) == 0;
}

bool ss_file_schema_handler::is_file(const ss_uri& uri){
	struct stat st;
	if (stat(uri.path.c_str() + 1, &st) != 0){
		return false;
	}
	return (st.st_mode & S_IFMT) == S_IFREG;
}

bool ss_file_schema_handler::is_directory(const ss_uri& uri)
{
	struct stat st;
	if (stat(uri.path.c_str() + 1, &st) != 0)
	{
		return false;
	}
	return (st.st_mode & S_IFMT) == S_IFDIR;
}

bool ss_file_schema_handler::mkdir(const ss_uri& uri)
{
	return _mkdir(uri.path.c_str() + 1) == 0;
}

struct file_input_stream : input_stream
{
	file_input_stream(FILE* f)
	: _f(f)
	{
	}
	~file_input_stream()
	{
		fclose(_f);
		_f = NULL;
	}

	virtual void close();

	virtual bool seek(long long offset, long origin);
	virtual long long tell();

	virtual size_t read(void* buf, size_t size);

	FILE* _f;
};

void file_input_stream::close()
{
	delete this;
}

bool file_input_stream::seek(long long offset, long origin)
{
	return fseek(_f, (long)offset, origin) == 0;
}

long long file_input_stream::tell()
{
	return ftell(_f);
}

size_t file_input_stream::read(void* buf, size_t sz)
{
	return fread(buf, 1, sz, _f);
}


struct file_output_stream : output_stream
{
	file_output_stream(FILE* f)
	: _f(f)
	{
	}
	~file_output_stream()
	{
		fclose(_f);
		_f = NULL;
	}

	virtual void close();

	virtual bool seek(long long offset, long origin);
	virtual long long tell();

	virtual size_t write(const void* buf, size_t size);

	FILE* _f;
};


void file_output_stream::close()
{
	delete this;
}

bool file_output_stream::seek(long long offset, long origin)
{
	return fseek(_f, (long)offset, origin) == 0;
}

long long file_output_stream::tell()
{
	return ftell(_f);
}

size_t file_output_stream::write(const void* buf, size_t sz)
{
	return fwrite(buf, 1, sz, _f);
}

struct input_stream* ss_file_schema_handler::open_for_read(const ss_uri& uri)
{
	FILE* f = fopen(uri.path.c_str() + 1, "rb");
	if (f){
		return new file_input_stream(f);
	}
	return NULL;
}

struct output_stream* ss_file_schema_handler::open_for_write(const ss_uri& uri)
{
	FILE* f = fopen(uri.path.c_str() + 1, "wb");
	if (f){
		return new file_output_stream(f);
	}
	return NULL;
}

struct output_stream* ss_file_schema_handler::open_for_append(const ss_uri& uri)
{
	FILE* f = fopen(uri.path.c_str() + 1, "ab");
	if (f){
		return new file_output_stream(f);
	}
	return NULL;
}
