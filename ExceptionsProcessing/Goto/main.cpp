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

void usage(const _TCHAR *prog);

// Defines the entry point for the console application.
int _tmain(int argc, _TCHAR* argv[]) {
	// Floating point exceptions are masked by default.
	_clearfp();
	_controlfp_s(NULL, 0, _EM_OVERFLOW | _EM_ZERODIVIDE);

	__try {
		goto OUT_POINT;
		RaiseException(EXCEPTION_FLT_DIVIDE_BY_ZERO, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("Handler in action.");
	}
OUT_POINT:
	printf("A point outside the __try block.");
	return 0;
}