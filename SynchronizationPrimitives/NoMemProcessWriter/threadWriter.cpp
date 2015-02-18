#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	int myid = (int)prm;

	Logger log(_T("NoMemProcessWriter.ThreadWriter"), myid);
	extern bool isDone;
	extern struct Configuration config;

	extern HANDLE readerCanReadEvent;
	extern HANDLE readerGetReadyEvent;
	extern HANDLE canChangeCountEvent;
	extern HANDLE changeCountEvent;
	extern HANDLE exitEvent;

	extern int reportCounter;  // Счётчиков отчётов
	extern LPVOID lpFileMapForWriters;

	int msgnum = 0;
	HANDLE writerHandlers[2];
	writerHandlers[0] = exitEvent;
	writerHandlers[1] = changeCountEvent;

	// Состояние готовности:
	// true - сообщение записано, ждём отчётов о прочтении
	// false - переводим всех читателей в состояние готовности
	bool readyState = false;

	while (isDone != true) {
		log.quietlog(_T("Waining for multiple objects"));
		DWORD dwEvent = WaitForMultipleObjects(2, writerHandlers, false,
			INFINITE);
		//   2 - следим за 2-я параметрами
		//   writerHandlers - из массива writerHandlers
		//   false - ждём, когда освободится хотя бы один
		//   INFINITE - ждать бесконечно
		switch (dwEvent) {
		case WAIT_OBJECT_0:	//сработало событие exit
			log.quietlog(_T("Get exitEvent"));
			log.loudlog(_T("Writer %d finishing work"), myid);
			return 0;
		case WAIT_OBJECT_0 + 1: // Пришёл отчёт о выполнении
			log.quietlog(_T("Get changeCountEvent"));
			// Если отчитались все читатели
			if (++reportCounter == config.numOfReaders) {
				// Обнуление счётчика
				reportCounter = 0;
				if (readyState) { // все всё прочитали
					// Теперь ожидаем отчётов о готовности
					readyState = false;
					// Больше ни кто не читает
					log.quietlog(_T("Reset Event readerCanReadEvent"));
					ResetEvent(readerCanReadEvent);
					// Можно готовится
					log.quietlog(_T("Set Event readerGetReadyEvent"));
					SetEvent(readerGetReadyEvent);
				}
				else { // все готовы читать
					// Запись сообщения
					swprintf_s((_TCHAR *)lpFileMapForWriters, 1500,
						_T("Writer_id	%d, msg with num = %d"), myid, ++msgnum);
					log.loudlog(_T("Writer put msg: \"%s\""), (_TCHAR *)lpFileMapForWriters);
					
					// Теперь ожидаем отчётов о прочтении
					readyState = true;
					// Больше ни кто не готовится
					log.quietlog(_T("Reset Event readerGetReadyEvent"));
					ResetEvent(readerGetReadyEvent);
					// Можно читать
					log.quietlog(_T("Set Event readerCanReadEvent"));
					SetEvent(readerCanReadEvent);
				}
			}
			// Ждём следующего отчёта
			log.quietlog(_T("Set Event canChangeCountEvent"));
			SetEvent(canChangeCountEvent);

			break;
		default:
			log.loudlog(_T("Error with func WaitForMultipleObjects in writerHandle, GLE = %d"), GetLastError());
			ExitProcess(1000);
		}
	}
	log.loudlog(_T("Writer %d finishing work"), myid);
	return 0;
}
