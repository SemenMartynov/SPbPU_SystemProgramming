#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadReaderHandler(LPVOID prm) {
	int myid = (int)prm;

	Logger log(_T("ConditionVariable.ThreadReader"), myid);
	extern bool isDone;
	extern struct FIFOQueue queue;
	extern struct Configuration config;
	extern CRITICAL_SECTION crs;
	extern CONDITION_VARIABLE condread;
	extern CONDITION_VARIABLE condwrite;

	while (isDone != true) {
		//������ ������� �������������
		log.quietlog(_T("Waining for Critical Section"));
		EnterCriticalSection(&crs);
		log.quietlog(_T("Get Critical Section"));

		log.quietlog(_T("Waining for empty space in the queue"));
		while (!(queue.readindex != queue.writeindex || queue.full == 1))
			//���� ���� � ������� �� �������� ������
			SleepConditionVariableCS(&condread, &crs, INFINITE);
		log.quietlog(_T("Get space in the queue"));

		//����� ������, ������ ������� �� �����
		queue.full = 0;
		//�������� �������� ������
		log.loudlog(_T("Reader %d get data: \"%s\" from position %d"), myid,
			queue.data[queue.readindex], queue.readindex);
		free(queue.data[queue.readindex]); //������� ������� �� ������
		queue.data[queue.readindex] = NULL;
		queue.readindex = (queue.readindex + 1) % queue.size;

		//���� ������ �������-���������
		log.quietlog(_T("Wake Condition Variable"));
		WakeConditionVariable(&condwrite);
		// ������������ ����������������� �������
		log.quietlog(_T("Leave Critical Section"));
		LeaveCriticalSection(&crs);

		//��������
		Sleep(config.readersDelay);
	}
	log.loudlog(_T("Reader %d finishing work"), myid);
	return 0;
}
