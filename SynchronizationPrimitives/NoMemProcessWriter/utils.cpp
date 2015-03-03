#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "thread.h"
#include "utils.h"
#include "Logger.h"

//создание, установка и запуск таймера
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

//создание всех потоков
void CreateAllThreads(struct Configuration* config, Logger* log) {
	extern HANDLE *allhandlers;
	
	int total = config->numOfReaders + config->numOfWriters + 1;
	log->quietlog(_T("Total num of threads is %d"), total);
	allhandlers = new HANDLE[total];
	int count = 0;

	//создаем потоки-читатели
	log->loudlog(_T("Create readers"));

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	TCHAR szCommandLine[100];
	
	for (int i = 0; i != config->numOfReaders; i++, count++) {
		_stprintf_s(szCommandLine, _T("NoMemProcessReader.exe %d %d"), i, config->readersDelay);
		log->loudlog(_T("Count = %d"), count);
		if (!CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE |
			CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
			log->loudlog(_T("Impossible to create Process-reader, GLE = %d"), GetLastError());
			exit(8000);
		}
		allhandlers[count] = pi.hThread;
	}

	//создаем потоки-писатели
	log->loudlog(_T("Create writers"));
	for (int i = 0; i != config->numOfWriters; i++, count++) {
		log->loudlog(_T("count = %d"), count);
		//создаем потоки-читатели, которые пока не стартуют
		if ((allhandlers[count] = CreateThread(NULL, 0, ThreadWriterHandler,
			(LPVOID)i, CREATE_SUSPENDED, NULL)) == NULL) {
			log->loudlog(_T("Impossible to create thread-writer, GLE = %d"), GetLastError());
			exit(8001);
		}
	}

	//создаем поток TimeManager
	log->loudlog(_T("Create TimeManager"));
	log->loudlog(_T("Count = %d"), count);
	//создаем поток TimeManager, который пока не стартуют
	if ((allhandlers[count] = CreateThread(NULL, 0, ThreadTimeManagerHandler,
		(LPVOID)config->ttl, CREATE_SUSPENDED, NULL)) == NULL) {
		log->loudlog(_T("impossible to create thread-reader, GLE = %d"), GetLastError());
		exit(8002);
	}
	log->loudlog(_T("Successfully created threads!"));
	return;
}

//функция установки конфигурации
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

	//начинаем читать конфигурацию
	fscanf_s(confsource, "%s %d", trash, _countof(trash), &numOfReaders); //число потоков-читателей
	fscanf_s(confsource, "%s %d", trash, _countof(trash), &readersDelay); //задержки потоков-читателей
	fscanf_s(confsource, "%s %d", trash, _countof(trash), &numOfWriters); //число потоков-писателей
	fscanf_s(confsource, "%s %d", trash, _countof(trash), &writersDelay); //задержки потоков-писателей
	fscanf_s(confsource, "%s %d", trash, _countof(trash), &sizeOfQueue); //размер очереди
	fscanf_s(confsource, "%s %d", trash, _countof(trash), &ttl); //время жизни

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
	config->numOfWriters = 1;
	config->writersDelay = writersDelay;
	config->sizeOfQueue = sizeOfQueue;
	config->ttl = ttl;

	log->quietlog(_T("Config:\n\tNumOfReaders = %d\n\tReadersDelay = %d\n\tNumOfWriters = %d\n\tWritersDelay = %d\n\tSizeOfQueue = %d\n\tttl = %d"),
		config->numOfReaders, config->readersDelay, config->numOfWriters, config->writersDelay, config->sizeOfQueue, config->ttl);
}

void SetDefaultConfig(struct Configuration* config, Logger* log) {
	log->quietlog(_T("Using default config"));
	//Вид конфигурационного файла:
	//     NumOfReaders= 10
	//     ReadersDelay= 100
	//     NumOfWriters= 10
	//     WritersDelay= 200
	//     SizeOfQueue= 10
	//     ttl= 3

	config->numOfReaders = 10;
	config->readersDelay = 100;
	config->numOfWriters = 1;
	config->writersDelay = 200;
	config->sizeOfQueue = 10;
	config->ttl = 3;

	log->quietlog(_T("Config:\n\tNumOfReaders = %d\n\tReadersDelay = %d\n\tNumOfWriters = %d\n\tWritersDelay = %d\n\tSizeOfQueue = %d\n\tttl = %d"),
		config->numOfReaders, config->readersDelay, config->numOfWriters, config->writersDelay, config->sizeOfQueue, config->ttl);
}
