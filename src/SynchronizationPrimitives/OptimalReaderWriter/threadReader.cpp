#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"

DWORD WINAPI ThreadReaderHandler(LPVOID prm) {
	int myid = (int)prm;

	Logger log(_T("OptimalReaderWriter.ThreadReader"), myid);
	extern bool isDone;
	extern struct Configuration config;
	extern SRWLOCK lock;
	extern CONDITION_VARIABLE condread;
	extern LPVOID lpFileMapForReaders;

	while (isDone != true) {
		// Захват объекта синхронизации (совместный доступ!)
		log.quietlog(_T("Waining for Slim Reader/Writer (SRW) Lock"));
		AcquireSRWLockShared(&lock);
		SleepConditionVariableSRW(&condread, &lock, INFINITE, CONDITION_VARIABLE_LOCKMODE_SHARED);
		log.quietlog(_T("Get SRW Lock"));

		//читаем сообщение
		log.loudlog(_T("Reader %d read msg \"%s\""), myid,
			(_TCHAR *)lpFileMapForReaders);

		//освобождение объекта синхронизации
		log.quietlog(_T("Release SRW Lock"));
		ReleaseSRWLockShared(&lock);

		//задержка
		Sleep(config.readersDelay);
	}
	log.loudlog(_T("Reader %d finishing work"), myid);
	return 0;
}
