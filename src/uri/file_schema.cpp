#include <ssengine/uri.h>
#include "schemas.h"

#ifdef WIN32
#include <sys/stat.h>
#include <direct.h>
#include <sys/types.h>
#include <wchar.h>
#else
#include <unistd.h>
#endif

//TODO: support file that more than 2G
//TODO: support chinese file name(use UNICODE API)
//TODO: use uri.to_path() instead of path.c_str()+1
//TODO: support path long than MAX_PATH

#if WIN32
#include <Shlwapi.h>
#include <Wininet.h>

#pragma comment(lib, "Shlwapi.lib")
static std::wstring ss_file_uri_to_path(const ss_uri& uri){
    wchar_t* wUri = char2wchar_t(uri.str().c_str());

    wchar_t wOutPath[MAX_PATH + 1];
    DWORD wOutSize = MAX_PATH;
    if (S_OK != PathCreateFromUrlW(wUri, wOutPath, &wOutSize, NULL)){
        wOutPath[0] = 0;
    }

    delete[] wUri;
    return wOutPath;
}
#else
static std::wstring ss_file_uri_to_path(const ss_uri& path){
    ss_uri ret;

    bool isAbsolute = (path[0] == '/');

    return ret;
}
#endif


bool ss_file_schema_handler::is_local(){
	return true;
}

bool ss_file_schema_handler::exists(const ss_uri& uri){
    struct _stat st;
    return _wstat(ss_file_uri_to_path(uri).c_str(), &st) == 0;
}

bool ss_file_schema_handler::is_file(const ss_uri& uri){
    struct _stat st;
    if (_wstat(ss_file_uri_to_path(uri).c_str(), &st) != 0)
    {
		return false;
	}
	return (st.st_mode & S_IFMT) == S_IFREG;
}

bool ss_file_schema_handler::is_directory(const ss_uri& uri)
{
    struct _stat st;
    if (_wstat(ss_file_uri_to_path(uri).c_str(), &st) != 0)
	{
		return false;
	}
	return (st.st_mode & S_IFMT) == S_IFDIR;
}

bool ss_file_schema_handler::mkdir(const ss_uri& uri)
{
    return _wmkdir(ss_file_uri_to_path(uri).c_str()) == 0;
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
    FILE* f = _wfopen(ss_file_uri_to_path(uri).c_str(), L"rb");
	if (f){
		return new file_input_stream(f);
	}
	return NULL;
}

struct output_stream* ss_file_schema_handler::open_for_write(const ss_uri& uri)
{
    FILE* f = _wfopen(ss_file_uri_to_path(uri).c_str(), L"wb");
	if (f){
		return new file_output_stream(f);
	}
	return NULL;
}

struct output_stream* ss_file_schema_handler::open_for_append(const ss_uri& uri)
{
    FILE* f = _wfopen(ss_file_uri_to_path(uri).c_str(), L"ab");
	if (f){
		return new file_output_stream(f);
	}
	return NULL;
}
