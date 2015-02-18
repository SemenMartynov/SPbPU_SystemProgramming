#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	int myid = (int)prm;

	Logger log(_T("Semaphore.ThreadWriter"), myid);
	extern bool isDone;
	extern struct FIFOQueue queue;
	extern struct Configuration config;
	extern HANDLE sem;

	_TCHAR tmp[50];
	int msgnum = 0; //����� ������������� ���������
	while (isDone != true) {
		//������ ����������������� �������
		log.quietlog(_T("Waining for Semaphore"));
		WaitForSingleObject(sem, INFINITE);
		log.quietlog(_T("Get Semaphore"));

		//���� � ������� ���� �����
		if (queue.readindex != queue.writeindex || !queue.full == 1) {
			//������� � ������� ������
			swprintf_s(tmp, _T("writer_id = %d numMsg= %3d"), myid, msgnum);
			queue.data[queue.writeindex] = _wcsdup(tmp);
			msgnum++;

			//�������� �������� ������
			log.loudlog(_T("Writer %d put data: \"%s\" in position %d"), myid,
				queue.data[queue.writeindex], queue.writeindex);
			queue.writeindex = (queue.writeindex + 1) % queue.size;
			//���� ������� �����������
			queue.full = queue.writeindex == queue.readindex ? 1 : 0;
		}
		//������������ ������� �������������
		log.quietlog(_T("Release Semaphore"));
		ReleaseSemaphore(sem, 1, NULL);

		//��������
		Sleep(config.writersDelay);
	}
	log.loudlog(_T("Writer %d finishing work"), myid);
	return 0;
}
