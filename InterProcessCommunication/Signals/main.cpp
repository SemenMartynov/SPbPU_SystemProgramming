#include <windows.h>
#include <stdio.h>
#include <iostream>

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdout == INVALID_HANDLE_VALUE)
	{
		std::cout << "Error while getting input handle" << std::endl;
		return EXIT_FAILURE;
	}

	switch (fdwCtrlType) //тип сигнала
	{
	// Handle the CTRL-C signal.
	case CTRL_C_EVENT:
		SetConsoleTextAttribute(hStdout, FOREGROUND_RED | BACKGROUND_BLUE | FOREGROUND_INTENSITY);
		std::cout << "Ctrl-C event\n\n" << std::endl;
		SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		return(TRUE);
		// CTRL-CLOSE: confirm that the user wants to exit.
	case CTRL_CLOSE_EVENT:
		SetConsoleTextAttribute(hStdout, FOREGROUND_RED | BACKGROUND_BLUE | FOREGROUND_INTENSITY);
		std::cout << "Ctrl-Close event\n\n" << std::endl;
		SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		return(TRUE);
		// Pass other signals to the next handler.
	case CTRL_BREAK_EVENT:
		SetConsoleTextAttribute(hStdout, FOREGROUND_RED | BACKGROUND_BLUE | FOREGROUND_INTENSITY);
		std::cout << "Ctrl-Break event\n\n" << std::endl;
		SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		return FALSE;
	case CTRL_LOGOFF_EVENT:
		SetConsoleTextAttribute(hStdout, FOREGROUND_RED | BACKGROUND_BLUE | FOREGROUND_INTENSITY);
		std::cout << "Ctrl-Logoff event\n\n" << std::endl;
		SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		return FALSE;
	case CTRL_SHUTDOWN_EVENT:
		SetConsoleTextAttribute(hStdout, FOREGROUND_RED | BACKGROUND_BLUE | FOREGROUND_INTENSITY);
		std::cout << "Ctrl-Shutdown event\n\n" << std::endl;
		SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		return FALSE;
	default:
		return FALSE;
	}
}
int main(void)
{
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
	{
		std::cout << "The Control Handler is installed." << std::endl
			<< "-- Now try pressing Ctrl+C or Ctrl+Break, or" << std::endl
			<< "   try logging off or closing the console..." << std::endl << std::endl
			<< "(...waiting in a loop for events...)" << std::endl << std::endl;
		while (1){}
	}
	else
	{
		std::cout << "ERROR: Could not set control handler" << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}