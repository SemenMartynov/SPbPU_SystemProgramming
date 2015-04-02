#include "Logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <stdarg.h>
#include <time.h>
#include <Windows.h>

Logger::Logger(const _TCHAR* prog, int tid /* = -1*/) {
	_TCHAR logname[255];

	if (tid > 0)
		swprintf_s(logname, _T("%s.%d.log"), prog, tid);
	else
		swprintf_s(logname, _T("%s.log"), prog);

	// Try to open log file for append
	if (_wfopen_s(&logfile, logname, _T("a+"))) {
		_wperror(_T("The following error occurred"));
		_tprintf(_T("Can't open log file %s\n"), logname);
		exit(-1);
	}
	quietlog(_T("%s is starting."), prog);
}

Logger::~Logger() {
	quietlog(_T("Shutting down.\n"));
	fclose(logfile);
}

void Logger::quietlog(_TCHAR* format, ...) {
	_TCHAR buf[255];
	va_list ap;
	struct tm newtime;
	__time64_t long_time;
	// Get time as 64-bit integer.
	_time64(&long_time);
	// Convert to local time.
	_localtime64_s(&newtime, &long_time);
	// Convert to normal representation.
	swprintf_s(buf, _T("[%d/%d/%d %d:%d:%d] "), newtime.tm_mday,
		newtime.tm_mon + 1, newtime.tm_year + 1900, newtime.tm_hour,
		newtime.tm_min, newtime.tm_sec);
	// Write date and time
	fwprintf(logfile, _T("%s"), buf);
	// Write all params
	va_start(ap, format);
	_vsnwprintf_s(buf, sizeof(buf) - 1, format, ap);
	fwprintf(logfile, _T("%s"), buf);
	va_end(ap);
	// New sting
	fwprintf(logfile, _T("\n"));
}

void Logger::loudlog(_TCHAR* format, ...) {
	_TCHAR buf[255];
	va_list ap;
	struct tm newtime;
	__time64_t long_time;
	// Get time as 64-bit integer.
	_time64(&long_time);
	// Convert to local time.
	_localtime64_s(&newtime, &long_time);
	// Convert to normal representation.
	swprintf_s(buf, _T("[%d/%d/%d %d:%d:%d] "), newtime.tm_mday,
		newtime.tm_mon + 1, newtime.tm_year + 1900, newtime.tm_hour,
		newtime.tm_min, newtime.tm_sec);
	// Write date and time
	fwprintf(logfile, _T("%s"), buf);
	// Write all params
	va_start(ap, format);
	_vsnwprintf_s(buf, sizeof(buf) - 1, format, ap);
	fwprintf(logfile, _T("%s"), buf);
	_tprintf(_T("%s"), buf);
	va_end(ap);
	// New sting
	fwprintf(logfile, _T("\n"));
	_tprintf(_T("\n"));
}
