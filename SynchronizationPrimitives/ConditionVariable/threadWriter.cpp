#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	int myid = (int)prm;

	Logger log(_T("ConditionVariable.ThreadWriter"), myid);
	extern bool isDone;
	extern struct FIFOQueue queue;
	extern struct Configuration config;
	extern CRITICAL_SECTION crs;
	extern CONDITION_VARIABLE condread;
	extern CONDITION_VARIABLE condwrite;

	_TCHAR tmp[50];
	int msgnum = 0; //номер передаваемого сообщения
	while (isDone != true) {
		//Захват синхронизирующего объекта
		log.quietlog(_T("Waining for Critical Section"));
		EnterCriticalSection(&crs);
		log.quietlog(_T("Get Critical Section"));

		log.quietlog(_T("Waining for empty space in the queue"));
		while (!(queue.readindex != queue.writeindex || !queue.full == 1))
			//спим пока в очереди не освободится место
			SleepConditionVariableCS(&condwrite, &crs, INFINITE);
		log.quietlog(_T("Get space in the queue"));

		//заносим в очередь данные
		swprintf_s(tmp, _T("writer_id = %d numMsg= %3d"), myid, msgnum);
		queue.data[queue.writeindex] = _wcsdup(tmp);
		msgnum++;

		//печатаем принятые данные
		log.loudlog(_T("Writer %d put data: \"%s\" in position %d"), myid,
			queue.data[queue.writeindex], queue.writeindex);
		queue.writeindex = (queue.writeindex + 1) % queue.size;
		//если очередь заполнилась
		queue.full = queue.writeindex == queue.readindex ? 1 : 0;

		if (queue.full == 1)
			log.loudlog(_T("Queue is full"));
		//шлем сигнал потокам-читателям
		log.quietlog(_T("Wake Condition Variable"));
		WakeConditionVariable(&condread);
		// освобождение синхронизируемого объекта
		log.quietlog(_T("Leave Critical Section"));
		LeaveCriticalSection(&crs);

		//задержка
		Sleep(config.writersDelay);
	}
	log.loudlog(_T("Writer %d finishing work"), myid);
	return 0;
}
