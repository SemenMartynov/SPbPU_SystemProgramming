/*  Task 10.
	Use the final handler finally;
*/

// IMPORTANT: Don't forget to disable Enhanced Instructions!!!
// Properties -> Configuration Properties -> C/C++ -> Code Generation ->
// Enable Enhanced Instruction Set = No Enhanced Instructions (/arch:IA32)

#include <stdio.h>
#include <tchar.h>
#include <cfloat>
#include <excpt.h>

void usage(const _TCHAR *prog);

// Defines the entry point for the console application.
int _tmain(int argc, _TCHAR* argv[]) {
	// Floating point exceptions are masked by default.
	_clearfp();
	_controlfp_s(NULL, 0, _EM_OVERFLOW | _EM_ZERODIVIDE);

	__try {
		// No exception
	}
	__finally
	{
		printf("Thre is no exception, but the handler is called.\n");
	}

	return 0;
}