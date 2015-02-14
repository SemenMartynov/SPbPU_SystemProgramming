#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "logger.h"

#define BUF_SIZE 256
#define TIME 150
// number of reading operation in this process
TCHAR szName[] = _T("MyFileMappingObject");
HANDLE mutex;

int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);
	Sleep(1000);

	HANDLE hMapFile;
	LPCTSTR pBuf;
	mutex = OpenMutex( 
		MUTEX_ALL_ACCESS, // request full access
		FALSE, // handle not inheritable
		TEXT("SyncMutex")); // object name
	if (mutex == NULL) {
		double errorcode = GetLastError();
		writelog(_T("OpenMutex error, GLE=%d"), errorcode);
		_tprintf(_T("OpenMutex error, GLE=%d\n"), errorcode);
	}
	writelog(_T("OpenMutex successfully opened the mutex"));
	_tprintf(_T("OpenMutex successfully opened the mutex.\n"));

	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS, // доступ к чтению/записи
		FALSE, // имя не наследуется
		szName); // имя "проецируемого " объекта
	if (hMapFile == NULL) {
		double errorcode = GetLastError();
		writelog(_T("OpenFileMapping failed, GLE=%d"), errorcode);
		_tprintf(_T("OpenFileMapping failed, GLE=%d"), errorcode);
		closelog();
		exit(1);
	}
	pBuf = (LPTSTR)MapViewOfFile(hMapFile,
		// дескриптор "проецируемого" объекта
		FILE_MAP_ALL_ACCESS, // разрешение чтения/записи
		0, 0, BUF_SIZE);
	if (pBuf == NULL) {
		double errorcode = GetLastError();
		writelog(_T("MapViewOfFile failed, GLE=%d"), errorcode);
		_tprintf(_T("MapViewOfFile failed, GLE=%d"), errorcode);
		closelog();
		exit(1);
	}
	for (int i = 0; i < TIME; i++) {
		writelog(_T("Wait For Mutex"));
		WaitForSingleObject(mutex, INFINITE);
		writelog(_T("Get Mutex"));
		_tprintf(_T("Read message: %s\n"), pBuf);
		writelog(_T("Read message: %s"), pBuf);
		ReleaseMutex(mutex);
		writelog(_T("Release Mutex"));
	}
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);

	closelog();
	exit(0);
}