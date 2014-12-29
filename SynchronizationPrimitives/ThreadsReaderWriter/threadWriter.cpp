#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	extern bool isDone;
	extern struct Configuration config;

	extern HANDLE canReadEvent;
	extern HANDLE canWriteEvent;
	extern HANDLE exitEvent;

	extern int countread;
	extern LPVOID lpFileMapForWriters;

	int myid = (int)prm;
	int msgnum = 0;
	HANDLE writerhandlers[2];
	writerhandlers[0] = exitEvent;
	writerhandlers[1] = canWriteEvent;

	while (isDone != true) {
		DWORD dwEvent = WaitForMultipleObjects(2, writerhandlers, false,
			INFINITE);
		//   2 - следим за 2-я параметрами
		//   writerhandlers - из массива writerhandlers
		//   false - ждём, когда освободится хотя бы один
		//   INFINITE - ждать бесконечно
		switch (dwEvent) {
		case WAIT_OBJECT_0:	//сработало событие exit
			printf("Writer %d finishing work\n", myid);
			return 0;
		case WAIT_OBJECT_0 + 1: // сработало событие на возможность записи
			//увеличиваем номер сообщения
			msgnum++;
			//число потоков которые должны прочитать сообщение
			countread = config.numOfReaders;
			// Запись сообщения
			sprintf_s((char *)lpFileMapForWriters, 1500,
				"writer_id %d, msg with num = %d", myid, msgnum);
			printf("writer put msg: \"%s\" \n", lpFileMapForWriters);
			//разрешаем читателям прочитать сообщение и опять ставим событие в занятое
			SetEvent(canReadEvent);
			break;
		default:
			printf("error with func WaitForMultipleObjects in writerHandle\n");
			printf("getlasterror= %d\n", GetLastError());
			ExitProcess(1000);
		}
	}
	printf("Writer %d finishing work\n", myid);
	return 0;
}
