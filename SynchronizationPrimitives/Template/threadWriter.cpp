#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	extern bool isDone;
	extern struct FIFOQueue queue;
	extern struct Configuration config;

	int myid = (int)prm;
	char tmp[50];
	int msgnum = 0; //номер передаваемого сообщени€
	while (isDone != true) {
		//«ахват синхронизирующего объекта
		// здесь размещаем код применени€ выбранного средства
		// . . .

		//если в очереди есть место
		if (queue.readindex != queue.writeindex || !queue.full == 1) {
			//заносим в очередь данные
			sprintf_s(tmp, "writer_id = %d numMsg= %3d", myid, msgnum);
			queue.data[queue.writeindex] = _strdup(tmp);
			msgnum++;

			//печатаем прин€тые данные
			printf("Writer %d put data: \"%s\" in position %d\n", myid,
				queue.data[queue.writeindex], queue.writeindex);
			queue.writeindex = (queue.writeindex + 1) % queue.size;
			//если очередь заполнилась
			queue.full = queue.writeindex == queue.readindex ? 1 : 0;
		}
		//освобождение объекта синхронизации
		//здесь размещаем код применени€ выбранного средства
		// . . .

		//задержка
		Sleep(config.writersDelay);
	}
	printf("Writer %d finishing work\n", myid);
	return 0;
}
