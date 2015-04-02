#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"

DWORD WINAPI ThreadReaderHandler(LPVOID prm) {
	int myid = (int)prm;

	Logger log(_T("ThreadReader"), myid);
	extern bool isDone;
	extern struct FIFOQueue queue;
	extern struct Configuration config;
	extern HANDLE mutex;

	while (isDone != true) {
		//Захват объекта синхронизации
		log.quietlog(_T("Waining for mutex"));
		//здесь размещается код применения выбранного средства
		//. . .
		log.quietlog(_T("Get mutex"));

		//если в очереди есть данные
		if (queue.readindex != queue.writeindex || queue.full == 1) {
				//взяли данные, значит очередь не пуста
				queue.full = 0;
				//печатаем принятые данные
				log.loudlog(_T("Reader %d get data: \"%s\" from position %d\n"), myid,
					queue.data[queue.readindex], queue.readindex);
				free(queue.data[queue.readindex]); //очищаем очередь от данных 
				queue.data[queue.readindex] = NULL;
				queue.readindex = (queue.readindex + 1) % queue.size;
		}
		//Освобождение объекта синхронизации
		log.quietlog(_T("Release mutex"));
		//здесь размещаем код применения выбранного средства
		//. . .

		//задержка
		Sleep(config.readersDelay);
	}
	log.loudlog(_T("Reader %d finishing work"), myid);
	return 0;
}
