#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <strsafe.h>
#include "Logger.h"

#define BUFSIZE 512
//Init log
Logger log(_T("ReaderWriterClient"), GetCurrentProcessId());

int _tmain(int argc, _TCHAR* argv[]) {
	log.loudlog(_T("Client is started!\n\n"));

	HANDLE hPipe = INVALID_HANDLE_VALUE; // Идентификатор канала
	LPTSTR lpszPipename = _T("\\\\.\\pipe\\$$MyPipe$$"); // Имя создаваемого канала Pipe
	_TCHAR chBuf[BUFSIZE]; // Буфер для передачи данных через канал
	DWORD readbytes; // Число байт прочитанных и переданных

	log.loudlog(_T("Try to use WaitNamedPipe..."));

	// Пытаемся открыть именованный канал, если надо - ожидаем его освобождения
	while (1) {
		// Создаем канал с процессом-клиентом:
		hPipe = CreateFile(
			lpszPipename,		// имя канала,
			GENERIC_READ,		// текущий клиент имеет доступ на чтение,
			0,					// тип доступа,
			NULL,				// атрибуты защиты,
			OPEN_EXISTING,		// открывается существующий файл,
			0,					// атрибуты и флаги для файла,
			NULL);				// доступа к файлу шаблона.

		// Продолжаем работу, если канал создать удалось
		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		// Выход, если ошибка связана не с занятым каналом.
		double errorcode = GetLastError();
		if (errorcode != ERROR_PIPE_BUSY) {
			log.loudlog(_T("Could not open pipe. GLE=%d\n"), errorcode);
			exit(1);
		}

		// Если все каналы заняты, ждём 20 секунд 
		if (!WaitNamedPipe(lpszPipename, 20000)) {
			log.loudlog(_T("Could not open pipe: 20 second wait timed out."));
			exit(2);
		}
	}

	// Выводим сообщение о создании канала
	log.loudlog(_T("Successfully connected!"));
	// Цикл обмена данными с серверным процессом
	while (1) {
		// Получаем команду от сервера
		if (ReadFile(hPipe, chBuf, BUFSIZE * sizeof(TCHAR), &readbytes, NULL)) {
			log.loudlog(_T("Received from server: %s"), chBuf);
		} else {
			// Если произошла ошибка, выводим ее код и завершаем работу приложения
			double errorcode = GetLastError();
			log.loudlog(_T("ReadFile: Error %ld\n"), errorcode);
			_getch();
			break;
		}
		// В ответ на команду "exit" завершаем цикл обмена данными с серверным процессом
		if (!_tcsncmp(chBuf, L"exit", 4)) {
			log.loudlog(_T("Processing exit code"));
			break;
		}
	}
	// Закрываем идентификатор канала
	CloseHandle(hPipe);

	// Завершение работы
	log.loudlog(_T("All tasks are done!"));
	_getch();
	exit(0);
	return 0;
}