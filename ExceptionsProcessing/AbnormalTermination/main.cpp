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
	__finally
	{
		if (AbnormalTermination())
			printf("%s", "Abnormal termination in goto case\n");
		else
			printf("%s", "Normal termination  in goto case\n");
	}
OUT_POINT:
	__try {
		__leave;
		RaiseException(EXCEPTION_FLT_DIVIDE_BY_ZERO, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}
	__finally
	{
		if (AbnormalTermination())
			printf("%s", "Abnormal termination in __leave case");
		else
			printf("%s", "Normal termination  in __leave case");
	}
	
	return 0;
}