#include "core.h"

//define DllMain for windows
#ifdef WIN32
#include <Windows.h>
BOOL WINAPI DllMain(
	_In_  HINSTANCE hinstDLL,
	_In_  DWORD fdwReason,
	_In_  LPVOID lpvReserved
	){
	return TRUE;
}
#endif

wchar_t* char2wchar_t(const char* str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	if (len == 0){
		return NULL;
	}
	wchar_t* buf = (wchar_t*)malloc(sizeof(wchar_t)*(len + 1));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, buf, len);

	return buf;
}

char* wchar_t2char(const wchar_t* str)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	if (len == 0){
		return NULL;
	}
	char* buf = (char*)malloc(sizeof(char)*(len + 1));
	WideCharToMultiByte(CP_UTF8, 0, str, -1, buf, len, NULL, NULL);

	return buf;
}
