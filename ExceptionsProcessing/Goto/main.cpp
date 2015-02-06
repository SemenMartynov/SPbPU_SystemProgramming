/*  Task 7.
	Get out of the __try block by using the goto;
	*/

// IMPORTANT: Don't forget to disable Enhanced Instructions!!!
// Properties -> Configuration Properties -> C/C++ -> Code Generation ->
// Enable Enhanced Instruction Set = No Enhanced Instructions (/arch:IA32)

#include <stdio.h>
#include <tchar.h>
#include <cstring>
#include <cfloat>
#include <excpt.h>
#include <windows.h>
#include <time.h>

// log
FILE* logfile;

void initlog(const _TCHAR* prog);
void closelog();
void writelog(_TCHAR* format, ...);

// Defines the entry point for the console application.
int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);

	// Floating point exceptions are masked by default.
	_clearfp();
	_controlfp_s(NULL, 0, _EM_OVERFLOW | _EM_ZERODIVIDE);

	__try {
		writelog(_T("Call goto"));
		goto OUT_POINT;
		writelog(_T("Ready for generate DIVIDE_BY_ZERO exception."));
		RaiseException(EXCEPTION_FLT_DIVIDE_BY_ZERO,
			EXCEPTION_NONCONTINUABLE, 0, NULL);
		writelog(_T("DIVIDE_BY_ZERO exception is generated."));
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		writelog(_T("Handler in action."));
		_tprintf(_T("Handler in action."));
	}
OUT_POINT:
	writelog(_T("A point outside the __try block."));
	_tprintf(_T("A point outside the __try block."));

	closelog();
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
		_tprintf(_T("Can't open log file %s\n"), logname);
		_wperror(_T("The following error occurred"));
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
