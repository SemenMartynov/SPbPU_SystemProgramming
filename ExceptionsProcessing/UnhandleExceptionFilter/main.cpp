/*  Task 5.
	Use the UnhandledExceptionFilter and SetUnhandledexceptionfilter
	for unhandled exceptions;.
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

void usage(const _TCHAR *prog);
LONG WINAPI UnhandledExceptionFilter(const _EXCEPTION_POINTERS *ExceptionInfo);

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
		return 1;
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
		return 2;
	}

	// Floating point exceptions are masked by default.
	_clearfp();
	_controlfp_s(NULL, 0, _EM_OVERFLOW | _EM_ZERODIVIDE);

	volatile float tmp = 0;
	SetUnhandledExceptionFilter(UnhandledExceptionFilter);

	switch (task) {
	case DIVIDE_BY_ZERO:
		// throw an exception using the RaiseException() function
		RaiseException(EXCEPTION_FLT_DIVIDE_BY_ZERO, EXCEPTION_EXECUTE_FAULT, 0, NULL);
		break;
	case FLT_OVERFLOW:
		// throw an exception using the RaiseException() function
		RaiseException(EXCEPTION_FLT_OVERFLOW, EXCEPTION_EXECUTE_FAULT, 0, NULL);
		break;
	default:
		break;
	}
	
	return 0;
}

LONG WINAPI UnhandledExceptionFilter(const _EXCEPTION_POINTERS *ExceptionPointers) {
	enum { size = 200 };
	char buf[size] = { '\0' };
	const char* err = "Unhandled exception!\nexeption code : 0x";
	// Get information about the exception using the GetExceptionInformation
	sprintf_s(buf, "%s%x%s%x%s%x%s", err, ExceptionPointers->ExceptionRecord->ExceptionCode,
	", data adress: 0x", ExceptionPointers->ExceptionRecord->ExceptionInformation[1],
	", instruction adress: 0x", ExceptionPointers->ExceptionRecord->ExceptionAddress);
	printf("%s", buf);
	
	return EXCEPTION_CONTINUE_SEARCH;
}

// Usage manual
void usage(const _TCHAR *prog) {
	printf("Usage: \n");
	_tprintf(_T("\t%s -d\n"), prog);
	printf("\t\t\t for exception float divide by zero,\n");
	_tprintf(_T("\t%s -o\n"), prog);
	printf("\t\t\t for exception float overflow.\n");
}
