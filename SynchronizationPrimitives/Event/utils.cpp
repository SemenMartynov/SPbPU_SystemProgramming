#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "thread.h"
#include "utils.h"
#include "Logger.h"

//��������, ��������� � ������ �������
HANDLE CreateAndStartWaitableTimer(int sec) {
	__int64 end_time;
	LARGE_INTEGER end_time2;
	HANDLE tm = CreateWaitableTimer(NULL, false, _T("Timer!"));
	end_time = -1 * sec * 10000000;
	end_time2.LowPart = (DWORD)(end_time & 0xFFFFFFFF);
	end_time2.HighPart = (LONG)(end_time >> 32);
	SetWaitableTimer(tm, &end_time2, 0, NULL, NULL, false);
	return tm;
}

//�������� ���� �������
void CreateAllThreads(struct Configuration* config, Logger* log) {
	extern HANDLE *allhandlers;

	int total = config->numOfReaders + config->numOfWriters + 1;
	log->quietlog(_T("Total num of threads is %d"), total);
	allhandlers = new HANDLE[total];
	int count = 0;

	//������� ������-��������
	log->loudlog(_T("Create readers"));
	for (int i = 0; i != config->numOfReaders; ++i, ++count) {
		log->loudlog(_T("Count = %d"), count);
		//������� ������-��������, ������� ���� �� ��������
		if ((allhandlers[count] = CreateThread(NULL, 0, ThreadReaderHandler, (LPVOID)i, CREATE_SUSPENDED, NULL)) == NULL) {
			log->loudlog(_T("Impossible to create thread-reader, GLE = %d"), GetLastError());
			exit(8000);
		}
	}

	//������� ������-��������
	log->loudlog(_T("Create writers"));
	for (int i = 0; i != config->numOfWriters; ++i, ++count) {
		log->loudlog(_T("count = %d"), count);
		//������� ������-���������, ������� ���� �� ��������
		if ((allhandlers[count] = CreateThread(NULL, 0, ThreadWriterHandler, (LPVOID)i, CREATE_SUSPENDED, NULL)) == NULL) {
			log->loudlog(_T("Impossible to create thread-writer, GLE = %d"), GetLastError());
			exit(8001);
		}
	}

	//������� ����� TimeManager
	log->loudlog(_T("Create TimeManager"));
	log->loudlog(_T("Count = %d"), count);
	//������� ������-��������, ������� ���� �� ��������
	if ((allhandlers[count] = CreateThread(NULL, 0, ThreadTimeManagerHandler, (LPVOID)config->ttl, CREATE_SUSPENDED, NULL)) == NULL) {
		log->loudlog(_T("impossible to create thread-reader, GLE = %d"), GetLastError());
		exit(8002);
	}
	log->loudlog(_T("Successfully created threads!"));
}

//������� ��������� ������������
void SetConfig(_TCHAR* path, struct Configuration* config, Logger* log) {
	_TCHAR filename[255];
	wcscpy_s(filename, path);
	log->quietlog(_T("Using config from %s"), filename);

	FILE *confsource;
	int numOfReaders;
	int numOfWriters;
	int readersDelay;
	int writersDelay;
	int sizeOfQueue;
	int ttl;
	_TCHAR trash[30];

	if (_wfopen_s(&confsource, filename, _T("r"))) {
		_wperror(_T("The following error occurred"));
		log->loudlog(_T("impossible open config file %s\n"), filename);
		exit(1000);
	}

	//�������� ������ ������������
	fscanf_s(confsource, "%s %d", trash, _countof(trash), &numOfReaders); //����� �������-���������
	fscanf_s(confsource, "%s %d", trash, _countof(trash), &readersDelay); //�������� �������-���������
	fscanf_s(confsource, "%s %d", trash, _countof(trash), &numOfWriters); //����� �������-���������
	fscanf_s(confsource, "%s %d", trash, _countof(trash), &writersDelay); //�������� �������-���������
	fscanf_s(confsource, "%s %d", trash, _countof(trash), &sizeOfQueue); //������ �������
	fscanf_s(confsource, "%s %d", trash, _countof(trash), &ttl); //����� �����

	if (numOfReaders <= 0 || numOfWriters <= 0) {
		log->loudlog(_T("Incorrect num of Readers or writers"));
		exit(500);
	}
	else if (readersDelay <= 0 || writersDelay <= 0) {
		log->loudlog(_T("Incorrect delay of Readers or writers"));
		exit(501);
	}
	else if (sizeOfQueue <= 0) {
		log->loudlog(_T("Incorrect size of queue"));
		exit(502);
	}
	else if (ttl == 0) {
		log->loudlog(_T("Incorrect ttl"));
		exit(503);
	}
	fclose(confsource);

	config->numOfReaders = numOfReaders;
	config->readersDelay = readersDelay;
	config->numOfWriters = numOfWriters;
	config->writersDelay = writersDelay;
	config->sizeOfQueue = sizeOfQueue;
	config->ttl = ttl;

	log->quietlog(_T("Config:\n\tNumOfReaders = %d\n\tReadersDelay = %d\n\tNumOfWriters = %d\n\tWritersDelay = %d\n\tSizeOfQueue = %d\n\tttl = %d"),
		config->numOfReaders, config->readersDelay, config->numOfWriters, config->writersDelay, config->sizeOfQueue, config->ttl);
}

void SetDefaultConfig(struct Configuration* config, Logger* log) {
	log->quietlog(_T("Using default config"));
	//��� ����������������� �����:
	//     NumOfReaders= 10
	//     ReadersDelay= 100
	//     NumOfWriters= 10
	//     WritersDelay= 200
	//     SizeOfQueue= 10
	//     ttl= 3

	config->numOfReaders = 10;
	config->readersDelay = 100;
	config->numOfWriters = 10;
	config->writersDelay = 200;
	config->sizeOfQueue = 10;
	config->ttl = 3;

	log->quietlog(_T("Config:\n\tNumOfReaders = %d\n\tReadersDelay = %d\n\tNumOfWriters = %d\n\tWritersDelay = %d\n\tSizeOfQueue = %d\n\tttl = %d"),
		config->numOfReaders, config->readersDelay, config->numOfWriters, config->writersDelay, config->sizeOfQueue, config->ttl);
}