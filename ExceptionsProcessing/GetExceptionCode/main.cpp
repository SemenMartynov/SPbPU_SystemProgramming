/*  Task 2.
	Get the exceptions code using the GetExceptionCode gunction:
	- Use this function in the filter expression;
	- Use this function in the handler.
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

// log
FILE* logfile;

void usage(const _TCHAR* prog);
void initlog(const _TCHAR* prog);
void closelog();
void writelog(_TCHAR* format, ...);

// Task switcher
enum {
	DIVIDE_BY_ZERO,
	FLT_OVERFLOW
} task;

// Defines the entry point for the console application.
int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);

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
	volatile float tmp = 0;
	switch (task) {
	case DIVIDE_BY_ZERO:
		__try {
			writelog(_T("Ready for generate DIVIDE_BY_ZERO exception."));
			tmp = 1 / tmp;
			writelog(_T("DIVIDE_BY_ZERO exception is generated."));
		}
		// Use GetExceptionCode() function in the filter expression;
		__except ((GetExceptionCode() == EXCEPTION_FLT_DIVIDE_BY_ZERO) ?
		EXCEPTION_EXECUTE_HANDLER :
								  EXCEPTION_CONTINUE_SEARCH)
		{
			_tprintf(_T("Caught exception is: EXCEPTION_FLT_DIVIDE_BY_ZERO"));
			writelog(_T("Caught exception is: EXCEPTION_FLT_DIVIDE_BY_ZERO"));
		}
		break;
	case FLT_OVERFLOW:
		__try {
			// Note: floating point execution happens asynchronously.
			// So, the exception will not be handled until the next
			// floating point instruction.
			writelog(_T("Ready for generate FLT_OVERFLOW exception."));
			tmp = pow(FLT_MAX, 3);
			writelog(_T("Task: FLT_OVERFLOW exception is generated."));
		}
		// Use GetExceptionCode() function in the handler.
		__except (EXCEPTION_EXECUTE_HANDLER) {
			if (GetExceptionCode() == EXCEPTION_FLT_OVERFLOW) {
				writelog(_T("Caught exception is: EXCEPTION_FLT_OVERFLOW"));
				_tprintf(_T("Caught exception is: EXCEPTION_FLT_OVERFLOW"));
			}
			else {
				writelog(_T("UNKNOWN exception: %x\n"), GetExceptionCode());
				_tprintf(_T("UNKNOWN exception: %x\n"), GetExceptionCode());
			}
		}
		break;
	default:
		break;
	}
	closelog();
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
