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

void translator(unsigned int u, EXCEPTION_POINTERS* pExp);

// Defines the entry point for the console application.
int _tmain(int argc, _TCHAR* argv[]) {
	// Floating point exceptions are masked by default.
	_clearfp();
	_controlfp_s(NULL, 0, _EM_OVERFLOW | _EM_ZERODIVIDE);

	try {
		_set_se_translator(translator);
		RaiseException(EXCEPTION_FLT_DIVIDE_BY_ZERO, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}
	catch (std::overflow_error e) {
		printf("Error: %s", e.what());
	}
	return 0;
}

void translator(unsigned int u, EXCEPTION_POINTERS* pExp) {
	if (u == EXCEPTION_FLT_DIVIDE_BY_ZERO)
		throw std::overflow_error("EXCEPTION_FLT_DIVIDE_BY_ZERO");
}
