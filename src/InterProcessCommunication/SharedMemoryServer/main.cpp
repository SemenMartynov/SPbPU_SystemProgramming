#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "logger.h"

#define BUF_SIZE 256
TCHAR szName[] = _T("MyFileMappingObject");
TCHAR szMsg[] = _T("Message from first process");
HANDLE mutex;

int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);

	HANDLE hMapFile;
	LPCTSTR pBuf;
	mutex = CreateMutex(NULL, false, TEXT("SyncMutex"));
	writelog(_T("Mutex created"));
	// create a memory, wicth two proccess will be working
	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE, // использование файла подкачки
		NULL, // защита по умолчанию
		PAGE_READWRITE, // доступ к чтению/записи
		0, // макс. размер объекта
		BUF_SIZE, // размер буфера
		szName); // имя отраженного в памяти объекта

	if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) {
		double errorcode = GetLastError();
		writelog(_T("CreateFileMapping failed, GLE=%d"), errorcode);
		_tprintf(_T("CreateFileMapping failed, GLE=%d"), errorcode);
		closelog();
		exit(1);
	}
	writelog(_T("FileMappingObject created"));

	pBuf = (LPTSTR)MapViewOfFile(
		hMapFile, //дескриптор проецируемого в памяти объекта
		FILE_MAP_ALL_ACCESS, // разрешение чтения/записи(режим доступа)
		0, //Старшее слово смещения файла, где начинается отображение
		0, //Младшее слово смещения файла, где начинается отображение
		BUF_SIZE); //Число отображаемых байтов файла

	if (pBuf == NULL) {
		double errorcode = GetLastError();
		writelog(_T("MapViewOfFile failed, GLE=%d"), errorcode);
		_tprintf(_T("MapViewOfFile failed, GLE=%d"), errorcode);
		closelog();
		exit(1);
	}

	int i = 0;
	while (true) {
		i = rand();
		_itow_s(i, szMsg, sizeof(szMsg), 10);
		writelog(_T("Wait For Mutex"));
		WaitForSingleObject(mutex, INFINITE);
		writelog(_T("Get Mutex"));
		CopyMemory((PVOID)pBuf, szMsg, sizeof(szMsg));
		_tprintf(_T("Write message: %s\n"), pBuf);
		writelog(_T("Write message: %s"), pBuf);
		Sleep(1000); //необходимо только для отладки - для удобства
		// представления и анализа результатов
		ReleaseMutex(mutex);
		writelog(_T("Release Mutex"));
	}
	// освобождение памяти и закрытие описателя handle
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);
	CloseHandle(mutex);

	closelog();
	exit(0);
}