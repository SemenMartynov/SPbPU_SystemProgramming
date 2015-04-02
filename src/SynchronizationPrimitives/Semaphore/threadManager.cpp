#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"

DWORD WINAPI ThreadTimeManagerHandler(LPVOID prm) {
	Logger log(_T("Semaphore.ThreadTimeManager"));

	extern bool isDone;

	int ttl = (int)prm;
	if (ttl < 0) {
		//завершение по команде оператора
		_TCHAR buf[100];
		while (1) {
			fgetws(buf, sizeof(buf), stdin);
			if (buf[0] == _T('s')) {
				isDone = true;
				log.quietlog(_T("'s' signal received!"));
				break;
			}
		}
	}
	else {
		//завершение по таймеру
		HANDLE h = CreateAndStartWaitableTimer(ttl);
		WaitForSingleObject(h, INFINITE);
		log.quietlog(_T("Timer signal received!"));
		isDone = true;
		CloseHandle(h);
	}
	log.loudlog(_T("TimeManager finishing work"));
	return 0;
}