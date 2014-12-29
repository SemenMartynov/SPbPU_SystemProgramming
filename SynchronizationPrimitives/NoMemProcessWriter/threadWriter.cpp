#include<windows.h>
#include<stdio.h>

#include"utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	extern bool isDone;
	extern struct Configuration config;

	extern HANDLE readerCanReadEvent;
	extern HANDLE readerGetReadyEvent;
	extern HANDLE canChangeCountEvent;
	extern HANDLE changeCountEvent;
	extern HANDLE exitEvent;

	extern int reportCounter;  // ��������� �������
	extern LPVOID lpFileMapForWriters;

	int myid = (int)prm;
	int msgnum = 0;
	HANDLE writerHandlers[2];
	writerHandlers[0] = exitEvent;
	writerHandlers[1] = changeCountEvent;

	// ��������� ����������:
	// true - ��������� ��������, ��� ������� � ���������
	// false - ��������� ���� ��������� � ��������� ����������
	bool readyState = false;

	while (isDone != true) {
		DWORD dwEvent = WaitForMultipleObjects(2, writerHandlers, false,
			INFINITE);
		//   2 - ������ �� 2-� �����������
		//   writerHandlers - �� ������� writerHandlers
		//   false - ���, ����� ����������� ���� �� ����
		//   INFINITE - ����� ����������
		switch (dwEvent) {
		case WAIT_OBJECT_0:	//��������� ������� exit
			printf("Writer %d finishing work\n", myid);
			return 0;
		case WAIT_OBJECT_0 + 1: // ������ ����� � ����������
			// ���� ���������� ��� ��������
			if (++reportCounter == config.numOfReaders) {
				// ��������� ��������
				reportCounter = 0;
				if (readyState) { // ��� �� ���������
					// ������ ������� ������� � ����������
					readyState = false;
					// ������ �� ��� �� ������
					ResetEvent(readerCanReadEvent);
					// ����� ���������
					SetEvent(readerGetReadyEvent);
				}
				else { // ��� ������ ������
					// ������ ���������
					sprintf_s(((char *)lpFileMapForWriters), 1500,
						"writer_id	%d, msg with num = %d", myid, ++msgnum);
					printf("writer put msg: \"%s\" \n",	((char *)lpFileMapForWriters));
					
					// ������ ������� ������� � ���������
					readyState = true;
					// ������ �� ��� �� ���������
					ResetEvent(readerGetReadyEvent);
					// ����� ������
					SetEvent(readerCanReadEvent);
				}
			}
			// ��� ���������� ������
			SetEvent(canChangeCountEvent);

			break;
		default:
			printf("error with func WaitForMultipleObjects in writerHandle\n");
			printf("getlasterror= %d\n", GetLastError());
			ExitProcess(1000);
		}
	}
	printf("Writer %d finishing work\n", myid);
	return 0;
}
