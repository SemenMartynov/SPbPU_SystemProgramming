#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"

DWORD WINAPI ThreadReaderHandler(LPVOID prm) {
	int myid = (int)prm;

	Logger log(_T("ThreadsReaderWriter.ThreadReader"), myid);
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

	HANDLE readerhandlers[2];
	readerhandlers[0] = exitEvent;
	readerhandlers[1] = canReadEvent;

	while (isDone != true) {
		//ждем, пока все прочитают
		log.quietlog(_T("Waining for allReadEvent"));
		WaitForSingleObject(allReadEvent, INFINITE);
		//узнаем, сколько потоков-читателей прошло данную границу
		log.quietlog(_T("Waining for changeCountEvent"));
		WaitForSingleObject(changeCountEvent, INFINITE);
		countready++;
		//если все прошли, то "закрываем за собой дверь" и разрешаем писать
		if (countready == config.numOfReaders) {
			countready = 0;
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
			return 0;
		case WAIT_OBJECT_0 + 1: // сработало событие на возможность чтения
			log.quietlog(_T("Get canReadEvent"));
			//читаем сообщение
			log.loudlog(_T("Reader %d read msg \"%s\""), myid,
				(_TCHAR *)lpFileMapForReaders);

			//необходимо уменьшить счетчик количества читателей, которые прочитать еще не успели
			log.quietlog(_T("Waining for changeCountEvent"));
			WaitForSingleObject(changeCountEvent, INFINITE);
			countread--;

			// если мы последние читали, то запрещаем читать и открываем границу
			if (countread == 0) {
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
			ExitProcess(1001);
		}
	}
	log.loudlog(_T("Reader %d finishing work"), myid);
	return 0;
}
