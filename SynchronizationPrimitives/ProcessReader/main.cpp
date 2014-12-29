#include<windows.h>
#include<stdio.h>
#include <conio.h>

int main(int argc, char* argv[]) {
	//проверяем число аргументов
	if (argc != 2) {
		printf("error with start reader process. Need 2 arguments\n");
		_getch();
		ExitProcess(1000);
	}
	//получаем из командной строки наш номер
	int myid = atoi(argv[1]);

	printf("reader with id= %d is started\n", myid);

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
		printf("impossible to open objects, run server first\n getlasterror=%d",
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
		WaitForSingleObject(allReadEvent, INFINITE);
		//узнаем, сколько потоков-читателей прошло данную границу
		WaitForSingleObject(changeCountEvent, INFINITE);
		(*(((int *)lpFileMapForReaders) + 1))--;
		printf("readready= %d\n", (*(((int *)lpFileMapForReaders) + 1)));
		//если все прошли, то "закрываем за собой дверь" и разрешаем писать
		if ((*(((int *)lpFileMapForReaders) + 1)) == 0) {
			ResetEvent(allReadEvent);
			SetEvent(canWriteEvent);
		}

		//разрешаем изменять счетчик
		SetEvent(changeCountEvent);

		DWORD dwEvent = WaitForMultipleObjects(2, readerhandlers, false,
			INFINITE);
		//   2 - следим за 2-я параметрами
		//   readerhandlers - из массива readerhandlers
		//   false - ждём, когда освободится хотя бы один
		//   INFINITE - ждать бесконечно
		switch (dwEvent) {
		case WAIT_OBJECT_0: //сработало событие exit
			printf("Reader %d finishing work\n", myid);
			goto exit;
		case WAIT_OBJECT_0 + 1: // сработало событие на возможность чтения
			//читаем сообщение
			printf("Reader %d read msg \"%s\"\n", myid,
				((char *)lpFileMapForReaders) + sizeof(int) * 2);

			//необходимо уменьшить счетчик количества читателей, которые прочитать еще не успели
			WaitForSingleObject(changeCountEvent, INFINITE);
			(*((int *)lpFileMapForReaders))--;
			printf("readcount= %d\n", (*(((int *)lpFileMapForReaders))));

			// если мы последние читали, то запрещаем читать и открываем границу
			if ((*((int *)lpFileMapForReaders)) == 0) {
				ResetEvent(canReadEvent);
				SetEvent(allReadEvent);
			}

			//разрешаем изменять счетчик
			SetEvent(changeCountEvent);
			break;
		default:
			printf("error with func WaitForMultipleObjects in readerHandle\n");
			printf("getlasterror= %d\n", GetLastError());
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

	printf("all is done\n");
	_getch();
	return 0;
}
