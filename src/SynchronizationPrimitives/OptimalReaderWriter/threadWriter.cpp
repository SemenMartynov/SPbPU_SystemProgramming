#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	int myid = (int)prm;

	Logger log(_T("OptimalReaderWriter.ThreadWriter"), myid);
	extern bool isDone;
	extern struct Configuration config;
	extern SRWLOCK lock;
	extern CONDITION_VARIABLE condread;
	extern LPVOID lpFileMapForWriters;

	int msgnum = 0;
	while (isDone != true) {
		// Захват объекта синхронизации (монопольный доступ!)
		log.quietlog(_T("Waining for Slim Reader/Writer (SRW) Lock"));
		AcquireSRWLockExclusive(&lock);
		log.quietlog(_T("Get SRW Lock"));

		// Запись сообщения
		swprintf_s((_TCHAR *)lpFileMapForWriters, 1500,
			_T("writer_id %d, msg with num = %d"), myid, msgnum++);
		log.loudlog(_T("writer put msg: \"%s\""), lpFileMapForWriters);

		//освобождение объекта синхронизации
		log.quietlog(_T("Release SRW Lock"));
		WakeAllConditionVariable(&condread);
		ReleaseSRWLockExclusive(&lock);

		//задержка
		Sleep(config.writersDelay);
	}
	log.loudlog(_T("Writer %d finishing work"), myid);
	return 0;
}
