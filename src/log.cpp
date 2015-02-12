#include "core.h"
#include <stdarg.h>
#include <ssengine/log.hpp>
#include <vector>

std::vector<ss_logger*>  loggers;

//TODO: lock loggers list while logging (in same thread);
//TODO: use spin_lock for _ss_log and add_logger/remove_logger.

static void _ss_log(int level, const char* format, size_t sz){
	for (auto itor = loggers.begin(); itor != loggers.end(); ++itor){
		ss_logger* logger = *itor;
		logger->log(level, format, sz, logger->userdata);
	}
}

void ss_log(int level, _SS_LOG_FORMAT_ARGUMENT_ const char* format, ...){
	char buf[4097];
	va_list argp;
	va_start(argp, format);
	
	int nTemp = _vsnprintf(buf, 4096, format, argp);
	if (nTemp >= 0){
		_ss_log(level, buf, nTemp);
	} else {
		buf[4096] = 0;
		_ss_log(level, buf, 4096);
	}

	va_end(argp);
}

static void _ss_wlog(int level, const wchar_t* format, size_t sz){
	for (auto itor = loggers.begin(); itor != loggers.end(); ++itor){
		ss_logger* logger = *itor;
		logger->wlog(level, format, sz, logger->userdata);
	}
}

void ss_wlog(int level, _SS_LOG_FORMAT_ARGUMENT_ const wchar_t* format, ...){
	wchar_t buf[4097];
	va_list argp;
	va_start(argp, format);

	int nTemp = _vsnwprintf(buf, 4096, format, argp);
	if (nTemp >= 0){
		_ss_wlog(level, buf, nTemp);
	}
	else {
		_ss_wlog(level, buf, 4096);
	}

	va_end(argp);
}

void ss_add_logger(ss_logger* logger){
	loggers.push_back(logger);
}

void ss_remove_logger(ss_logger* logger){
	for (auto itor = loggers.begin(); itor != loggers.end(); ++itor){
		if (*itor == logger){
			loggers.erase(itor);
			return;
		}
	}
}
