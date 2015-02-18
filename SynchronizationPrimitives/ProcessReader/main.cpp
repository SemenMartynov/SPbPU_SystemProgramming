#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <conio.h>

#include "Logger.h"

int _tmain(int argc, _TCHAR* argv[]) {
	//��������� ����� ����������
	if (argc != 2) {
		Logger log(_T("ProcessReader"));
		log.loudlog(_T("Error with start reader process. Need 2 arguments."));
		_getch();
		ExitProcess(1000);
	}
	//�������� �� ��������� ������ ��� �����
	int myid = _wtoi(argv[1]);

	Logger log(_T("ProcessReader"), myid);
	log.loudlog(_T("Reader with id= %d is started"), myid);

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
		log.loudlog(_T("Impossible to open objects, run server first\n getlasterror=%d"),
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
		log.quietlog(_T("Waining for allReadEvent"));
		WaitForSingleObject(allReadEvent, INFINITE);
		//������, ������� �������-��������� ������ ������ �������
		log.quietlog(_T("Waining for changeCountEvent"));
		WaitForSingleObject(changeCountEvent, INFINITE);
		(*(((int *)lpFileMapForReaders) + 1))--;
		log.loudlog(_T("Readready= %d\n"), (*(((int *)lpFileMapForReaders) + 1)));
		//���� ��� ������, �� "��������� �� ����� �����" � ��������� ������
		if ((*(((int *)lpFileMapForReaders) + 1)) == 0) {
			log.quietlog(_T("Reset Event allReadEvent"));
			ResetEvent(allReadEvent);
			log.quietlog(_T("Set Event canWriteEvent"));
			SetEvent(canWriteEvent);
		}

		//��������� �������� �������
		log.quietlog(_T("Set Event changeCountEvent"));
		SetEvent(changeCountEvent);

		log.quietlog(_T("Waining for multiple objects"));
		DWORD dwEvent = WaitForMultipleObjects(2, readerhandlers, false,
			INFINITE);
		//   2 - ������ �� 2-� �����������
		//   readerhandlers - �� ������� readerhandlers
		//   false - ���, ����� ����������� ���� �� ����
		//   INFINITE - ����� ����������
		switch (dwEvent) {
		case WAIT_OBJECT_0: //��������� ������� exit
			log.quietlog(_T("Get exitEvent"));
			log.loudlog(_T("Reader %d finishing work"), myid);
			goto exit;
		case WAIT_OBJECT_0 + 1: // ��������� ������� �� ����������� ������
			log.quietlog(_T("Get canReadEvent"));
			//������ ���������
			log.loudlog(_T("Reader %d read msg \"%s\""), myid,
				((_TCHAR *)lpFileMapForReaders) + sizeof(int) * 2);

			//���������� ��������� ������� ���������� ���������, ������� ��������� ��� �� ������
			log.quietlog(_T("Waining for changeCountEvent"));
			WaitForSingleObject(changeCountEvent, INFINITE);
			(*((int *)lpFileMapForReaders))--;
			log.loudlog(_T("Readcount= %d"), (*(((int *)lpFileMapForReaders))));

			// ���� �� ��������� ������, �� ��������� ������ � ��������� �������
			if ((*((int *)lpFileMapForReaders)) == 0) {
				log.quietlog(_T("Reset Event canReadEvent"));
				ResetEvent(canReadEvent);
				log.quietlog(_T("Set Event allReadEvent"));
				SetEvent(allReadEvent);
			}

			//��������� �������� �������
			log.quietlog(_T("Set Event changeCountEvent"));
			SetEvent(changeCountEvent);
			break;
		default:
			log.loudlog(_T("Error with func WaitForMultipleObjects in readerHandle, GLE = %d"), GetLastError());
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

	log.loudlog(_T("All is done"));
	_getch();
	return 0;
}
