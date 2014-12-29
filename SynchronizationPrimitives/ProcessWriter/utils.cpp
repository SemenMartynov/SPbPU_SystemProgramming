#include<windows.h>
#include<stdio.h>
#include<tchar.h>

#include"thread.h"
#include"utils.h"

//создание, установка и запуск таймера
HANDLE CreateAndStartWaitableTimer(int sec) {
	__int64 end_time;
	LARGE_INTEGER end_time2;
	HANDLE tm = CreateWaitableTimer(NULL, false, L"timer");
	end_time = -1 * sec * 10000000;
	end_time2.LowPart = (DWORD)(end_time & 0xFFFFFFFF);
	end_time2.HighPart = (LONG)(end_time >> 32);
	SetWaitableTimer(tm, &end_time2, 0, NULL, NULL, false);
	return tm;
}

//создание всех потоков
void CreateAllThreads(struct Configuration * config) {
	extern HANDLE *allhandlers;
	printf(
		"createConfig:\n NumOfreadrs = %d | ReadersDelay= %d | NumOfwriters= %d | WritersDelay = %d | sizeofqueue = %d | ttl = %d\n",
		config->numOfReaders, config->readersDelay, config->numOfWriters,
		config->writersDelay, config->sizeOfQueue, config->ttl);
	allhandlers = new HANDLE[config->numOfReaders + config->numOfWriters + 1];
	int count = 0;

	//создаем потоки-читатели
	printf("create readers\n");

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	TCHAR szCommandLine[100];
	
	for (int i = 0; i != config->numOfReaders; i++, count++) {
		_stprintf_s(szCommandLine,  _T("ProcessReader.exe %d"), i);
		printf("count= %d\n", count);
		if (!CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE |
			CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
			perror("Create process");
			exit(1);
		}
		allhandlers[count] = pi.hThread;
	}

	//создаем потоки-писатели
	printf("create writers\n");
	for (int i = 0; i != config->numOfWriters; i++, count++) {
		printf("count= %d\n", count);
		//создаем потоки-читатели, которые пока не стартуют
		if ((allhandlers[count] = CreateThread(NULL, 0, ThreadWriterHandler,
			(LPVOID)i, CREATE_SUSPENDED, NULL)) == NULL) {
			printf("impossible to create thread-writer\n");
			exit(8001);
		}
	}

	//создаем поток TimeManager
	printf("create TimeManager\n");
	printf("count= %d\n", count);
	//создаем потоки-читатели, которые пока не стартуют
	if ((allhandlers[count] = CreateThread(NULL, 0, ThreadTimeManagerHandler,
		(LPVOID)config->ttl, CREATE_SUSPENDED, NULL)) == NULL) {
		printf("impossible to create thread-reader\n");
		exit(8002);
	}
	printf("successfully created threads\n");
	return;
}

//функци€ установки конфигурации
void SetConfig(char * filename, struct Configuration * config) {
	if (filename) {
		FILE *f;
		int numOfReaders;
		int numOfWriters;
		int readersDelay;
		int writersDelay;
		int sizeOfQueue;
		int ttl;
		char tmp[20];

		if (!fopen_s(&f, filename, "r")) {
			printf("impossible open config file %s\n", filename);
			exit(1000);
		}

		//начинаем читать конфигурацию
		fscanf_s(f, "%s %d", tmp, sizeof(tmp), &numOfReaders); //число потоков-читателей
		fscanf_s(f, "%s %d", tmp, sizeof(tmp), &readersDelay); //задержки потоков-читателей
		fscanf_s(f, "%s %d", tmp, sizeof(tmp), &numOfWriters); //число потоков-писателей
		fscanf_s(f, "%s %d", tmp, sizeof(tmp), &writersDelay); //задержки потоков-писателей
		fscanf_s(f, "%s %d", tmp, sizeof(tmp), &sizeOfQueue); //размер очереди
		fscanf_s(f, "%s %d", tmp, sizeof(tmp), &ttl); //врем€ жизни

		if (numOfReaders <= 0 || numOfWriters <= 0) {
			printf("incorrect num of Readers or writers\n");
			exit(500);
		}
		else if (readersDelay <= 0 || writersDelay <= 0) {
			printf("incorrect delay of Readers or writers\n");
			exit(501);
		}
		else if (sizeOfQueue <= 0) {
			printf("incorrect size of queue\n");
			exit(502);
		}
		else if (ttl == 0) {
			printf("incorrect ttl\n");
			exit(503);
		}

		config->numOfReaders = numOfReaders;
		config->readersDelay = readersDelay;
		config->numOfWriters = numOfWriters;
		config->writersDelay = writersDelay;
		config->sizeOfQueue = sizeOfQueue;
		config->ttl = ttl;
	}
	else {
		//¬ид конфигурационного файла:
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
		config->ttl = 30;
	}
	return;
}
