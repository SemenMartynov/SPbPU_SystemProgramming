/*  Task 1.
	Generate and handle exceptions using the WinAPI functions;
	*/

// IMPORTANT: Don't forget to disable Enhanced Instructions!!!
// Properties -> Configuration Properties -> C/C++ -> Code Generation ->
// Enable Enhanced Instruction Set = No Enhanced Instructions (/arch:IA32)

#include <stdio.h>
#include <tchar.h>
#include <cstring>
#include <cfloat>
#include <cmath>
#include <excpt.h>
#include <windows.h>
#include <time.h>

#include "messages.h"

// log
FILE* logfile;
HANDLE eventlog;

void usage(const _TCHAR* prog);
void initlog(const _TCHAR* prog);
void closelog();
void writelog(_TCHAR* format, ...);
void syslog(WORD category, WORD identifier, LPWSTR message);

// Task switcher
enum {
	DIVIDE_BY_ZERO,
	FLT_OVERFLOW
} task;

// Defines the entry point for the console application.
int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);
	eventlog = RegisterEventSource(NULL, L"MyEventProvider");

	// Check parameters number
	if (argc != 2) {
		_tprintf(_T("Too few parameters.\n\n"));
		writelog(_T("Too few parameters."));
		usage(argv[0]);
		closelog();
		exit(1);
	}

	// Set task
	if (!_tcscmp(_T("-d"), argv[1])) {
		task = DIVIDE_BY_ZERO;
		writelog(_T("Task: DIVIDE_BY_ZERO exception."));
	}
	else if (!_tcscmp(_T("-o"), argv[1])) {
		task = FLT_OVERFLOW;
		writelog(_T("Task: FLT_OVERFLOW exception."));
	}
	else {
		_tprintf(_T("Can't parse parameters.\n\n"));
		writelog(_T("Can't parse parameters."));
		usage(argv[0]);
		closelog();
		exit(1);
	}

	// Floating point exceptions are masked by default.
	_clearfp();
	_controlfp_s(NULL, 0, _EM_OVERFLOW | _EM_ZERODIVIDE);

	// Set exception
	__try {
		volatile float tmp = 0;
		switch (task) {
		case DIVIDE_BY_ZERO:
			writelog(_T("Ready for generate DIVIDE_BY_ZERO exception."));
			syslog(ZERODIVIDE_CATEGORY, READY_FOR_EXCEPTION,
				_T("Ready for generate DIVIDE_BY_ZERO exception."));
			tmp = 1 / tmp;
			writelog(_T("DIVIDE_BY_ZERO exception is generated."));
			break;
		case FLT_OVERFLOW:
			// Note: floating point execution happens asynchronously.
			// So, the exception will not be handled until the next floating
			// point instruction.
			writelog(_T("Ready for generate FLT_OVERFLOW exception."));
			syslog(OVERFLOW_CATEGORY, READY_FOR_EXCEPTION,
				_T("Ready for generate FLT_OVERFLOW exception."));
			tmp = pow(FLT_MAX, 3);
			writelog(_T("Task: FLT_OVERFLOW exception is generated."));
			break;
		default:
			break;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		_tprintf(_T("Well, it looks like we caught something."));
		writelog(_T("Exception is caught."));
		syslog(OVERFLOW_CATEGORY, CAUGHT_EXCEPRION,
			_T("Well, it looks like we caught something."));
	}

	closelog();
	CloseHandle(eventlog);
	exit(0);
}

// Usage manual
void usage(const _TCHAR* prog) {
	_tprintf(_T("Usage: \n"));
	_tprintf(_T("\t%s -d\n"), prog);
	_tprintf(_T("\t\t\t for exception float divide by zero,\n"));
	_tprintf(_T("\t%s -o\n"), prog);
	_tprintf(_T("\t\t\t for exception float overflow.\n"));
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
