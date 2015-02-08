#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <strsafe.h>

#define BUFSIZE 512

int _tmain(int argc, TCHAR *argv[])
{
	_tprintf(TEXT("Client is started!\n\n"));

	HANDLE hPipe = INVALID_HANDLE_VALUE; // Идентификатор канала
	LPTSTR lpszPipename = TEXT("\\\\192.168.124.2\\pipe\\$$MyPipe$$"); // Имя создаваемого канала Pipe
	TCHAR chBuf[BUFSIZE]; // Буфер для передачи данных через канал
	DWORD readbytes, writebytes; // Число байт прочитанных и переданных

	// Создание SECURITY_ATTRIBUTES и SECURITY_DESCRIPTOR объектов
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	// Инициализация SECURITY_DESCRIPTOR значениями по-умолчанию
	if (InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) == 0)
	{
		_tprintf(TEXT("InitializeSecurityDescriptor failed with error %d\n"), GetLastError());
		_getch();
		return -1;
	}
	// Установка поля DACL в SECURITY_DESCRIPTOR в NULL
	if (SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE) == 0)
	{
		_tprintf(TEXT("SetSecurityDescriptorDacl failed with error %d\n"), GetLastError());
		_getch();
		return -1;
	}
	// Установка SECURITY_DESCRIPTOR в структуре SECURITY_ATTRIBUTES
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = FALSE; //запрещение наследования

	_tprintf(TEXT("Try to use WaitNamedPipe...\n"));
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
		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
			_getch();
			return -1;
		}

		// Если все каналы заняты, ждём 20 секунд 
		if (!WaitNamedPipe(lpszPipename, 20000))
		{
			_tprintf(TEXT("Could not open pipe: 20 second wait timed out."));
			_getch();
			return -1;
		}
	}

	// Выводим сообщение о создании канала
	_tprintf(TEXT("Successfully connected!\n\nInput message...\n"));
	// Цикл обмена данными с серверным процессом
	while (1)
	{
		// Выводим приглашение для ввода команды
		_tprintf(TEXT("cmd>"));
		// Вводим текстовую строку
		_fgetts(chBuf, BUFSIZE, stdin);
		// Передаем введенную строку серверному процессу в качестве команды
		if (!WriteFile(hPipe, chBuf, (lstrlen(chBuf) + 1)*sizeof(TCHAR), &writebytes, NULL))
		{
			_tprintf(TEXT("connection refused\n"));
			break;
		}
		// Получаем эту же команду обратно от сервера
		if (ReadFile(hPipe, chBuf, BUFSIZE*sizeof(TCHAR), &readbytes, NULL))
			_tprintf(TEXT("Received from server: %s\n"), chBuf);
		// Если произошла ошибка, выводим ее код и завершаем работу приложения
		else {
			_tprintf(TEXT("ReadFile: Error %ld\n"), GetLastError());
			_getch();
			break;
		}
		// В ответ на команду "exit" завершаем цикл обмена данными с серверным процессом
		if (!_tcsncmp(chBuf, L"exit", 4))
			break;
	}

	// Закрываем идентификатор канала
	CloseHandle(hPipe);

	_tprintf(TEXT("Press ENTER to terminate connection and exit\n"));
	_getch();
	return 0;
}