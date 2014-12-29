#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadReaderHandler(LPVOID prm) {
	extern bool isDone;
	extern struct Configuration config;

	extern HANDLE canReadEvent;
	extern HANDLE canWriteEvent;
	extern HANDLE allReadEvent;
	extern HANDLE changeCountEvent;
	extern HANDLE exitEvent;

	extern int countread;
	extern int countready;
	extern LPVOID lpFileMapForReaders;

	int myid = (int)prm;
	HANDLE readerhandlers[2];
	readerhandlers[0] = exitEvent;
	readerhandlers[1] = canReadEvent;

	while (isDone != true) {
		//ждем, пока все прочитают
		WaitForSingleObject(allReadEvent, INFINITE);
		//узнаем, сколько потоков-читателей прошло данную границу
		WaitForSingleObject(changeCountEvent, INFINITE);
		countready++;
		//если все прошли, то "закрываем за собой дверь" и разрешаем писать
		if (countready == config.numOfReaders) {
			countready = 0;
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
			return 0;
		case WAIT_OBJECT_0 + 1: // сработало событие на возможность чтения
			//читаем сообщение
			printf("Reader %d read msg \"%s\"\n", myid,
				(char *)lpFileMapForReaders);

			//необходимо уменьшить счетчик количества читателей, которые прочитать еще не успели
			WaitForSingleObject(changeCountEvent, INFINITE);
			countread--;

			// если мы последние читали, то запрещаем читать и открываем границу
			if (countread == 0) {
				ResetEvent(canReadEvent);
				SetEvent(allReadEvent);
			}

			//разрешаем изменять счетчик
			SetEvent(changeCountEvent);
			break;
		default:
			printf("error with func WaitForMultipleObjects in readerHandle\n");
			printf("getlasterror= %d\n", GetLastError());
			ExitProcess(1001);
		}
	}
	printf("Reader %d finishing work\n", myid);
	return 0;
}
