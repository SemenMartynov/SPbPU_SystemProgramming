#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <conio.h>

#include "Logger.h"

int _tmain(int argc, _TCHAR* argv[]) {
	//��������� ����� ����������
	if (argc != 3) {
		Logger log(_T("ProcessReader"));
		log.loudlog(_T("Error with start reader process. Need 2 arguments, but %d presented."), argc);
		_getch();
		ExitProcess(1000);
	}
	//�������� �� ��������� ������ ��� �����
	int myid = _wtoi(argv[1]);
	int pause = _wtoi(argv[2]);

	Logger log(_T("ProcessReader"), myid);
	log.loudlog(_T("Reader with id= %d is started"), myid);

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
		log.loudlog(_T("Impossible to open objects, run server first\n getlasterror=%d"),
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
			log.quietlog(_T("Waining for multiple objects"));
			DWORD dwEvent = WaitForMultipleObjects(2, readerHandlers, false,
				INFINITE);
			//   2 - ������ �� 2-� �����������
			//   readerHandlers - �� ������� readerHandlers
			//   false - ���, ����� ����������� ���� �� ����
			//   INFINITE - ����� ����������
			switch (dwEvent) {
			case WAIT_OBJECT_0: //��������� ������� exit
				log.quietlog(_T("Get exitEvent"));
				log.loudlog(_T("Reader %d finishing work"), myid);
				goto exit;

			case WAIT_OBJECT_0 + 1: // ��������� ������� �� ����������� ������
				log.quietlog(_T("Get readerCanReadEvent"));
				//������ ���������
				log.loudlog(_T("Reader %d read msg \"%s\""), myid, (_TCHAR *)lpFileMapForReaders);

				// ���������� �����
				log.quietlog(_T("Waining for canChangeCountEvent"));
				WaitForSingleObject(canChangeCountEvent, INFINITE);
				log.quietlog(_T("Set Event changeCountEvent"));
				SetEvent(changeCountEvent);

				// ��������� ������
				readyState = false;
				break;
			default:
				log.loudlog(_T("Error with func WaitForMultipleObjects in readerHandle, GLE = %d"), GetLastError());
				getchar();
				ExitProcess(1001);
				break;
			}
		}
		else {
			log.quietlog(_T("Waining for multiple objects"));
			DWORD dwEvent = WaitForMultipleObjects(2, readyHandlers, false,
				INFINITE);
			//   2 - ������ �� 2-� �����������
			//   readyHandlers - �� ������� readyHandlers
			//   false - ���, ����� ����������� ���� �� ����
			//   INFINITE - ����� ����������
			switch (dwEvent) {
			case WAIT_OBJECT_0: //��������� ������� exit
				log.quietlog(_T("Get exitEvent"));
				log.loudlog(_T("Reader %d finishing work"), myid);
				goto exit;

			case WAIT_OBJECT_0 + 1: // ��������� ������� �������� � ����� ����������
				log.quietlog(_T("Get readerGetReadyEvent"));
				// ���������� �����
				log.quietlog(_T("Waining for canChangeCountEvent"));
				WaitForSingleObject(canChangeCountEvent, INFINITE);
				log.quietlog(_T("Set Event changeCountEvent"));
				SetEvent(changeCountEvent);

				// ��������� ������
				readyState = true;
				break;
			default:
				log.loudlog(_T("Error with func WaitForMultipleObjects in readerHandle, GLE = %d"), GetLastError());
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

	log.loudlog(_T("All is done"));
	_getch();
	return 0;
}
