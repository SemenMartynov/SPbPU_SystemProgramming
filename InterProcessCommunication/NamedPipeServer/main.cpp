//http://msdn.microsoft.com/ru-ru/windows/desktop/aa365802%28v=vs.85%29

#include <windows.h> 
#include <stdio.h> 
#include <conio.h>
#include <tchar.h>
#include <strsafe.h>

#define BUFSIZE 512

DWORD WINAPI InstanceThread(LPVOID);

int _tmain(int argc, TCHAR *argv[])
{
	_tprintf(TEXT("Server is started.\n\n"));

	BOOL   fConnected = FALSE; // Флаг наличия подключенных клиентов
	DWORD  dwThreadId = 0; // Номер обслуживающего потока
	HANDLE hPipe = INVALID_HANDLE_VALUE; // Идентификатор канала
	HANDLE hThread = NULL; // Идентификатор обслуживающего потока
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\$$MyPipe$$"); // Имя создаваемого канала Pipe

	// Цикл ожидает клиентов и создаёт для них потоки обработки
	for (;;)
	{
		_tprintf(TEXT("Try to create named pipe on %s\n"), lpszPipename);

		// Создаем канал:
		hPipe = CreateNamedPipe(
			// имя канала,
			lpszPipename,
			// режим отрытия канала - двунаправленный,
			PIPE_ACCESS_DUPLEX,
			// данные записываются в канал в виде потока сообщений,
			PIPE_TYPE_MESSAGE |
			// данные считываются в виде потока сообщений,
			PIPE_READMODE_MESSAGE |
			// функции передачи и приема блокируются до их окончания,
			PIPE_WAIT,
			// максимальное число экземпляров каналов не ограничено,
			PIPE_UNLIMITED_INSTANCES,
			//размеры выходного и входного буферов канала,
			BUFSIZE,
			BUFSIZE,
			// 5 секунд - длительность для функции WaitNamedPipe,
			5000,
			// дескриптор безопасности по умолчанию.
			NULL);

		// Если возникла ошибка, выводим ее код и завершаем	работу приложения
		if (hPipe == INVALID_HANDLE_VALUE)
		{
			_tprintf(TEXT("CreateNamedPipe failed, GLE=%d.\n"), GetLastError());
			_getch();
			return -1;
		}
		_tprintf(TEXT("Named pipe created successfully!\n\n"));

		// Ожидаем соединения со стороны клиента
		_tprintf(TEXT("Waiting for connect...\n"));
		fConnected = ConnectNamedPipe(hPipe, NULL) ?
			TRUE :
			(GetLastError() == ERROR_PIPE_CONNECTED);

		// Если произошло соединение
		if (fConnected)
		{
			_tprintf(TEXT("Client connected!\n\nCreating a processing thread...\n"));

			// Создаём поток для обслуживания клиента 
			hThread = CreateThread(
				NULL,              // дескриптор защиты 
				0,                 // начальный размер стека
				InstanceThread,    // функция потока
				(LPVOID)hPipe,     // параметр потока
				0,                 // опции создания
				&dwThreadId);      // номер потока

			// Если поток создать не удалось - сообщаем об ошибке
			if (hThread == NULL)
			{
				_tprintf(TEXT("CreateThread failed, GLE=%d.\n"), GetLastError());
				_getch();
				return -1;
			}
			else CloseHandle(hThread);
		}
		else
			// Если клиенту не удалось подключиться, закрываем канал
			CloseHandle(hPipe);
	}

	return 0;
}

DWORD WINAPI InstanceThread(LPVOID lpvParam)
{
	_tprintf(TEXT("Thread started!\n"));
	HANDLE hPipe = (HANDLE)lpvParam; // Идентификатор канала
	// Буфер для хранения полученного и передаваемого сообщения
	TCHAR* chBuf = (TCHAR*)HeapAlloc(GetProcessHeap(), 0, BUFSIZE * sizeof(TCHAR));
	DWORD readbytes, writebytes; // Число байт прочитанных и переданных

	while (1)
	{
		// Получаем очередную команду через канал Pipe
		if (ReadFile(hPipe, chBuf, BUFSIZE*sizeof(TCHAR), &readbytes, NULL))
		{
			// Посылаем эту команду обратно клиентскому приложению
			if (!WriteFile(hPipe, chBuf, (lstrlen(chBuf) + 1)*sizeof(TCHAR), &writebytes, NULL))
				break;
			// Выводим принятую команду на консоль
			_tprintf(TEXT("Get client msg: %s\n"), chBuf);
			// Если пришла команда "exit", завершаем работу приложения
			if (!_tcsncmp(chBuf, L"exit", 4))
				break;
		}
		else
		{
			_tprintf(TEXT("ReadFile: Error %ld\n"), GetLastError());
			_getch();
			break;
		}
	}

	// Освобождение ресурсов 
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	HeapFree(hPipe, 0, chBuf);

	_tprintf(TEXT("InstanceThread exitting.\n"));
	return 1;
}
