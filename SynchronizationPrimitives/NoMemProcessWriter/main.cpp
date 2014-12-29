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
// ������� ������� ���������, �������� ����� ��� ���������
HANDLE readerCanReadEvent;
// ��� �������� ������ ������� � ����� ����������
HANDLE readerGetReadyEvent;
// ����� ����� ���� ���������
HANDLE canChangeCountEvent;
// �����
HANDLE changeCountEvent;
//���������� ��������� (������ �����);
HANDLE exitEvent;

//���������� ��� ������������� ������ �������:
int reportCounter = 0; // ��������� �������

//��� ����������� ������
wchar_t shareFileName[] = L"$$MyVerySpecialShareFileName$$";

HANDLE hFileMapping; //������-����������� �����
LPVOID lpFileMapForWriters; // ��������� �� ������������ ������

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

	//�������������� �������� �������������
	// (�������� ������, ������ �����, ��������� ���������, ���):
	//������� "��������� ������" (����� ������), ������ �����, ���������� ������
	readerCanReadEvent = CreateEvent(NULL, true, false, L"$$My_readerCanReadEvent$$");
	//������� - "����� ������",���������(��������� ������ ������ ������), ���������� ��������
	readerGetReadyEvent = CreateEvent(NULL, true, true, L"$$My_readerGetReadyEvent$$");
	//������� ��� ��������� �������� (������� �������� ��� �� ��������� ���������)
	canChangeCountEvent = CreateEvent(NULL, false, true, L"$$My_canChangeCountEvent$$");
	changeCountEvent = CreateEvent(NULL, false, false, L"$$My_changeCountEvent$$");
	//������� "���������� ������ ���������", ������ �����, ���������� ������
	exitEvent = CreateEvent(NULL, true, false, L"$$My_exitEvent$$");

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
	CloseHandle(readerCanReadEvent);
	CloseHandle(readerGetReadyEvent);
	CloseHandle(canChangeCountEvent);
	CloseHandle(changeCountEvent);
	CloseHandle(exitEvent);

	UnmapViewOfFile(lpFileMapForWriters); //��������� handle ������ �������
	CloseHandle(hFileMapping); //��������� ������ "������������ ����"

	printf("all is done\n");
	_getch();
	return 0;
}
