#include<windows.h>
#include<stdio.h>
#include <conio.h>

int main(int argc, char* argv[]) {
	//��������� ����� ����������
	if (argc != 2) {
		printf("error with start reader process. Need 2 arguments\n");
		_getch();
		ExitProcess(1000);
	}
	//�������� �� ��������� ������ ��� �����
	int myid = atoi(argv[1]);

	printf("reader with id= %d is started\n", myid);

	//�������������� �������� �������������
	// (�������� ������, ������������ ���������, ���):
	//�������� ������� ��������� (������ �����);
	HANDLE canReadEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_canReadEvent$$");
	//��� �������� ������ � ������ ���������� (���������);
	HANDLE canWriteEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_canWriteEvent$$");
	//��� �������� ��������� ��������� (������ �����);
	HANDLE allReadEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_allReadEvent$$");
	//���������� ������ �� ��������� (���������);
	HANDLE changeCountEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_changeCountEvent$$");
	//���������� ��������� (������ �����);
	HANDLE exitEvent = OpenEvent(EVENT_ALL_ACCESS, false, L"$$My_exitEvent$$");

	//����� ������ (�������� ������, ������������ ���������, ���):
	HANDLE hFileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, false,
		L"$$MyVerySpecialShareFileName$$");

	//���� ������� �� �������, �� �� ������ ��������
	if (canReadEvent == NULL || canWriteEvent == NULL || allReadEvent == NULL
		|| changeCountEvent == NULL || exitEvent == NULL
		|| hFileMapping == NULL) {
		printf("impossible to open objects, run server first\n getlasterror=%d",
			GetLastError());
		_getch();
		return 1001;
	}

	//���������� ���� �� �������� ������������ ������ �������� ��� �������-���������
	LPVOID lpFileMapForReaders = MapViewOfFile(hFileMapping,
		FILE_MAP_ALL_ACCESS, 0, 0, 0);
	//  hFileMapping - ���������� �������-����������� �����
	//  FILE_MAP_ALL_ACCESS - ������� � �����
	//  0, 0 - ������� � ������� ����� �������� ������ ������������� ������� � �����
	//         (0 - ������ ������������� ������� ��������� � ������� �����)
	//  0 - ������ ������������� ������� ����� � ������ (0 - ���� ����)

	HANDLE readerhandlers[2];
	readerhandlers[0] = exitEvent;
	readerhandlers[1] = canReadEvent;

	while (1) { //�������� ����
		//����, ���� ��� ���������
		WaitForSingleObject(allReadEvent, INFINITE);
		//������, ������� �������-��������� ������ ������ �������
		WaitForSingleObject(changeCountEvent, INFINITE);
		(*(((int *)lpFileMapForReaders) + 1))--;
		printf("readready= %d\n", (*(((int *)lpFileMapForReaders) + 1)));
		//���� ��� ������, �� "��������� �� ����� �����" � ��������� ������
		if ((*(((int *)lpFileMapForReaders) + 1)) == 0) {
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
			goto exit;
		case WAIT_OBJECT_0 + 1: // ��������� ������� �� ����������� ������
			//������ ���������
			printf("Reader %d read msg \"%s\"\n", myid,
				((char *)lpFileMapForReaders) + sizeof(int) * 2);

			//���������� ��������� ������� ���������� ���������, ������� ��������� ��� �� ������
			WaitForSingleObject(changeCountEvent, INFINITE);
			(*((int *)lpFileMapForReaders))--;
			printf("readcount= %d\n", (*(((int *)lpFileMapForReaders))));

			// ���� �� ��������� ������, �� ��������� ������ � ��������� �������
			if ((*((int *)lpFileMapForReaders)) == 0) {
				ResetEvent(canReadEvent);
				SetEvent(allReadEvent);
			}

			//��������� �������� �������
			SetEvent(changeCountEvent);
			break;
		default:
			printf("error with func WaitForMultipleObjects in readerHandle\n");
			printf("getlasterror= %d\n", GetLastError());
			getchar();
			ExitProcess(1001);
			break;
		}
	}
exit:
	//��������� HANDLE �������� �������������
	CloseHandle(canReadEvent);
	CloseHandle(canWriteEvent);
	CloseHandle(allReadEvent);
	CloseHandle(changeCountEvent);
	CloseHandle(exitEvent);

	UnmapViewOfFile(lpFileMapForReaders); //��������� ����� ������
	CloseHandle(hFileMapping); //��������� ������ "������������ ����"

	printf("all is done\n");
	_getch();
	return 0;
}
