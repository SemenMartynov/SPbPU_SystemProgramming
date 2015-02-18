#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	int myid = (int)prm;

	Logger log(_T("ThreadWriter"), myid);
	extern bool isDone;
	extern struct FIFOQueue queue;
	extern struct Configuration config;

	_TCHAR tmp[50];
	int msgnum = 0; //номер передаваемого сообщени€
	while (isDone != true) {
		//«ахват синхронизирующего объекта
		log.quietlog(_T("Waining for mutex"));
		// здесь размещаем код применени€ выбранного средства
		// . . .
		log.quietlog(_T("Get mutex"));

		//если в очереди есть место
		if (queue.readindex != queue.writeindex || !queue.full == 1) {
			//заносим в очередь данные
			swprintf_s(tmp, _T("writer_id = %d numMsg= %3d"), myid, msgnum);
			queue.data[queue.writeindex] = _wcsdup(tmp);
			msgnum++;

			//печатаем прин€тые данные
			log.loudlog(_T("Writer %d put data: \"%s\" in position %d"), myid,
				queue.data[queue.writeindex], queue.writeindex);
			queue.writeindex = (queue.writeindex + 1) % queue.size;
			//если очередь заполнилась
			queue.full = queue.writeindex == queue.readindex ? 1 : 0;
		}
		//освобождение объекта синхронизации
		log.quietlog(_T("Release mutex"));
		//здесь размещаем код применени€ выбранного средства
		// . . .

		//задержка
		Sleep(config.writersDelay);
	}
	log.loudlog(_T("Writer %d finishing work"), myid);
	return 0;
}
