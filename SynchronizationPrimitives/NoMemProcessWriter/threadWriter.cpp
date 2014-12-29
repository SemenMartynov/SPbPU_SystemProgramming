#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	extern bool isDone;
	extern struct Configuration config;

	extern HANDLE readerCanReadEvent;
	extern HANDLE readerGetReadyEvent;
	extern HANDLE canChangeCountEvent;
	extern HANDLE changeCountEvent;
	extern HANDLE exitEvent;

	extern int reportCounter;  // Счётчиков отчётов
	extern LPVOID lpFileMapForWriters;

	int myid = (int)prm;
	int msgnum = 0;
	HANDLE writerHandlers[2];
	writerHandlers[0] = exitEvent;
	writerHandlers[1] = changeCountEvent;

	// Состояние готовности:
	// true - сообщение записано, ждём отчётов о прочтении
	// false - переводим всех читателей в состояние готовности
	bool readyState = false;

	while (isDone != true) {
		DWORD dwEvent = WaitForMultipleObjects(2, writerHandlers, false,
			INFINITE);
		//   2 - следим за 2-я параметрами
		//   writerHandlers - из массива writerHandlers
		//   false - ждём, когда освободится хотя бы один
		//   INFINITE - ждать бесконечно
		switch (dwEvent) {
		case WAIT_OBJECT_0:	//сработало событие exit
			printf("Writer %d finishing work\n", myid);
			return 0;
		case WAIT_OBJECT_0 + 1: // Пришёл отчёт о выполнении
			// Если отчитались все читатели
			if (++reportCounter == config.numOfReaders) {
				// Обнуление счётчика
				reportCounter = 0;
				if (readyState) { // все всё прочитали
					// Теперь ожидаем отчётов о готовности
					readyState = false;
					// Больше ни кто не читает
					ResetEvent(readerCanReadEvent);
					// Можно готовится
					SetEvent(readerGetReadyEvent);
				}
				else { // все готовы читать
					// Запись сообщения
					sprintf_s(((char *)lpFileMapForWriters), 1500,
						"writer_id	%d, msg with num = %d", myid, ++msgnum);
					printf("writer put msg: \"%s\" \n",	((char *)lpFileMapForWriters));
					
					// Теперь ожидаем отчётов о прочтении
					readyState = true;
					// Больше ни кто не готовится
					ResetEvent(readerGetReadyEvent);
					// Можно читать
					SetEvent(readerCanReadEvent);
				}
			}
			// Ждём следующего отчёта
			SetEvent(canChangeCountEvent);

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
