#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <conio.h>

#include "Logger.h"

int _tmain(int argc, _TCHAR* argv[]) {
	//проверяем число аргументов
	if (argc != 3) {
		Logger log(_T("ProcessReader"));
		log.loudlog(_T("Error with start reader process. Need 2 arguments, but %d presented."), argc);
		_getch();
		ExitProcess(1000);
	}
	//получаем из командной строки наш номер
	int myid = _wtoi(argv[1]);
	int pause = _wtoi(argv[2]);

	Logger log(_T("ProcessReader"), myid);
	log.loudlog(_T("Reader with id= %d is started"), myid);

	// Состояние готовности:
	// true - ждём сообщение для чтения
	// false - текущее сообщение уже прочитано,
	//         ждём сигнала перехода в режим готовности
	bool readyState = false;

	//Инициализируем средства синхронизации
	// (атрибуты защиты, наследование описателя, имя):
	//писатель записал сообщение (ручной сброс);
	HANDLE readerCanReadEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_readerCanReadEvent$$");
	//все читатели готовы к приему следующего (автосброс);
	HANDLE readerGetReadyEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_readerGetReadyEvent$$");
	//разрешение работы со счетчиком (автосброс);
	HANDLE canChangeCountEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_canChangeCountEvent$$");
	//
	HANDLE changeCountEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_changeCountEvent$$");
	//завершение программы (ручной сброс);
	HANDLE exitEvent = OpenEvent(EVENT_ALL_ACCESS, false, L"$$My_exitEvent$$");

	//Общий ресурс (атрибуты защиты, наследование описателя, имя):
	HANDLE hFileMapping = OpenFileMapping(FILE_MAP_READ, false,
		L"$$MyVerySpecialShareFileName$$");

	//если объекты не созданы, то не сможем работать
	if (readerCanReadEvent == NULL || readerGetReadyEvent == NULL || canChangeCountEvent == NULL
		|| changeCountEvent == NULL || exitEvent == NULL
		|| hFileMapping == NULL) {
		log.loudlog(_T("Impossible to open objects, run server first\n getlasterror=%d"),
			GetLastError());
		_getch();
		return 1001;
	}

	//отображаем файл на адресное пространство нашего процесса для потоков-читателей
	LPVOID lpFileMapForReaders = MapViewOfFile(hFileMapping,
		FILE_MAP_READ, 0, 0, 0);
	//  hFileMapping - дескриптор объекта-отображения файла
	//  FILE_MAP_ALL_ACCESS - доступа к файлу
	//  0, 0 - старшая и младшая части смещения начала отображаемого участка в файле
	//         (0 - начало отображаемого участка совпадает с началом файла)
	//  0 - размер отображаемого участка файла в байтах (0 - весь файл)

	// События чтиения
	HANDLE readerHandlers[2];
	readerHandlers[0] = exitEvent;
	readerHandlers[1] = readerCanReadEvent;

	// События готовности
	HANDLE readyHandlers[2];
	readyHandlers[0] = exitEvent;
	readyHandlers[1] = readerGetReadyEvent;

	while (1) { //основной цикл
		// Ожидаем набор событий в зависимости от состояния
		if (readyState) {
			log.quietlog(_T("Waining for multiple objects"));
			DWORD dwEvent = WaitForMultipleObjects(2, readerHandlers, false,
				INFINITE);
			//   2 - следим за 2-я параметрами
			//   readerHandlers - из массива readerHandlers
			//   false - ждём, когда освободится хотя бы один
			//   INFINITE - ждать бесконечно
			switch (dwEvent) {
			case WAIT_OBJECT_0: //сработало событие exit
				log.quietlog(_T("Get exitEvent"));
				log.loudlog(_T("Reader %d finishing work"), myid);
				goto exit;

			case WAIT_OBJECT_0 + 1: // сработало событие на возможность чтения
				log.quietlog(_T("Get readerCanReadEvent"));
				//читаем сообщение
				log.loudlog(_T("Reader %d read msg \"%s\""), myid, (_TCHAR *)lpFileMapForReaders);

				// Отправляем отчёт
				log.quietlog(_T("Waining for canChangeCountEvent"));
				WaitForSingleObject(canChangeCountEvent, INFINITE);
				log.quietlog(_T("Set Event changeCountEvent"));
				SetEvent(changeCountEvent);

				// Завершаем работу
				readyState = false;
				break;
			default:
				log.loudlog(_T("Error with func WaitForMultipleObjects in readerHandle, GLE = %d"), GetLastError());
				getchar();
				ExitProcess(1001);
				break;
			}
		}
		else {
			log.quietlog(_T("Waining for multiple objects"));
			DWORD dwEvent = WaitForMultipleObjects(2, readyHandlers, false,
				INFINITE);
			//   2 - следим за 2-я параметрами
			//   readyHandlers - из массива readyHandlers
			//   false - ждём, когда освободится хотя бы один
			//   INFINITE - ждать бесконечно
			switch (dwEvent) {
			case WAIT_OBJECT_0: //сработало событие exit
				log.quietlog(_T("Get exitEvent"));
				log.loudlog(_T("Reader %d finishing work"), myid);
				goto exit;

			case WAIT_OBJECT_0 + 1: // сработало событие перехода в режим готовности
				log.quietlog(_T("Get readerGetReadyEvent"));
				// Отправляем отчёт
				log.quietlog(_T("Waining for canChangeCountEvent"));
				WaitForSingleObject(canChangeCountEvent, INFINITE);
				log.quietlog(_T("Set Event changeCountEvent"));
				SetEvent(changeCountEvent);

				// Завершаем работу
				readyState = true;
				break;
			default:
				log.loudlog(_T("Error with func WaitForMultipleObjects in readerHandle, GLE = %d"), GetLastError());
				getchar();
				ExitProcess(1001);
				break;
			}
		}
		Sleep(pause);
	}
exit:
	//закрываем HANDLE объектов синхронизации
	CloseHandle(readerCanReadEvent);
	CloseHandle(readerGetReadyEvent);
	CloseHandle(canChangeCountEvent);
	CloseHandle(changeCountEvent);
	CloseHandle(exitEvent);

	UnmapViewOfFile(lpFileMapForReaders); //закрываем общий ресурс
	CloseHandle(hFileMapping); //закрываем объект "отображаемый файл"

	log.loudlog(_T("All is done"));
	_getch();
	return 0;
}
