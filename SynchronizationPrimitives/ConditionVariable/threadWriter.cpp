#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	extern bool isDone;
	extern struct FIFOQueue queue;
	extern struct Configuration config;
	extern CRITICAL_SECTION crs;
	extern CONDITION_VARIABLE condread;
	extern CONDITION_VARIABLE condwrite;

	int myid = (int)prm;
	char tmp[50];
	int msgnum = 0; //����� ������������� ���������
	while (isDone != true) {
		//������ ����������������� �������
		EnterCriticalSection(&crs);

		while (!(queue.readindex != queue.writeindex || !queue.full == 1))
			//���� ���� � ������� �� ����������� �����
			SleepConditionVariableCS(&condwrite, &crs, INFINITE);

		//������� � ������� ������
		sprintf_s(tmp, "writer_id = %d numMsg= %3d", myid, msgnum);
		queue.data[queue.writeindex] = _strdup(tmp);
		msgnum++;

		//�������� �������� ������
		printf("Writer %d put data: \"%s\" in position %d\n", myid,
			queue.data[queue.writeindex], queue.writeindex);
		queue.writeindex = (queue.writeindex + 1) % queue.size;
		//���� ������� �����������
		queue.full = queue.writeindex == queue.readindex ? 1 : 0;

		if (queue.full == 1)
			printf("queue is full\n");
		//���� ������ �������-���������
		WakeConditionVariable(&condread);
		// ������������ ����������������� �������
		LeaveCriticalSection(&crs);

		//��������
		Sleep(config.writersDelay);
	}
	printf("Writer %d finishing work\n", myid);
	return 0;
}
