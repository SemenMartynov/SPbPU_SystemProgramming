#include<windows.h>
#include<stdio.h>
#include <conio.h>

int main(int argc, char* argv[]) {
	//проверяем число аргументов
	if (argc != 3) {
		printf("error with start reader process. Need 2 arguments, but %d presented.\n", argc);
		_getch();
		ExitProcess(1000);
	}
	//получаем из командной строки наш номер
	int myid = atoi(argv[1]);
	int pause = atoi(argv[2]);
	printf("reader with id= %d is started\n", myid);

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
		printf("impossible to open objects, run server first\n getlasterror=%d",
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
			DWORD dwEvent = WaitForMultipleObjects(2, readerHandlers, false,
				INFINITE);
			//   2 - следим за 2-я параметрами
			//   readerHandlers - из массива readerHandlers
			//   false - ждём, когда освободится хотя бы один
			//   INFINITE - ждать бесконечно
			switch (dwEvent) {
			case WAIT_OBJECT_0: //сработало событие exit
				printf("Reader %d finishing work\n", myid);
				goto exit;

			case WAIT_OBJECT_0 + 1: // сработало событие на возможность чтения
				//читаем сообщение
				printf("Reader %d read msg \"%s\"\n", myid, (char *)lpFileMapForReaders);

				// Отправляем отчёт
				WaitForSingleObject(canChangeCountEvent, INFINITE);
				SetEvent(changeCountEvent);

				// Завершаем работу
				readyState = false;
				break;
			default:
				printf("error with func WaitForMultipleObjects in readerHandle\n");
				printf("getlasterror= %d\n", GetLastError());
				getchar();
				ExitProcess(1001);
				break;
			}
		}
		else {
			DWORD dwEvent = WaitForMultipleObjects(2, readyHandlers, false,
				INFINITE);
			//   2 - следим за 2-я параметрами
			//   readyHandlers - из массива readyHandlers
			//   false - ждём, когда освободится хотя бы один
			//   INFINITE - ждать бесконечно
			switch (dwEvent) {
			case WAIT_OBJECT_0: //сработало событие exit
				printf("Reader %d finishing work\n", myid);
				goto exit;

			case WAIT_OBJECT_0 + 1: // сработало событие перехода в режим готовности
				// Отправляем отчёт
				WaitForSingleObject(canChangeCountEvent, INFINITE);
				SetEvent(changeCountEvent);

				// Завершаем работу
				readyState = true;
				break;
			default:
				printf("error with func WaitForMultipleObjects in readerHandle\n");
				printf("getlasterror= %d\n", GetLastError());
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

	printf("all is done\n");
	_getch();
	return 0;
}
