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

void usage(const _TCHAR *prog);

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

	// Set exception
	volatile float tmp = 0;
	switch (task) {
	case DIVIDE_BY_ZERO:
		__try {
			tmp = 1 / tmp;
		}
		// Use GetExceptionCode() function in the filter expression;
		__except ((GetExceptionCode() == EXCEPTION_FLT_DIVIDE_BY_ZERO) ?
		EXCEPTION_EXECUTE_HANDLER :
								  EXCEPTION_CONTINUE_SEARCH)
		{
			printf("Caught exception is: EXCEPTION_FLT_DIVIDE_BY_ZERO");
		}
		break;
	case FLT_OVERFLOW:
		__try {
			// Note: floating point execution happens asynchronously.
			// So, the exception will not be handled until the next
			// floating point instruction.
			tmp = pow(FLT_MAX, 3);
		}
		// Use GetExceptionCode() function in the handler.
		__except (EXCEPTION_EXECUTE_HANDLER) {
			if (GetExceptionCode() == EXCEPTION_FLT_OVERFLOW)
				printf("Caught exception is: EXCEPTION_FLT_OVERFLOW");
			else
				printf("UNKNOWN exception: %x\n", GetExceptionCode());
		}
		break;
	default:
		break;
	}
	return 0;
}

// Usage manual
void usage(const _TCHAR *prog) {
	printf("Usage: \n");
	_tprintf(_T("\t%s -d\n"), prog);
	printf("\t\t\t for exception float divide by zero,\n");
	_tprintf(_T("\t%s -o\n"), prog);
	printf("\t\t\t for exception float overflow.\n");
}
