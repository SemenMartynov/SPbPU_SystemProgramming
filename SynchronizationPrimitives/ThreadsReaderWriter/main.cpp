#include<windows.h>
#include<string.h>
#include<stdio.h>
#include<conio.h>

#include"thread.h"
#include"utils.h"

//���������� ����������
struct Configuration config; //������������ ���������
bool isDone = false; //���� ����������
HANDLE *allhandlers; //������ ���� ����������� �������

//������� ��� �������������:
HANDLE canReadEvent; //�������� ������� ��������� (������ �����);
HANDLE canWriteEvent; //��� �������� ������ � ������ ���������� (���������);
HANDLE allReadEvent; //��� �������� ��������� ��������� (������ �����);
HANDLE changeCountEvent; //���������� ������ �� ��������� (���������);
HANDLE exitEvent; //���������� ��������� (������ �����);

//���������� ��� ������������� ������ �������:
int countread = 0;  //����� �������, ������� ��� ��������� ������
//                    (��������������� ��������� � ����������
//                     ���������� ����� ��������� ���������)
int countready = 0; //����� �������, ������� ��� ������ ���������
//                    (��������� ������� �� ��������)

//��� ����������� ������
wchar_t shareFileName[] = L"$$MyVerySpecialShareFileName$$";

HANDLE hFileMapping; //������-����������� �����
// ��������� �� ������������ ������
LPVOID lpFileMapForWriters;
LPVOID lpFileMapForReaders;

int main(int argc, char* argv[]) {
	if (argc < 2) {
		//���������� ������������ ��-���������
		SetConfig(NULL, &config);
	}
	else {
		char filename[30]; //��� ����� ������������
		strcpy_s(filename, argv[1]);
		// �������� ��� ��������� ����� � ����������� ��������
		SetConfig(filename, &config);
	}

	//������� ����������� ������ ��� �� �������
	//������-�������� ����������� ����� (����� ��� ������ ����� �� ������� ��������)
	CreateAllThreads(&config);

	//�������������� ������ (share memory): ������� ������ "������������ ����"
	// ����� ����������� ��������� ���� �������� (�� ����� ���� �����������
	// �� �����), �.�. � �������� ����������� ����� ������������ ��������
	// ������ 0xFFFFFFFF (��� ���������� � ������������� ��������� INVALID_HANDLE_VALUE)
	if ((hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
		PAGE_READWRITE, 0, 1500, shareFileName)) == NULL) {
		// INVALID_HANDLE_VALUE - ���������� ��������� �����
		//                        (INVALID_HANDLE_VALUE - ���� ��������)
		// NULL - �������� ������ �������-�����������
		// PAGE_READWRITE - ���������� ������� � ������������� ����� ���
		//                  ����������� (PAGE_READWRITE - ������/������)
		// 0, 1500 - ������� � ������� ����� �������� �������������
		//           ������� ������� ����������� �����
		// shareFileName - ��� �������-�����������.
		printf("impossible to create shareFile\n Last error %d\n",
			GetLastError());
		ExitProcess(10000);
	}
	//���������� ���� �� �������� ������������ ������ �������� ��� ������-��������
	lpFileMapForWriters = MapViewOfFile(hFileMapping, FILE_MAP_WRITE, 0, 0, 0);
	//  hFileMapping - ���������� �������-����������� �����
	//  FILE_MAP_WRITE - ������� � �����
	//  0, 0 - ������� � ������� ����� �������� ������ ������������� ������� � �����
	//         (0 - ������ ������������� ������� ��������� � ������� �����)
	//  0 - ������ ������������� ������� ����� � ������ (0 - ���� ����)

	//���������� ���� �� �������� ������������ ������ �������� ��� �������-���������
	lpFileMapForReaders = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

	//�������������� �������� �������������
	// (�������� ������, ���������, ��������� ���������, ���):
	//������� "��������� ������" (����� ������), ������ �����, ���������� ������
	canReadEvent = CreateEvent(NULL, true, false, L"");
	//������� - "����� ������",���������(��������� ������ ������ ������), ���������� ��������
	canWriteEvent = CreateEvent(NULL, false, false, L"");
	//������� "��� ���������"
	allReadEvent = CreateEvent(NULL, true, true, L"");
	//������� ��� ��������� �������� (������� �������� ��� �� ��������� ���������)
	changeCountEvent = CreateEvent(NULL, false, true, L"");
	//������� "���������� ������ ���������", ������ �����, ���������� ������
	exitEvent = CreateEvent(NULL, true, false, L"");

	//��������� ������-�������� � �����-����������� �� ����������
	for (int i = 0; i < config.numOfReaders + config.numOfWriters + 1; i++)
		ResumeThread(allhandlers[i]);

	//������� ���������� ���� �������
	WaitForMultipleObjects(config.numOfReaders + config.numOfWriters + 1,
		allhandlers, TRUE, INFINITE);

	//��������� handle �������
	for (int i = 0; i < config.numOfReaders + config.numOfWriters + 1; i++)
		CloseHandle(allhandlers[i]);

	//��������� ��������� �������� �������������
	CloseHandle(canReadEvent);
	CloseHandle(canWriteEvent);
	CloseHandle(allReadEvent);
	CloseHandle(changeCountEvent);
	CloseHandle(exitEvent);

	//��������� handle ������ �������
	UnmapViewOfFile(lpFileMapForReaders);
	UnmapViewOfFile(lpFileMapForWriters);

	//��������� ������ "������������ ����"
	CloseHandle(hFileMapping);
	printf("all is done\n");
	_getch();
	return 0;
}
