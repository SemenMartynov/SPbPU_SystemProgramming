#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	extern bool isDone;
	extern struct Configuration config;

	extern HANDLE canReadEvent;
	extern HANDLE canWriteEvent;
	extern HANDLE changeCountEvent;
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

			// Запись сообщения
			sprintf_s(((char *)lpFileMapForWriters) + sizeof(int) * 2, 1500 - sizeof(int) * 2,
				"writer_id	%d, msg with num = %d", myid, msgnum);
			printf("writer put msg: \"%s\" \n",
				((char *)lpFileMapForWriters) + sizeof(int) * 2);

			//число потоков которые должны прочитать сообщение
			WaitForSingleObject(changeCountEvent, INFINITE);
			*((int *)lpFileMapForWriters) += config.numOfReaders;
			*(((int *)lpFileMapForWriters) + 1) += config.numOfReaders;
			SetEvent(changeCountEvent);

			//разрешаем потокам-читателям прочитать сообщение и опять ставим событие в состояние «занято»
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
