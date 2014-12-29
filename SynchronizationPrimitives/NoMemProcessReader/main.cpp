#include<windows.h>
#include<stdio.h>
#include <conio.h>

int main(int argc, char* argv[]) {
	//��������� ����� ����������
	if (argc != 3) {
		printf("error with start reader process. Need 2 arguments, but %d presented.\n", argc);
		_getch();
		ExitProcess(1000);
	}
	//�������� �� ��������� ������ ��� �����
	int myid = atoi(argv[1]);
	int pause = atoi(argv[2]);
	printf("reader with id= %d is started\n", myid);

	// ��������� ����������:
	// true - ��� ��������� ��� ������
	// false - ������� ��������� ��� ���������,
	//         ��� ������� �������� � ����� ����������
	bool readyState = false;

	//�������������� �������� �������������
	// (�������� ������, ������������ ���������, ���):
	//�������� ������� ��������� (������ �����);
	HANDLE readerCanReadEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_readerCanReadEvent$$");
	//��� �������� ������ � ������ ���������� (���������);
	HANDLE readerGetReadyEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_readerGetReadyEvent$$");
	//���������� ������ �� ��������� (���������);
	HANDLE canChangeCountEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_canChangeCountEvent$$");
	//
	HANDLE changeCountEvent = OpenEvent(EVENT_ALL_ACCESS, false,
		L"$$My_changeCountEvent$$");
	//���������� ��������� (������ �����);
	HANDLE exitEvent = OpenEvent(EVENT_ALL_ACCESS, false, L"$$My_exitEvent$$");

	//����� ������ (�������� ������, ������������ ���������, ���):
	HANDLE hFileMapping = OpenFileMapping(FILE_MAP_READ, false,
		L"$$MyVerySpecialShareFileName$$");

	//���� ������� �� �������, �� �� ������ ��������
	if (readerCanReadEvent == NULL || readerGetReadyEvent == NULL || canChangeCountEvent == NULL
		|| changeCountEvent == NULL || exitEvent == NULL
		|| hFileMapping == NULL) {
		printf("impossible to open objects, run server first\n getlasterror=%d",
			GetLastError());
		_getch();
		return 1001;
	}

	//���������� ���� �� �������� ������������ ������ �������� ��� �������-���������
	LPVOID lpFileMapForReaders = MapViewOfFile(hFileMapping,
		FILE_MAP_READ, 0, 0, 0);
	//  hFileMapping - ���������� �������-����������� �����
	//  FILE_MAP_ALL_ACCESS - ������� � �����
	//  0, 0 - ������� � ������� ����� �������� ������ ������������� ������� � �����
	//         (0 - ������ ������������� ������� ��������� � ������� �����)
	//  0 - ������ ������������� ������� ����� � ������ (0 - ���� ����)

	// ������� �������
	HANDLE readerHandlers[2];
	readerHandlers[0] = exitEvent;
	readerHandlers[1] = readerCanReadEvent;

	// ������� ����������
	HANDLE readyHandlers[2];
	readyHandlers[0] = exitEvent;
	readyHandlers[1] = readerGetReadyEvent;

	while (1) { //�������� ����
		// ������� ����� ������� � ����������� �� ���������
		if (readyState) {
			DWORD dwEvent = WaitForMultipleObjects(2, readerHandlers, false,
				INFINITE);
			//   2 - ������ �� 2-� �����������
			//   readerHandlers - �� ������� readerHandlers
			//   false - ���, ����� ����������� ���� �� ����
			//   INFINITE - ����� ����������
			switch (dwEvent) {
			case WAIT_OBJECT_0: //��������� ������� exit
				printf("Reader %d finishing work\n", myid);
				goto exit;

			case WAIT_OBJECT_0 + 1: // ��������� ������� �� ����������� ������
				//������ ���������
				printf("Reader %d read msg \"%s\"\n", myid, (char *)lpFileMapForReaders);

				// ���������� �����
				WaitForSingleObject(canChangeCountEvent, INFINITE);
				SetEvent(changeCountEvent);

				// ��������� ������
				readyState = false;
				break;
			default:
				printf("error with func WaitForMultipleObjects in readerHandle\n");
				printf("getlasterror= %d\n", GetLastError());
				getchar();
				ExitProcess(1001);
				break;
			}
		}
		else {
			DWORD dwEvent = WaitForMultipleObjects(2, readyHandlers, false,
				INFINITE);
			//   2 - ������ �� 2-� �����������
			//   readyHandlers - �� ������� readyHandlers
			//   false - ���, ����� ����������� ���� �� ����
			//   INFINITE - ����� ����������
			switch (dwEvent) {
			case WAIT_OBJECT_0: //��������� ������� exit
				printf("Reader %d finishing work\n", myid);
				goto exit;

			case WAIT_OBJECT_0 + 1: // ��������� ������� �������� � ����� ����������
				// ���������� �����
				WaitForSingleObject(canChangeCountEvent, INFINITE);
				SetEvent(changeCountEvent);

				// ��������� ������
				readyState = true;
				break;
			default:
				printf("error with func WaitForMultipleObjects in readerHandle\n");
				printf("getlasterror= %d\n", GetLastError());
				getchar();
				ExitProcess(1001);
				break;
			}
		}
		Sleep(pause);
	}
exit:
	//��������� HANDLE �������� �������������
	CloseHandle(readerCanReadEvent);
	CloseHandle(readerGetReadyEvent);
	CloseHandle(canChangeCountEvent);
	CloseHandle(changeCountEvent);
	CloseHandle(exitEvent);

	UnmapViewOfFile(lpFileMapForReaders); //��������� ����� ������
	CloseHandle(hFileMapping); //��������� ������ "������������ ����"

	printf("all is done\n");
	_getch();
	return 0;
}
