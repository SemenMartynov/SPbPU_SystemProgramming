#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#include "thread.h"
#include "utils.h"
#include "Logger.h"

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
LPVOID lpFileMapForWriters; // ��������� �� ������������ ������

int _tmain(int argc, _TCHAR* argv[]) {
	Logger log(_T("ProcessWriter"));

	if (argc < 2)
		// ���������� ������������ ��-���������
		SetDefaultConfig(&config, &log);
	else
		// �������� ������� �� �����
		SetConfig(argv[1], &config, &log);

	//������� ����������� ������ ��� �� �������
	//������-�������� ����������� ����� (����� ��� ������ ����� �� ������� ��������)
	CreateAllThreads(&config, &log);

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
		log.loudlog(_T("Impossible to create shareFile, GLE = %d"),
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

	//�������������� 2 ���������� � ����� ������ (readready � readcount)
	*((int *)lpFileMapForWriters) = 0;
	*(((int *)lpFileMapForWriters) + 1) = config.numOfReaders;

	//�������������� �������� �������������
	// (�������� ������, ���������, ��������� ���������, ���):
	//������� "��������� ������" (����� ������), ������ �����, ���������� ������
	canReadEvent = CreateEvent(NULL, true, false, L"$$My_canReadEvent$$");
	//������� - "����� ������",���������(��������� ������ ������ ������), ���������� ��������
	canWriteEvent = CreateEvent(NULL, false, false, L"$$My_canWriteEvent$$");
	//������� "��� ���������"
	allReadEvent = CreateEvent(NULL, true, true, L"$$My_allReadEvent$$");
	//������� ��� ��������� �������� (������� �������� ��� �� ��������� ���������)
	changeCountEvent = CreateEvent(NULL, false, true, L"$$My_changeCountEvent$$");
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
	CloseHandle(canReadEvent);
	CloseHandle(canWriteEvent);
	CloseHandle(allReadEvent);
	CloseHandle(changeCountEvent);
	CloseHandle(exitEvent);

	UnmapViewOfFile(lpFileMapForWriters); //��������� handle ������ �������
	CloseHandle(hFileMapping); //��������� ������ "������������ ����"

	log.loudlog(_T("All is done!"));
	_getch();
	return 0;
}
