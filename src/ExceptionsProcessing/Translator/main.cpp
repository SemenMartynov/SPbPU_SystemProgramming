/*  Task 9.
	Convert structural exceptions to the C language exceptions,
	using the translator;
	*/

// IMPORTANT: Don't forget to disable Enhanced Instructions!!!
// Properties -> Configuration Properties -> C/C++ -> Code Generation ->
// Enable Enhanced Instruction Set = No Enhanced Instructions (/arch:IA32)

// IMPORTANT: Don't forget to enable SEH!!!
// Properties -> Configuration Properties -> C/C++ -> Code Generation ->
// Enable C++ Exceptions = Yes with SEH Exceptions(/ EHa)

#include <stdio.h>
#include <tchar.h>
#include <cstring>
#include <cfloat>
#include <stdexcept>
#include <excpt.h>
#include <windows.h>
#include <time.h>

#include "messages.h"

// log
FILE* logfile;
HANDLE eventlog;

void initlog(const _TCHAR* prog);
void closelog();
void writelog(_TCHAR* format, ...);
void syslog(WORD category, WORD identifier, LPWSTR message);

void translator(unsigned int u, EXCEPTION_POINTERS* pExp);

// My exception
class translator_exception {
public:
	translator_exception(const wchar_t* str) {
		wcsncpy_s(buf, sizeof(buf), str, sizeof(buf));
	}
	const wchar_t* what() { return buf; }
private:
	wchar_t buf[255];
};

// Defines the entry point for the console application.
int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);
	eventlog = RegisterEventSource(NULL, L"MyEventProvider");

	// Floating point exceptions are masked by default.
	_clearfp();
	_controlfp_s(NULL, 0, _EM_OVERFLOW | _EM_ZERODIVIDE);

	try {
		writelog(_T("Ready for translator ativation."));
		_set_se_translator(translator);
		writelog(_T("Ready for generate DIVIDE_BY_ZERO exception."));
		syslog(ZERODIVIDE_CATEGORY, READY_FOR_EXCEPTION,
			_T("Ready for generate DIVIDE_BY_ZERO exception."));
		RaiseException(EXCEPTION_FLT_DIVIDE_BY_ZERO,
			EXCEPTION_NONCONTINUABLE, 0, NULL);
		writelog(_T("DIVIDE_BY_ZERO exception is generated."));
	}
	catch (translator_exception &e) {
		_tprintf(_T("CPP exception: %s"), e.what());
		writelog(_T("CPP exception: %s"), e.what());
		syslog(OVERFLOW_CATEGORY, CAUGHT_EXCEPRION, _T("CPP exception"));
	}

	closelog();
	CloseHandle(eventlog);
	exit(0);
}

void translator(unsigned int u, EXCEPTION_POINTERS* pExp) {
	writelog(_T("Translator in action."));
	if (u == EXCEPTION_FLT_DIVIDE_BY_ZERO)
		throw translator_exception(_T("EXCEPTION_FLT_DIVIDE_BY_ZERO"));
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
