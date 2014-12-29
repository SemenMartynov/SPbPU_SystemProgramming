#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadReaderHandler(LPVOID prm) {
	extern bool isDone;
	extern struct FIFOQueue queue;
	extern struct Configuration config;
	extern CRITICAL_SECTION crs;
	extern CONDITION_VARIABLE condread;
	extern CONDITION_VARIABLE condwrite;

	int myid = (int)prm;
	while (isDone != true) {
		//������ ������� �������������
		EnterCriticalSection(&crs);

		while (!(queue.readindex != queue.writeindex || queue.full == 1))
			//���� ���� � ������� �� �������� ������
			SleepConditionVariableCS(&condread, &crs, INFINITE);

		//����� ������, ������ ������� �� �����
		queue.full = 0;
		//�������� �������� ������
		printf("Reader %d get data: \"%s\" from position %d\n", myid,
			queue.data[queue.readindex], queue.readindex);
		free(queue.data[queue.readindex]); //������� ������� �� ������
		queue.data[queue.readindex] = NULL;
		queue.readindex = (queue.readindex + 1) % queue.size;

		//���� ������ �������-���������
		WakeConditionVariable(&condwrite);
		// ������������ ����������������� �������
		LeaveCriticalSection(&crs);

		//��������
		Sleep(config.readersDelay);
	}
	printf("Reader %d finishing work\n", myid);
	return 0;
}
