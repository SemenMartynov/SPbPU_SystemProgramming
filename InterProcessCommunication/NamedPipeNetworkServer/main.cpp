#include <windows.h> 
#include <stdio.h> 
#include <conio.h>
#include <tchar.h>
#include <strsafe.h>
#include "logger.h"

#define BUFSIZE 512

DWORD WINAPI InstanceThread(LPVOID);

int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);
	_tprintf(_T("Server is started.\n\n"));

	BOOL   fConnected = FALSE; // Флаг наличия подключенных клиентов
	DWORD  dwThreadId = 0; // Номер обслуживающего потока
	HANDLE hPipe = INVALID_HANDLE_VALUE; // Идентификатор канала
	HANDLE hThread = NULL; // Идентификатор обслуживающего потока
	LPTSTR lpszPipename = _T("\\\\.\\pipe\\$$MyPipe$$"); // Имя создаваемого канала

	//**************************************************************************
	// Создание SECURITY_ATTRIBUTES и SECURITY_DESCRIPTOR объектов
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	// Инициализация SECURITY_DESCRIPTOR значениями по-умолчанию
	if (InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) == 0) {
		double errorcode = GetLastError();
		writelog(_T("InitializeSecurityDescriptor failed, GLE=%d."), errorcode);
		_tprintf(_T("InitializeSecurityDescriptor failed, GLE=%d.\n"), errorcode);
		closelog();
		exit(5000);
	}
	// Установка поля DACL в SECURITY_DESCRIPTOR в NULL
	if (SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE) == 0)	{
		double errorcode = GetLastError();
		writelog(_T("SetSecurityDescriptorDacl failed, GLE=%d."), errorcode);
		_tprintf(_T("SetSecurityDescriptorDacl failed, GLE=%d.\n"), errorcode);
		closelog();
		exit(5001);
	}
	// Установка SECURITY_DESCRIPTOR в структуре SECURITY_ATTRIBUTES
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = FALSE; //запрещение наследования
	//**************************************************************************


	// Цикл ожидает клиентов и создаёт для них потоки обработки
	for (;;) {
		writelog(_T("Try to create named pipe on %s"), lpszPipename);
		_tprintf(_T("Try to create named pipe on %s\n"), lpszPipename);

		// Создаем канал:
		if ((hPipe = CreateNamedPipe(
			lpszPipename, // имя канала,
			PIPE_ACCESS_DUPLEX, // режим отрытия канала - двунаправленный,
			PIPE_TYPE_MESSAGE | // данные записываются в канал в виде потока сообщений,
			PIPE_READMODE_MESSAGE | // данные считываются в виде потока сообщений,
			PIPE_WAIT, // функции передачи и приема блокируются до их окончания,
			5, // максимальное число экземпляров каналов равно 5 (число клиентов),
			BUFSIZE, //размеры выходного и входного буферов канала,
			BUFSIZE,
			5000, // 5 секунд - длительность для функции WaitNamedPipe,
			&sa))// дескриптор безопасности
			== INVALID_HANDLE_VALUE) {
			double errorcode = GetLastError();
			writelog(_T("CreateNamedPipe failed, GLE=%d."), errorcode);
			_tprintf(_T("CreateNamedPipe failed, GLE=%d.\n"), errorcode);
			closelog();
			exit(1);
		}
		writelog(_T("Named pipe created successfully!"));
		_tprintf(_T("Named pipe created successfully!\n"));

		// Ожидаем соединения со стороны клиента
		writelog(_T("Waiting for connect..."));
		_tprintf(_T("Waiting for connect...\n"));
		fConnected = ConnectNamedPipe(hPipe, NULL) ?
		TRUE :
			 (GetLastError() == ERROR_PIPE_CONNECTED);

		// Если произошло соединение
		if (fConnected) {
			writelog(_T("Client connected!"));
			writelog(_T("Creating a processing thread..."));
			_tprintf(_T("Client connected!\nCreating a processing thread...\n"));

			// Создаём поток для обслуживания клиента 
			hThread = CreateThread(
				NULL,              // дескриптор защиты 
				0,                 // начальный размер стека
				InstanceThread,    // функция потока
				(LPVOID)hPipe,     // параметр потока
				0,                 // опции создания
				&dwThreadId);      // номер потока

			// Если поток создать не удалось - сообщаем об ошибке
			if (hThread == NULL) {
				double errorcode = GetLastError();
				writelog(_T("CreateThread failed, GLE=%d."), errorcode);
				_tprintf(_T("CreateThread failed, GLE=%d.\n"), errorcode);
				closelog();
				exit(1);
			}
			else CloseHandle(hThread);
		}
		else {
			// Если клиенту не удалось подключиться, закрываем канал
			CloseHandle(hPipe);
			writelog(_T("There are not connecrtion reqests."));
			_tprintf(_T("There are not connecrtion reqests.\n"));
		}
	}

	closelog();
	exit(0);
}

DWORD WINAPI InstanceThread(LPVOID lpvParam) {
	writelog(_T("Thread %d started!"), GetCurrentThreadId());
	_tprintf(_T("Thread %d started!\n"), GetCurrentThreadId());
	HANDLE hPipe = (HANDLE)lpvParam; // Идентификатор канала
	// Буфер для хранения полученного и передаваемого сообщения
	_TCHAR* chBuf = (_TCHAR*)HeapAlloc(GetProcessHeap(), 0, BUFSIZE * sizeof(_TCHAR));
	DWORD readbytes, writebytes; // Число байт прочитанных и переданных

	while (1) {
		// Получаем очередную команду через канал Pipe
		if (ReadFile(hPipe, chBuf, BUFSIZE*sizeof(_TCHAR), &readbytes, NULL)) {
			// Посылаем эту команду обратно клиентскому приложению
			if (!WriteFile(hPipe, chBuf, (lstrlen(chBuf) + 1)*sizeof(_TCHAR), &writebytes, NULL))
				break;
			// Выводим принятую команду на консоль
			writelog(_T("Thread %d: Get client msg: %s"), GetCurrentThreadId(), chBuf);
			_tprintf(TEXT("Get client msg: %s\n"), chBuf);
			// Если пришла команда "exit", завершаем работу приложения
			if (!_tcsncmp(chBuf, L"exit", 4))
				break;
		} else {
			double errorcode = GetLastError();
			writelog(_T("Thread %d: GReadFile: Error %ld"), GetCurrentThreadId(), errorcode);
			_tprintf(TEXT("ReadFile: Error %ld\n"), errorcode);
			_getch();
			break;
		}
	}

	// Освобождение ресурсов 
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	HeapFree(GetProcessHeap(), 0, chBuf);

	writelog(_T("Thread %d: InstanceThread exitting."), GetCurrentThreadId());
	_tprintf(TEXT("InstanceThread exitting.\n"));
	return 0;
}
