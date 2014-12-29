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
	__try {
		volatile float tmp = 0;
		switch (task) {
		case DIVIDE_BY_ZERO:
			tmp = 1 / tmp;
			break;
		case FLT_OVERFLOW:
			// Note: floating point execution happens asynchronously.
			// So, the exception will not be handled until the next floating
			// point instruction.
			tmp = pow(FLT_MAX, 3);
			break;
		default:
			break;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		printf("Well, it looks like we caught something.");
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
