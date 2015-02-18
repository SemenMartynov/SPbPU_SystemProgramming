#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"

DWORD WINAPI ThreadTimeManagerHandler(LPVOID prm) {
	Logger log(_T("NoMemProcessWriter.ThreadTimeManager"));

	extern bool isDone;
	extern HANDLE exitEvent;

	int ttl = (int)prm;
	if (ttl < 0) {
		//завершение по команде оператора
		_TCHAR buf[100];
		while (1) {
			fgetws(buf, sizeof(buf), stdin);
			if (buf[0] == _T('s')) {
				log.quietlog(_T("'s' signal received, set Event exitEvent"));
				isDone = true;
				SetEvent(exitEvent);
				break;
			}
		}
	}
	else {
		//завершение по таймеру
		HANDLE h = CreateAndStartWaitableTimer(ttl);
		WaitForSingleObject(h, INFINITE);
		log.quietlog(_T("Timer signal received, set Event exitEvent"));
		isDone = true;
		SetEvent(exitEvent);
		CloseHandle(h);
	}
	log.loudlog(_T("TimeManager finishing work"));
	return 0;
}
