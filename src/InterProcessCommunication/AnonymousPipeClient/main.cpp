#include <stdio.h>
#include <Windows.h>
#include "logger.h"

int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);

	_TCHAR strtosend[100]; //строка для передачи
	_TCHAR getbuf[100]; //буфер приема
	//число переданных и принятых байт
	DWORD bytessended, bytesreaded;
	
	// Начинаем взаимодействие с сервером через анонимный канал
	for (int i = 0; i < 10; i++) {
		//формирование строки для передачи
		bytessended = swprintf_s(strtosend, _T("Message num %d"), i + 1);
		strtosend[bytessended++] = _T('\0');

		writelog(_T("Client sended: \"%s\""), strtosend);
		fwprintf(stderr, _T("Client sended: \"%s\"\n"), strtosend);

		//передача данных
		if (!WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), strtosend, bytessended * sizeof(wchar_t), &bytesreaded, NULL)) {
			double errorcode = GetLastError();
			writelog(_T("Error with writeFile, GLE=%d."), errorcode);
			fwprintf(stderr, _T("Error with writeFile, GLE=%d."), errorcode);
			closelog();
			exit(1);
		}
		Sleep(1000);
		//прием ответа от сервера
		if (!ReadFile(GetStdHandle(STD_INPUT_HANDLE), getbuf, 100, &bytesreaded, NULL)) {
			double errorcode = GetLastError();
			writelog(_T("Error with readFile, GLE=%d."), errorcode);
			fwprintf(stderr, _T("Error with readFile, GLE=%d."), errorcode);
			closelog();
			exit(2);
		}
		writelog(_T("Get msg from server: \"%s\""), getbuf);
		fwprintf(stderr, _T("Get msg from server: \"%s\"\n"), getbuf);
	}
	Sleep(10000);
	closelog();
	return 0;
}