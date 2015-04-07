/*  Task 10.
	Use the final handler finally;
	*/

// IMPORTANT: Don't forget to disable Enhanced Instructions!!!
// Properties -> Configuration Properties -> C/C++ -> Code Generation ->
// Enable Enhanced Instruction Set = No Enhanced Instructions (/arch:IA32)

#include <stdio.h>
#include <tchar.h>
#include <cfloat>
#include <excpt.h>
#include <time.h>
#include <windows.h>

#include "messages.h"

// log
FILE* logfile;
HANDLE eventlog;

void initlog(const _TCHAR* prog);
void closelog();
void writelog(_TCHAR* format, ...);
void syslog(WORD category, WORD identifier, LPWSTR message);

// Defines the entry point for the console application.
int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);
	eventlog = RegisterEventSource(NULL, L"MyEventProvider");

	// Floating point exceptions are masked by default.
	_clearfp();
	_controlfp_s(NULL, 0, _EM_OVERFLOW | _EM_ZERODIVIDE);

	__try {
		// No exception
		writelog(_T("Try block."));
	}
	__finally
	{
		writelog(_T("Thre is no exception, but the handler is called."));
		_tprintf(_T("Thre is no exception, but the handler is called."));
		syslog(OVERFLOW_CATEGORY, CAUGHT_EXCEPRION,
			_T("Thre is no exception, but the handler is called."));
	}

	closelog();
	CloseHandle(eventlog);
	exit(0);
}

void initlog(const _TCHAR* prog) {
	_TCHAR logname[255];
	wcscpy_s(logname, prog);

	// replace extension
	_TCHAR* extension;
	extension = wcsstr(logname, _T(".exe"));
	wcsncpy_s(extension, 5, _T(".log"), 4);

	// Try to open log file for append
	if (_wfopen_s(&logfile, logname, _T("a+"))) {
		_wperror(_T("The following error occurred"));
		_tprintf(_T("Can't open log file %s\n"), logname);
		exit(1);
	}

	writelog(_T("%s is starting."), prog);
}

void closelog() {
	writelog(_T("Shutting down.\n"));
	fclose(logfile);
}

void writelog(_TCHAR* format, ...) {
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

void syslog(WORD category, WORD identifier, LPWSTR message) {
	LPWSTR pMessages[1] = { message };

	if (!ReportEvent(
		eventlog,					// event log handle 
		EVENTLOG_INFORMATION_TYPE,	// event type
		category,					// event category 
		identifier,					// event identifier
		NULL,						// user security identifier
		1,							// number of substitution strings
		0,							// data size
		(LPCWSTR*)pMessages,		// pointer to strings
		NULL)) {					// pointer to binary data buffer
		writelog(_T("ReportEvent failed with 0x%x"), GetLastError());
	}
}
