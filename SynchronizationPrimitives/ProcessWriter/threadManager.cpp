#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadTimeManagerHandler(LPVOID prm) {
	extern bool isDone;
	extern HANDLE exitEvent;

	int ttl = (int)prm;
	if (ttl < 0) {
		//���������� �� ������� ���������
		char buf[100];
		while (1) {
			fgets(buf, sizeof(buf), stdin);
			if (buf[0] == 's') {
				isDone = true;
				SetEvent(exitEvent);
				break;
			}
		}
	}
	else {
		//���������� �� �������
		HANDLE h = CreateAndStartWaitableTimer(ttl);
		WaitForSingleObject(h, INFINITE);
		isDone = true;
		SetEvent(exitEvent);
		CloseHandle(h);
	}
	printf("TimeManager finishing work\n");
	return 0;
}
