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
	LPTSTR lpszPipename = TEXT("\\\\192.168.124.235\\pipe\\$$MyPipe$$"); // Имя создаваемого канала Pipe
	_TCHAR chBuf[BUFSIZE]; // Буфер для передачи данных через канал
	DWORD readbytes, writebytes; // Число байт прочитанных и переданных

	// Создание SECURITY_ATTRIBUTES и SECURITY_DESCRIPTOR объектов
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	// Инициализация SECURITY_DESCRIPTOR значениями по-умолчанию
	if (InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) == 0) {
		double errorcode = GetLastError();
		writelog(_T("InitializeSecurityDescriptor failed, GLE = %d"), errorcode);
		_tprintf(_T("InitializeSecurityDescriptor failed, GLE = %d\n"), errorcode);
		closelog();
		exit(2);
	}
	// Установка поля DACL в SECURITY_DESCRIPTOR в NULL
	if (SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE) == 0)	{
		double errorcode = GetLastError();
		writelog(_T("SetSecurityDescriptorDacl failed, GLE = %d"), errorcode);
		_tprintf(_T("SetSecurityDescriptorDacl failed, GLE = %d\n"), errorcode);
		closelog();
		exit(3);
	}
	// Установка SECURITY_DESCRIPTOR в структуре SECURITY_ATTRIBUTES
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = FALSE; //запрещение наследования

	writelog(_T("Try to use WaitNamedPipe..."));
	_tprintf(_T("Try to use WaitNamedPipe...\n"));
	// Пытаемся открыть именованный канал, если надо - ожидаем его освобождения
	while (1)
	{
		// Создаем канал с процессом-сервером:
		hPipe = CreateFile(
			lpszPipename, // имя канала,
			GENERIC_READ // текущий клиент имеет доступ на чтение,
			| GENERIC_WRITE, // текущий клиент имеет доступ на запись,
			0, // тип доступа,
			&sa, // атрибуты защиты,
			OPEN_EXISTING, // открывается существующий файл,
			0, // атрибуты и флаги для файла,
			NULL); // доступа к файлу шаблона.

		// Продолжаем работу, если канал создать удалось 
		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		// Выход, если ошибка связана не с занятым каналом. 
		if (GetLastError() != ERROR_PIPE_BUSY) {
			double errorcode = GetLastError();
			writelog(_T("Could not open pipe, GLE = %d"), errorcode);
			_tprintf(_T("Could not open pipe, GLE = %d\n"), errorcode);
			closelog();
			exit(4);
		}

		// Если все каналы заняты, ждём 20 секунд 
		if (!WaitNamedPipe(lpszPipename, 20000)) {
			double errorcode = GetLastError();
			writelog(_T("Could not open pipe: 20 second wait timed out, GLE = %d"), errorcode);
			_tprintf(_T("ould not open pipe: 20 second wait timed out, GLE = %d\n"), errorcode);
			closelog();
			exit(5);
		}
	}

	// Выводим сообщение о создании канала
	writelog(_T("Successfully connected"));
	_tprintf(_T("Successfully connected!\n\nInput message...\n"));
	// Цикл обмена данными с серверным процессом
	while (1)
	{
		// Выводим приглашение для ввода команды
		_tprintf(TEXT("cmd>"));
		// Вводим текстовую строку
		_fgetts(chBuf, BUFSIZE, stdin);
		writelog(_T("Inpust string is %s"), chBuf);
		// Передаем введенную строку серверному процессу в качестве команды
		if (!WriteFile(hPipe, chBuf, (lstrlen(chBuf) + 1)*sizeof(TCHAR), &writebytes, NULL)) {
			double errorcode = GetLastError();
			writelog(_T("Connection refused, GLE = %d"), errorcode);
			_tprintf(_T("Connection refused, GLE = %d\n"), errorcode);
			break;
		}
		// Получаем эту же команду обратно от сервера
		if (ReadFile(hPipe, chBuf, BUFSIZE*sizeof(TCHAR), &readbytes, NULL)) {
			writelog(_T("Received from server: %s\n"), chBuf);
			_tprintf(_T("Received from server: %s\n"), chBuf);
		}
		// Если произошла ошибка, выводим ее код и завершаем работу приложения
		else {
			double errorcode = GetLastError();
			writelog(_T("ReadFile: Error %ld"), errorcode);
			_tprintf(_T("ReadFile: Error %ld\n"), errorcode);
			break;
		}
		// В ответ на команду "exit" завершаем цикл обмена данными с серверным процессом
		if (!_tcsncmp(chBuf, L"exit", 4))
			break;
	}

	// Закрываем идентификатор канала
	CloseHandle(hPipe);

	closelog();
	_tprintf(TEXT("Press ENTER to terminate connection and exit\n"));
	_getch();
	return 0;
}