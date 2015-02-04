/*  Task 4.
	Get information about the exception using the GetExceptionInformation() fnc;
	throw an exception using the RaiseException() function.
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

void usage(const _TCHAR *prog);
void initlog(const _TCHAR* prog);
void closelog();
void writelog(_TCHAR* format, ...);
LONG Filter(DWORD dwExceptionGode, const _EXCEPTION_POINTERS *ep);

// Task switcher
enum {
	DIVIDE_BY_ZERO,
	FLT_OVERFLOW
} task;

// Defines the entry point for the console application.
int _tmain(int argc, _TCHAR* argv[]) {
	// Check parameters number
	if (argc != 2) {
		printf("Too few parameters.\n\n");
		usage(argv[0]);
		exit(1);
	}

	// Set task
	if (!_tcscmp(_T("-d"), argv[1])) {
		task = DIVIDE_BY_ZERO;
	}
	else if (!_tcscmp(_T("-o"), argv[1])) {
		task = FLT_OVERFLOW;
	}
	else {
		printf("Can't parse parameters.\n\n");
		usage(argv[0]);
		exit(1);
	}

	// Floating point exceptions are masked by default.
	_clearfp();
	_controlfp_s(NULL, 0, _EM_OVERFLOW | _EM_ZERODIVIDE);

	__try {
		switch (task) {
		case DIVIDE_BY_ZERO:
			// throw an exception using the RaiseException() function
			RaiseException(EXCEPTION_FLT_DIVIDE_BY_ZERO, 
										EXCEPTION_NONCONTINUABLE, 0, NULL);
			break;
		case FLT_OVERFLOW:
			// throw an exception using the RaiseException() function
			RaiseException(EXCEPTION_FLT_OVERFLOW,
										EXCEPTION_NONCONTINUABLE, 0, NULL);
			break;
		default:
			break;
		}
	}
	__except (Filter(GetExceptionCode(), GetExceptionInformation())) {
		// There is nothing to do, everything is done in the filter function.
	}
	exit(0);
}

LONG Filter(DWORD dwExceptionGode, const _EXCEPTION_POINTERS *ExceptionPointers) {
	enum { size = 200 };
	char buf[size] = { '\0' };
	const char* err = "Fatal error!\nexeption code: 0x";
	const char* mes = "\nProgram terminate!";
	if (ExceptionPointers)
		// Get information about the exception using the GetExceptionInformation
		sprintf_s(buf, "%s%x%s%x%s%x%s", err, 
						ExceptionPointers->ExceptionRecord->ExceptionCode,
						", data adress: 0x", 
						ExceptionPointers->ExceptionRecord->ExceptionInformation[1],
						", instruction adress: 0x",
						ExceptionPointers->ExceptionRecord->ExceptionAddress, mes);
	else
		sprintf_s(buf, "%s%x%s", err, dwExceptionGode, mes);
	printf("%s", buf);

	return EXCEPTION_EXECUTE_HANDLER;
}

// Usage manual
void usage(const _TCHAR *prog) {
	printf("Usage: \n");
	_tprintf(_T("\t%s -d\n"), prog);
	printf("\t\t\t for exception float divide by zero,\n");
	_tprintf(_T("\t%s -o\n"), prog);
	printf("\t\t\t for exception float overflow.\n");
}

void initlog(const _TCHAR* prog) {
	_TCHAR logname[30];
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
	swprintf_s(buf, _T("[%d/%d/%d %d:%d:%d] "), newtime.tm_mday, newtime.tm_mon + 1, newtime.tm_year + 1900, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);

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
