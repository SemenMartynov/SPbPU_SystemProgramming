#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadReaderHandler(LPVOID prm) {
	extern bool isDone;
	extern struct FIFOQueue queue;
	extern struct Configuration config;
	extern HANDLE event;

	int myid = (int)prm;
	while (isDone != true) {
		//������ ������� �������������
		WaitForSingleObject(event, INFINITE);

		//���� � ������� ���� ������
		if (queue.readindex != queue.writeindex || queue.full == 1) {
			//����� ������, ������ ������� �� �����
			queue.full = 0;
			//�������� �������� ������
			printf("Reader %d get data: \"%s\" from position %d\n", myid,
				queue.data[queue.readindex], queue.readindex);
			free(queue.data[queue.readindex]); //������� ������� �� ������
			queue.data[queue.readindex] = NULL;
			queue.readindex = (queue.readindex + 1) % queue.size;
		}
		//������������ ������� �������������
		SetEvent(event);

		//��������
		Sleep(config.readersDelay);
	}
	printf("Reader %d finishing work\n", myid);
	return 0;
}
