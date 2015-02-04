/*  Task 11.
	Check the correctness of the exit from the __try block using
	the AbnormalTermination function in the final handler finally.
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

void usage(const _TCHAR *prog);
void initlog(const _TCHAR* prog);
void closelog();
void writelog(_TCHAR* format, ...);

// Defines the entry point for the console application.
int _tmain(int argc, _TCHAR* argv[]) {
	// Floating point exceptions are masked by default.
	_clearfp();
	_controlfp_s(NULL, 0, _EM_OVERFLOW | _EM_ZERODIVIDE);

	__try {
		goto OUT_POINT;
		RaiseException(EXCEPTION_FLT_DIVIDE_BY_ZERO, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}
	__finally
	{
		if (AbnormalTermination())
			printf("%s", "Normal termination  in goto case\n"); 
		else
			printf("%s", "Abnormal termination in goto case\n");
	}
OUT_POINT:
	__try {
		__leave;
		RaiseException(EXCEPTION_FLT_DIVIDE_BY_ZERO, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}
	__finally
	{
		if (AbnormalTermination())
			printf("%s", "Normal termination  in __leave case");
		else
			printf("%s", "Abnormal termination in __leave case");
			
	}
	
	exit(0);
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
