#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadReaderHandler(LPVOID prm) {
	extern bool isDone;
	extern struct Configuration config;

	extern HANDLE canReadEvent;
	extern HANDLE canWriteEvent;
	extern HANDLE allReadEvent;
	extern HANDLE changeCountEvent;
	extern HANDLE exitEvent;

	extern int countread;
	extern int countready;
	extern LPVOID lpFileMapForReaders;

	int myid = (int)prm;
	HANDLE readerhandlers[2];
	readerhandlers[0] = exitEvent;
	readerhandlers[1] = canReadEvent;

	while (isDone != true) {
		//����, ���� ��� ���������
		WaitForSingleObject(allReadEvent, INFINITE);
		//������, ������� �������-��������� ������ ������ �������
		WaitForSingleObject(changeCountEvent, INFINITE);
		countready++;
		//���� ��� ������, �� "��������� �� ����� �����" � ��������� ������
		if (countready == config.numOfReaders) {
			countready = 0;
			ResetEvent(allReadEvent);
			SetEvent(canWriteEvent);
		}

		//��������� �������� �������
		SetEvent(changeCountEvent);

		DWORD dwEvent = WaitForMultipleObjects(2, readerhandlers, false,
			INFINITE);
		//   2 - ������ �� 2-� �����������
		//   readerhandlers - �� ������� readerhandlers
		//   false - ���, ����� ����������� ���� �� ����
		//   INFINITE - ����� ����������
		switch (dwEvent) {
		case WAIT_OBJECT_0: //��������� ������� exit
			printf("Reader %d finishing work\n", myid);
			return 0;
		case WAIT_OBJECT_0 + 1: // ��������� ������� �� ����������� ������
			//������ ���������
			printf("Reader %d read msg \"%s\"\n", myid,
				(char *)lpFileMapForReaders);

			//���������� ��������� ������� ���������� ���������, ������� ��������� ��� �� ������
			WaitForSingleObject(changeCountEvent, INFINITE);
			countread--;

			// ���� �� ��������� ������, �� ��������� ������ � ��������� �������
			if (countread == 0) {
				ResetEvent(canReadEvent);
				SetEvent(allReadEvent);
			}

			//��������� �������� �������
			SetEvent(changeCountEvent);
			break;
		default:
			printf("error with func WaitForMultipleObjects in readerHandle\n");
			printf("getlasterror= %d\n", GetLastError());
			ExitProcess(1001);
		}
	}
	printf("Reader %d finishing work\n", myid);
	return 0;
}
