#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"

DWORD WINAPI ThreadReaderHandler(LPVOID prm) {
	int myid = (int)prm;

	Logger log(_T("ThreadReader"), myid);
	extern bool isDone;
	extern struct FIFOQueue queue;
	extern struct Configuration config;
	extern HANDLE mutex;

	while (isDone != true) {
		//������ ������� �������������
		log.quietlog(_T("Waining for mutex"));
		//����� ����������� ��� ���������� ���������� ��������
		//. . .
		log.quietlog(_T("Get mutex"));

		//���� � ������� ���� ������
		if (queue.readindex != queue.writeindex || queue.full == 1) {
				//����� ������, ������ ������� �� �����
				queue.full = 0;
				//�������� �������� ������
				log.loudlog(_T("Reader %d get data: \"%s\" from position %d\n"), myid,
					queue.data[queue.readindex], queue.readindex);
				free(queue.data[queue.readindex]); //������� ������� �� ������ 
				queue.data[queue.readindex] = NULL;
				queue.readindex = (queue.readindex + 1) % queue.size;
		}
		//������������ ������� �������������
		log.quietlog(_T("Release mutex"));
		//����� ��������� ��� ���������� ���������� ��������
		//. . .

		//��������
		Sleep(config.readersDelay);
	}
	log.loudlog(_T("Reader %d finishing work"), myid);
	return 0;
}
