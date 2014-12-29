/*  Task 6.
	Nested exception process;
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
		__try {
			RaiseException(EXCEPTION_FLT_DIVIDE_BY_ZERO, EXCEPTION_NONCONTINUABLE, 0, NULL);
		}
		__except ((GetExceptionCode() == EXCEPTION_FLT_OVERFLOW) ?
		EXCEPTION_EXECUTE_HANDLER :
								  EXCEPTION_CONTINUE_SEARCH)
		{
			printf("Internal handler in action.");
		}
	}
	__except ((GetExceptionCode() == EXCEPTION_FLT_DIVIDE_BY_ZERO) ?
	EXCEPTION_EXECUTE_HANDLER :
							  EXCEPTION_CONTINUE_SEARCH)
	{
		printf("External handler in action.");
	}
	return 0;
}