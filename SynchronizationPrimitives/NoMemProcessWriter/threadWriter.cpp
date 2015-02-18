#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"

DWORD WINAPI ThreadWriterHandler(LPVOID prm) {
	int myid = (int)prm;

	Logger log(_T("NoMemProcessWriter.ThreadWriter"), myid);
	extern bool isDone;
	extern struct Configuration config;

	extern HANDLE readerCanReadEvent;
	extern HANDLE readerGetReadyEvent;
	extern HANDLE canChangeCountEvent;
	extern HANDLE changeCountEvent;
	extern HANDLE exitEvent;

	extern int reportCounter;  // ��������� �������
	extern LPVOID lpFileMapForWriters;

	int msgnum = 0;
	HANDLE writerHandlers[2];
	writerHandlers[0] = exitEvent;
	writerHandlers[1] = changeCountEvent;

	// ��������� ����������:
	// true - ��������� ��������, ��� ������� � ���������
	// false - ��������� ���� ��������� � ��������� ����������
	bool readyState = false;

	while (isDone != true) {
		log.quietlog(_T("Waining for multiple objects"));
		DWORD dwEvent = WaitForMultipleObjects(2, writerHandlers, false,
			INFINITE);
		//   2 - ������ �� 2-� �����������
		//   writerHandlers - �� ������� writerHandlers
		//   false - ���, ����� ����������� ���� �� ����
		//   INFINITE - ����� ����������
		switch (dwEvent) {
		case WAIT_OBJECT_0:	//��������� ������� exit
			log.quietlog(_T("Get exitEvent"));
			log.loudlog(_T("Writer %d finishing work"), myid);
			return 0;
		case WAIT_OBJECT_0 + 1: // ������ ����� � ����������
			log.quietlog(_T("Get changeCountEvent"));
			// ���� ���������� ��� ��������
			if (++reportCounter == config.numOfReaders) {
				// ��������� ��������
				reportCounter = 0;
				if (readyState) { // ��� �� ���������
					// ������ ������� ������� � ����������
					readyState = false;
					// ������ �� ��� �� ������
					log.quietlog(_T("Reset Event readerCanReadEvent"));
					ResetEvent(readerCanReadEvent);
					// ����� ���������
					log.quietlog(_T("Set Event readerGetReadyEvent"));
					SetEvent(readerGetReadyEvent);
				}
				else { // ��� ������ ������
					// ������ ���������
					swprintf_s((_TCHAR *)lpFileMapForWriters, 1500,
						_T("Writer_id	%d, msg with num = %d"), myid, ++msgnum);
					log.loudlog(_T("Writer put msg: \"%s\""), (_TCHAR *)lpFileMapForWriters);
					
					// ������ ������� ������� � ���������
					readyState = true;
					// ������ �� ��� �� ���������
					log.quietlog(_T("Reset Event readerGetReadyEvent"));
					ResetEvent(readerGetReadyEvent);
					// ����� ������
					log.quietlog(_T("Set Event readerCanReadEvent"));
					SetEvent(readerCanReadEvent);
				}
			}
			// ��� ���������� ������
			log.quietlog(_T("Set Event canChangeCountEvent"));
			SetEvent(canChangeCountEvent);

			break;
		default:
			log.loudlog(_T("Error with func WaitForMultipleObjects in writerHandle, GLE = %d"), GetLastError());
			ExitProcess(1000);
		}
	}
	log.loudlog(_T("Writer %d finishing work"), myid);
	return 0;
}
