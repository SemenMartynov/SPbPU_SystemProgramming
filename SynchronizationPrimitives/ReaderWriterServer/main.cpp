#include <windows.h> 
#include <stdio.h> 
#include <conio.h>
#include <tchar.h>
#include <strsafe.h>
#include "Logger.h"

#define BUFSIZE 512

DWORD WINAPI InstanceThread(LPVOID);
HANDLE CreateAndStartWaitableTimer(int);
DWORD WINAPI ThreadTimeManagerHandler(LPVOID);
DWORD WINAPI ThreadWriter(LPVOID);

//Init log
Logger log(_T("ReaderWriterServer"), -1);
// инструмент синхронизации:
SRWLOCK lock;
CONDITION_VARIABLE condread;
int message; // сообщение
bool isDone = false; //флаг завершения

int _tmain(int argc, _TCHAR* argv[]) {
	log.loudlog(_T("Server is started.\n\n"));

	BOOL   fConnected = FALSE; // Флаг наличия подключенных клиентов
	DWORD  dwThreadId = 0; // Номер обслуживающего потока
	HANDLE hPipe = INVALID_HANDLE_VALUE; // Идентификатор канала
	HANDLE hThread = NULL; // Идентификатор обслуживающего потока
	HANDLE service[2]; // Идентификатор потока писателя и таймера
	LPTSTR lpszPipename = _T("\\\\.\\pipe\\$$MyPipe$$"); // Имя создаваемого канала

	// начальное сообщение
	message = 0;
	InitializeSRWLock(&lock);
	InitializeConditionVariable(&condread);

	// Создание потока-таймера
	log.loudlog(_T("Time Manager creation!"));
	service[0] = CreateThread(
		NULL,				// дескриптор защиты 
		0,					// начальный размер стека
		ThreadTimeManagerHandler,// функция потока
		(LPVOID)5,			// параметр потока (5 секунд)
		NULL,				// опции создания
		NULL);				// номер потока

	// Создание потока-писателя
	log.loudlog(_T("Writer creation!"));
	service[1] = CreateThread(
		NULL,              // дескриптор защиты 
		0,                 // начальный размер стека
		ThreadWriter,	   // функция потока
		NULL,		       // параметр потока
		NULL,              // опции создания
		NULL);		       // номер потока

	// Ожидаем соединения со стороны клиента
	log.loudlog(_T("Waiting for connect..."));
	// Цикл ожидает клиентов и создаёт для них потоки обработки
	while (isDone != true) {
		// Создаем канал:
		log.loudlog(_T("Try to create named pipe on %s"), lpszPipename);
		if ((hPipe = CreateNamedPipe(
			lpszPipename,		// имя канала,
			PIPE_ACCESS_DUPLEX,	// режим отрытия канала - двунаправленный,
			PIPE_TYPE_MESSAGE |	// данные записываются в канал в виде потока сообщений,
			PIPE_WAIT,			// функции передачи и приема блокируются до их окончания,
			PIPE_UNLIMITED_INSTANCES,// максимальное число экземпляров каналов не ограничено,
			BUFSIZE * sizeof(_TCHAR),//размеры выходного и входного буферов канала,
			BUFSIZE * sizeof(_TCHAR),
			5000,				// 5 секунд - длительность для функции WaitNamedPipe,
			NULL))				// дескриптор безопасности по умолчанию.
			== INVALID_HANDLE_VALUE) {
			log.loudlog(_T("CreateNamedPipe failed, GLE=%d."), GetLastError());
			exit(1);
		}
		log.loudlog(_T("Named pipe created successfully!"));

		// Если произошло соединение
		if (ConnectNamedPipe(hPipe, NULL)) {
			log.loudlog(_T("Client connected!"));

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
				log.loudlog(_T("CreateThread failed, GLE=%d."), errorcode);
				exit(1);
			}
			else CloseHandle(hThread);
		}
		else {
			// Если клиенту не удалось подключиться, закрываем канал
			CloseHandle(hPipe);
			log.loudlog(_T("There are not connecrtion reqests."));
		}
	}

	//ожидаем завершения всех потоков
	WaitForMultipleObjects(2, service, TRUE, INFINITE);

	//закрываем handle потоков
	for (int i = 0; i != 2; ++i)
		CloseHandle(service[i]);

	// Завершение работы
	log.loudlog(_T("All tasks are done!"));
	_getch();
	exit(0);
}

DWORD WINAPI InstanceThread(LPVOID lpvParam) {
	log.loudlog(_T("Thread %d started!"), GetCurrentThreadId());
	HANDLE hPipe = (HANDLE)lpvParam; // Идентификатор канала
	HANDLE hHeap = GetProcessHeap(); // локальная куча
	// Буфер для хранения передаваемого сообщения
	_TCHAR* chBuf = (_TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));
	DWORD writebytes; // Число байт прочитанных и переданных

	while (isDone != true) {
		//задержка
		Sleep(100);

		// Захват объекта синхронизации (совместный доступ!)
		log.quietlog(_T("Waining for Slim Reader/Writer (SRW) Lock"));
		AcquireSRWLockShared(&lock);
		SleepConditionVariableSRW(&condread, &lock, INFINITE, CONDITION_VARIABLE_LOCKMODE_SHARED);
		log.quietlog(_T("Get SRW Lock"));

		// Посылаем эту команду клиентскому приложению
		swprintf_s(chBuf, BUFSIZE, L"%i", message);
		if (WriteFile(hPipe, chBuf, (lstrlen(chBuf) + 1)*sizeof(_TCHAR), &writebytes, NULL)) {
			// Выводим сообщение на консоль
			log.loudlog(_T("Server %d: send msg: %s"), GetCurrentThreadId(), chBuf);
		}
		else {
			log.loudlog(_T("Thread %d: WriteFile: Error %ld"), GetCurrentThreadId(), GetLastError());
			break;
		}

		//освобождение объекта синхронизации
		log.quietlog(_T("Release SRW Lock"));
		ReleaseSRWLockShared(&lock);
	}
	// завершаем работу приложения
	StringCchCopy(chBuf, BUFSIZE, L"exit");
	WriteFile(hPipe, chBuf, (lstrlen(chBuf) + 1)*sizeof(_TCHAR), &writebytes, NULL);

	// Освобождение ресурсов 
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);

	HeapFree(hHeap, 0, chBuf);

	log.quietlog(_T("Thread %d: InstanceThread exitting."), GetCurrentThreadId());
	return 0;
}

DWORD WINAPI ThreadWriter(LPVOID lpvParam) {
	log.loudlog(_T("Writer thread %d started!"), GetCurrentThreadId());

	while (isDone != true) {
		//задержка
		Sleep(200);

		// Захват объекта синхронизации (монопольный доступ!)
		log.quietlog(_T("Writer: Waining for Slim Reader/Writer (SRW) Lock"));
		AcquireSRWLockExclusive(&lock);
		log.quietlog(_T("Writer: Get SRW Lock"));

		// меняем значение сообщения
		++message;

		//освобождение объекта синхронизации
		log.quietlog(_T("Writer: Release SRW Lock"));
		WakeAllConditionVariable(&condread);
		ReleaseSRWLockExclusive(&lock);
	}

	log.quietlog(_T("Thread %d: Writer Thread exitting."), GetCurrentThreadId());
	return 0;
}

//создание, установка и запуск таймера
HANDLE CreateAndStartWaitableTimer(int sec) {
	__int64 end_time;
	LARGE_INTEGER end_time2;
	HANDLE tm = CreateWaitableTimer(NULL, false, _T("Timer!"));
	end_time = -1 * sec * 10000000;
	end_time2.LowPart = (DWORD)(end_time & 0xFFFFFFFF);
	end_time2.HighPart = (LONG)(end_time >> 32);
	SetWaitableTimer(tm, &end_time2, 0, NULL, NULL, false);
	return tm;
}

DWORD WINAPI ThreadTimeManagerHandler(LPVOID prm) {
	int ttl = (int)prm;
	if (ttl < 0) {
		//завершение по команде оператора
		_TCHAR buf[100];
		while (1) {
			fgetws(buf, sizeof(buf), stdin);
			if (buf[0] == _T('s')) {
				log.quietlog(_T("'s' signal received, set Event exitEvent"));
				isDone = true;
				break;
			}
		}
	}
	else {
		//завершение по таймеру
		HANDLE h = CreateAndStartWaitableTimer(ttl);
		WaitForSingleObject(h, INFINITE);
		log.quietlog(_T("Timer signal received, set Event exitEvent"));
		isDone = true;
		CloseHandle(h);
	}
	log.loudlog(_T("TimeManager finishing work"));
	return 0;
}