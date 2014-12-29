#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	extern bool isDone;
	extern struct FIFOQueue queue;
	extern struct Configuration config;
	extern HANDLE event;

	int myid = (int)prm;
	char tmp[50];
	int msgnum = 0; //����� ������������� ���������
	while (isDone != true) {
		//������ ����������������� �������
		WaitForSingleObject(event, INFINITE);

		//���� � ������� ���� �����
		if (queue.readindex != queue.writeindex || !queue.full == 1) {
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
		}
		//������������ ������� �������������
		SetEvent(event);

		//��������
		Sleep(config.writersDelay);
	}
	printf("Writer %d finishing work\n", myid);
	return 0;
}
