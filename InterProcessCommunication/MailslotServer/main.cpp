#include <windows.h>
#include <stdio.h>
#include "logger.h"

LPTSTR SlotName = TEXT("\\\\.\\mailslot\\sample_mailslot");

BOOL WriteSlot(HANDLE hSlot, LPTSTR lpszMessage)
{
	BOOL fResult;
	DWORD cbWritten;

	writelog(_T("Text to send: %s"), lpszMessage);
	_tprintf(_T("Text to send: %s\n"), lpszMessage);

	fResult = WriteFile(hSlot,
		lpszMessage,
		(DWORD)(lstrlen(lpszMessage) + 1)*sizeof(TCHAR),
		&cbWritten,
		(LPOVERLAPPED)NULL);

	if (!fResult) {
		double errorcode = GetLastError();
		writelog(_T("WriteFile failed, GLE=%d."), errorcode);
		_tprintf(_T("WriteFile failed, GLE=%d."), errorcode);
		return FALSE;
	}
	writelog(_T("Slot written to successfully"));
	_tprintf(_T("Slot written to successfully\n"));

	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);

	HANDLE hFile;

	hFile = CreateFile(SlotName,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);

	if (hFile == INVALID_HANDLE_VALUE) 	{
		double errorcode = GetLastError();
		writelog(_T("CreateFile failed, GLE=%d."), errorcode);
		_tprintf(_T("CreateFile failed, GLE=%d."), errorcode);
		closelog();
		exit(1);
	}
	writelog(_T("Mailslot created"));
	_tprintf(_T("Mailslot created"));

	//for (int i = 0; i != 100; ++i) {
		WriteSlot(hFile, _T("Message one for mailslot."));
		WriteSlot(hFile, _T("Message two for mailslot."));
		Sleep(5000);
		WriteSlot(hFile, _T("Message three for mailslot."));
	//}
	CloseHandle(hFile);

	closelog();
	exit(0);
}