#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "logger.h"

//размер буфера для сообщений
#define BUF_SIZE 100

int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);

	//буфер приема/передачи
	_TCHAR buf[BUF_SIZE];
	//число прочитанных/переданных байт
	DWORD readbytes, writebytes;

	//дескрипторы канала для передачи от сервера клиенту
	HANDLE hReadPipeFromServToClient, hWritePipeFromServToClient;
	//дескрипторы канала для передачи от клиента серверу
	HANDLE hReadPipeFromClientToServ, hWritePipeFromClientToServ;

	//чтобы сделать дескрипторы наследуемыми
	SECURITY_ATTRIBUTES PipeSA = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	//создаем канал для передачи от сервера клиенту, сразу делаем дескрипторы наследуемыми
	if (CreatePipe(&hReadPipeFromServToClient, &hWritePipeFromServToClient, &PipeSA, 0) == 0) {
		double errorcode = GetLastError();
		writelog(_T("Can't create anonymous pipe from server to client, GLE=%d."), errorcode);
		_tprintf(_T("Can't create anonymous pipe from server to client, GLE=%d."), errorcode);
		closelog();
		exit(1);
	}
	writelog(_T("Anonymous pipe from server to client created"));
	_tprintf(_T("Anonymous pipe from server to client created\n"));

	//создаем канал для передачи от клиента серверу, сразу делаем дескрипторы наследуемыми
	if (CreatePipe(&hReadPipeFromClientToServ, &hWritePipeFromClientToServ, &PipeSA, 0) == 0) {
		double errorcode = GetLastError();
		writelog(_T("Can't create anonymous pipe from client to server, GLE=%d."), errorcode);
		_tprintf(_T("Can't create anonymous pipe from client to server, GLE=%d."), errorcode);
		closelog();
		exit(2);
	}
	writelog(_T("Anonymous pipe from client to server created"));
	_tprintf(_T("Anonymous pipe from client to server created\n"));

	PROCESS_INFORMATION processInfo_Client; // информация о процессе-клиенте
	//структура, которая описывает внешний вид основного окна и содержит
	// дескрипторы стандартных устройств нового процесса
	STARTUPINFO startupInfo_Client;

	//процесс-клиент будет иметь те же параметры запуска, что и сервер,
	// за исключением дескрипторов ввода, вывода и ошибок
	GetStartupInfo(&startupInfo_Client);
	//устанавливаем поток ввода
	startupInfo_Client.hStdInput = hReadPipeFromServToClient;
	//установим поток вывода
	startupInfo_Client.hStdOutput = hWritePipeFromClientToServ;
	//установим поток ошибок
	startupInfo_Client.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	//устанавливаем наследование
	startupInfo_Client.dwFlags = STARTF_USESTDHANDLES;
	//создаем процесс клиента
	if (!CreateProcess(_T("AnonymousPipeClient.exe"), // имя исполняемого модуля
		NULL, //командная строка
		NULL, //атрибуты безопасности процесса
		NULL, //атрибуты безопасности потока
		TRUE, //флаг наследования описателя
		CREATE_NEW_CONSOLE, //флаги создания
		NULL, //новый блок окружения
		NULL, //имя текущей директории
		&startupInfo_Client, // STARTUPINFO
		&processInfo_Client)) { //PROCESS_INFORMATION
		writelog(_T("Can't create process, GLE=%d."), GetLastError());
		_wperror(_T("Create process"));
		closelog();
		exit(3);
	}
	writelog(_T("New process created"));
	_tprintf(_T("New process created\n"));

	//закрываем дескрипторы созданного процесса и его потока
	CloseHandle(processInfo_Client.hThread);
	CloseHandle(processInfo_Client.hProcess);
	//закрываем ненужные дескрипторы каналов, которые не использует сервер
	CloseHandle(hReadPipeFromServToClient);
	CloseHandle(hWritePipeFromClientToServ);

	// Начинаем взаимодействие с клиентом через анонимный канал
	for (int i = 0; i < 10; i++) {
		//читаем данные из канала от клиента
		if (!ReadFile(hReadPipeFromClientToServ, buf, sizeof(buf), &readbytes, NULL)) {
			double errorcode = GetLastError();
			writelog(_T("Impossible to use readfile, GLE=%d."), errorcode);
			_tprintf(_T("Impossible to use readfile, GLE=%d."), errorcode);
			closelog();
			exit(4);
		}
		_tprintf(_T("Get from client: \"%s\"\n"), buf);
		writelog(_T("Get from client: \"%s\""), buf);
		//пишем данные в канал клиенту
		if (!WriteFile(hWritePipeFromServToClient, buf, readbytes, &writebytes, NULL)) {
			double errorcode = GetLastError();
			writelog(_T("Impossible to use writefile, GLE=%d."), errorcode);
			_tprintf(_T("Impossible to use writefile, GLE=%d."), errorcode);
			closelog();
			exit(5);
		}
	}
	//закрываем HANDLE каналов
	CloseHandle(hReadPipeFromClientToServ);
	CloseHandle(hWritePipeFromServToClient);

	closelog();
	exit(0);
}
