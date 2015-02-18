#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <strsafe.h>
#include "logger.h"

#define BUFSIZE 512

int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);
	_tprintf(_T("Client is started!\n\n"));

	HANDLE hPipe = INVALID_HANDLE_VALUE; // Идентификатор канала
	LPTSTR lpszPipename = _T("\\\\.\\pipe\\$$MyPipe$$"); // Имя создаваемого канала Pipe
	_TCHAR chBuf[BUFSIZE]; // Буфер для передачи данных через канал
	DWORD readbytes, writebytes; // Число байт прочитанных и переданных

	writelog(_T("Try to use WaitNamedPipe..."));
	_tprintf(_T("Try to use WaitNamedPipe...\n"));
	// Пытаемся открыть именованный канал, если надо - ожидаем его освобождения
	while (1) {
		// Создаем канал с процессом-сервером:
		hPipe = CreateFile(
			lpszPipename, // имя канала,
			GENERIC_READ // текущий клиент имеет доступ на чтение,
			| GENERIC_WRITE, // текущий клиент имеет доступ на запись,
			0, // тип доступа,
			NULL, // атрибуты защиты,
			OPEN_EXISTING, // открывается существующий файл,
			0, // атрибуты и флаги для файла,
			NULL); // доступа к файлу шаблона.

		// Продолжаем работу, если канал создать удалось 
		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		// Выход, если ошибка связана не с занятым каналом. 
		double errorcode = GetLastError();
		if (errorcode != ERROR_PIPE_BUSY) {
			writelog(_T("Could not open pipe. GLE=%d\n"), errorcode);
			_tprintf(_T("Could not open pipe. GLE=%d\n"), errorcode);
			closelog();
			exit(1);
		}

		// Если все каналы заняты, ждём 20 секунд 
		if (!WaitNamedPipe(lpszPipename, 20000)) {
			writelog(_T("Could not open pipe: 20 second wait timed out."));
			_tprintf(_T("Could not open pipe: 20 second wait timed out."));
			closelog();
			exit(2);
		}
	}

	// Выводим сообщение о создании канала
	writelog(_T("Successfully connected!"));
	_tprintf(_T("Successfully connected!\n\nInput message...\n"));
	// Цикл обмена данными с серверным процессом
	while (1) {
		// Выводим приглашение для ввода команды
		_tprintf(_T("cmd>"));
		// Вводим текстовую строку
		_fgetts(chBuf, BUFSIZE, stdin);
		// Заносим строку в протокол
		writelog(_T("Client sended: %s"), chBuf);
		// Передаем введенную строку серверному процессу в качестве команды
		if (!WriteFile(hPipe, chBuf, (lstrlen(chBuf) + 1)*sizeof(TCHAR), &writebytes, NULL)) {
			writelog(_T("connection refused\n"));
			_tprintf(_T("connection refused\n"));
			break;
		}
		// Получаем эту же команду обратно от сервера
		if (ReadFile(hPipe, chBuf, BUFSIZE*sizeof(TCHAR), &readbytes, NULL)) {
			writelog(_T("Received from server: %s"), chBuf);
			_tprintf(_T("Received from server: %s\n"), chBuf);
		} else {
			// Если произошла ошибка, выводим ее код и завершаем работу приложения
			double errorcode = GetLastError();
			writelog(_T("ReadFile: Error %ld\n"), errorcode);
			_tprintf(_T("ReadFile: Error %ld\n"), errorcode);
			_getch();
			break;
		}
		// В ответ на команду "exit" завершаем цикл обмена данными с серверным процессом
		if (!_tcsncmp(chBuf, L"exit", 4)) {
			writelog(_T("Processing exit code"));
			break;
		}
	}
	// Закрываем идентификатор канала
	CloseHandle(hPipe);
	closelog();
	return 0;
}