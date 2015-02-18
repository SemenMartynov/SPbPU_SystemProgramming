#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	int myid = (int)prm;

	Logger log(_T("ProcessWriter.ThreadWriter"), myid);
	extern bool isDone;
	extern struct Configuration config;

	extern HANDLE canReadEvent;
	extern HANDLE canWriteEvent;
	extern HANDLE changeCountEvent;
	extern HANDLE exitEvent;

	extern int countread;
	extern LPVOID lpFileMapForWriters;

	int msgnum = 0;
	HANDLE writerhandlers[2];
	writerhandlers[0] = exitEvent;
	writerhandlers[1] = canWriteEvent;

	while (isDone != true) {
		log.quietlog(_T("Waining for multiple objects"));
		DWORD dwEvent = WaitForMultipleObjects(2, writerhandlers, false,
			INFINITE);
		//   2 - следим за 2-я параметрами
		//   writerhandlers - из массива writerhandlers
		//   false - ждём, когда освободится хотя бы один
		//   INFINITE - ждать бесконечно
		switch (dwEvent) {
		case WAIT_OBJECT_0:	//сработало событие exit
			log.quietlog(_T("Get exitEvent"));
			log.loudlog(_T("Writer %d finishing work"), myid);
			return 0;
		case WAIT_OBJECT_0 + 1: // сработало событие на возможность записи
			log.quietlog(_T("Get canWriteEvent"));
			//увеличиваем номер сообщения
			msgnum++;

			// Запись сообщения
			swprintf_s((_TCHAR *)lpFileMapForWriters + sizeof(int) * 2, 1500 - sizeof(int) * 2,
				_T("Writer_id	%d, msg with num = %d"), myid, msgnum);
			log.loudlog(_T("Writer put msg: \"%s\""), (_TCHAR *)lpFileMapForWriters + sizeof(int) * 2);

			//число потоков которые должны прочитать сообщение
			log.quietlog(_T("Waining for changeCountEvent"));
			WaitForSingleObject(changeCountEvent, INFINITE);
			*((int *)lpFileMapForWriters) += config.numOfReaders;
			*(((int *)lpFileMapForWriters) + 1) += config.numOfReaders;
			log.quietlog(_T("Set Event changeCountEvent"));
			SetEvent(changeCountEvent);

			//разрешаем потокам-читателям прочитать сообщение и опять ставим событие в состояние «занято»
			log.quietlog(_T("Set Event canReadEvent"));
			SetEvent(canReadEvent);

			break;
		default:
			log.loudlog(_T("Error with func WaitForMultipleObjects in writerHandle, GLE = %d"), GetLastError());
			ExitProcess(1000);
		}
	}
	log.loudlog(_T("Writer %d finishing work"), myid);
	return 0;
}
