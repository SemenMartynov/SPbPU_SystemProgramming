#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadReaderHandler(LPVOID prm) {
	extern bool isDone;
	extern struct FIFOQueue queue;
	extern struct Configuration config;
	extern HANDLE sem;

	int myid = (int)prm;
	while (isDone != true) {
		//Захват объекта синхронизации
		WaitForSingleObject(sem, INFINITE);

		//если в очереди есть данные
		if (queue.readindex != queue.writeindex || queue.full == 1) {
			//взяли данные, значит очередь не пуста
			queue.full = 0;
			//печатаем принятые данные
			printf("Reader %d get data: \"%s\" from position %d\n", myid,
				queue.data[queue.readindex], queue.readindex);
			free(queue.data[queue.readindex]); //очищаем очередь от данных
			queue.data[queue.readindex] = NULL;
			queue.readindex = (queue.readindex + 1) % queue.size;
		}
		//Освобождение объекта синхронизации
		ReleaseSemaphore(sem, 1, NULL);

		//задержка
		Sleep(config.readersDelay);
	}
	printf("Reader %d finishing work\n", myid);
	return 0;
}
