#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <conio.h>

#include "Logger.h"

int _tmain(int argc, _TCHAR* argv[]) {
	//проверяем число аргументов
	if (argc != 2) {
		Logger log(_T("ProcessReader"));
		log.loudlog(_T("Error with start reader process. Need 2 arguments."));
		_getch();
		ExitProcess(1000);
	}
	//получаем из командной строки наш номер
	int myid = _wtoi(argv[1]);

	Logger log(_T("ProcessReader"), myid);
	log.loudlog(_T("Reader with id= %d is started"), myid);

	//Инициализируем средства синхронизации
	// (атрибуты защиты, наследование описателя, имя):
	//писатель записал сообщение (ручной сброс);
	HANDLE canReadEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_canReadEvent$$");
	//все читатели готовы к приему следующего (автосброс);
	HANDLE canWriteEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_canWriteEvent$$");
	//все читатели прочитали сообщение (ручной сброс);
	HANDLE allReadEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_allReadEvent$$");
	//разрешение работы со счетчиком (автосброс);
	HANDLE changeCountEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_changeCountEvent$$");
	//завершение программы (ручной сброс);
	HANDLE exitEvent = OpenEvent(EVENT_ALL_ACCESS, false, L"$$My_exitEvent$$");

	//Общий ресурс (атрибуты защиты, наследование описателя, имя):
	HANDLE hFileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, false,
		L"$$MyVerySpecialShareFileName$$");

	//если объекты не созданы, то не сможем работать
	if (canReadEvent == NULL || canWriteEvent == NULL || allReadEvent == NULL
		|| changeCountEvent == NULL || exitEvent == NULL
		|| hFileMapping == NULL) {
		log.loudlog(_T("Impossible to open objects, run server first\n getlasterror=%d"),
			GetLastError());
		_getch();
		return 1001;
	}

	//отображаем файл на адресное пространство нашего процесса для потоков-читателей
	LPVOID lpFileMapForReaders = MapViewOfFile(hFileMapping,
		FILE_MAP_ALL_ACCESS, 0, 0, 0);
	//  hFileMapping - дескриптор объекта-отображения файла
	//  FILE_MAP_ALL_ACCESS - доступа к файлу
	//  0, 0 - старшая и младшая части смещения начала отображаемого участка в файле
	//         (0 - начало отображаемого участка совпадает с началом файла)
	//  0 - размер отображаемого участка файла в байтах (0 - весь файл)

	HANDLE readerhandlers[2];
	readerhandlers[0] = exitEvent;
	readerhandlers[1] = canReadEvent;

	while (1) { //основной цикл
		//ждем, пока все прочитают
		log.quietlog(_T("Waining for allReadEvent"));
		WaitForSingleObject(allReadEvent, INFINITE);
		//узнаем, сколько потоков-читателей прошло данную границу
		log.quietlog(_T("Waining for changeCountEvent"));
		WaitForSingleObject(changeCountEvent, INFINITE);
		(*(((int *)lpFileMapForReaders) + 1))--;
		log.loudlog(_T("Readready= %d\n"), (*(((int *)lpFileMapForReaders) + 1)));
		//если все прошли, то "закрываем за собой дверь" и разрешаем писать
		if ((*(((int *)lpFileMapForReaders) + 1)) == 0) {
			log.quietlog(_T("Reset Event allReadEvent"));
			ResetEvent(allReadEvent);
			log.quietlog(_T("Set Event canWriteEvent"));
			SetEvent(canWriteEvent);
		}

		//разрешаем изменять счетчик
		log.quietlog(_T("Set Event changeCountEvent"));
		SetEvent(changeCountEvent);

		log.quietlog(_T("Waining for multiple objects"));
		DWORD dwEvent = WaitForMultipleObjects(2, readerhandlers, false,
			INFINITE);
		//   2 - следим за 2-я параметрами
		//   readerhandlers - из массива readerhandlers
		//   false - ждём, когда освободится хотя бы один
		//   INFINITE - ждать бесконечно
		switch (dwEvent) {
		case WAIT_OBJECT_0: //сработало событие exit
			log.quietlog(_T("Get exitEvent"));
			log.loudlog(_T("Reader %d finishing work"), myid);
			goto exit;
		case WAIT_OBJECT_0 + 1: // сработало событие на возможность чтения
			log.quietlog(_T("Get canReadEvent"));
			//читаем сообщение
			log.loudlog(_T("Reader %d read msg \"%s\""), myid,
				((_TCHAR *)lpFileMapForReaders) + sizeof(int) * 2);

			//необходимо уменьшить счетчик количества читателей, которые прочитать еще не успели
			log.quietlog(_T("Waining for changeCountEvent"));
			WaitForSingleObject(changeCountEvent, INFINITE);
			(*((int *)lpFileMapForReaders))--;
			log.loudlog(_T("Readcount= %d"), (*(((int *)lpFileMapForReaders))));

			// если мы последние читали, то запрещаем читать и открываем границу
			if ((*((int *)lpFileMapForReaders)) == 0) {
				log.quietlog(_T("Reset Event canReadEvent"));
				ResetEvent(canReadEvent);
				log.quietlog(_T("Set Event allReadEvent"));
				SetEvent(allReadEvent);
			}

			//разрешаем изменять счетчик
			log.quietlog(_T("Set Event changeCountEvent"));
			SetEvent(changeCountEvent);
			break;
		default:
			log.loudlog(_T("Error with func WaitForMultipleObjects in readerHandle, GLE = %d"), GetLastError());
			getchar();
			ExitProcess(1001);
			break;
		}
	}
exit:
	//закрываем HANDLE объектов синхронизации
	CloseHandle(canReadEvent);
	CloseHandle(canWriteEvent);
	CloseHandle(allReadEvent);
	CloseHandle(changeCountEvent);
	CloseHandle(exitEvent);

	UnmapViewOfFile(lpFileMapForReaders); //закрываем общий ресурс
	CloseHandle(hFileMapping); //закрываем объект "отображаемый файл"

	log.loudlog(_T("All tasks are done!"));
	_getch();
	return 0;
}
