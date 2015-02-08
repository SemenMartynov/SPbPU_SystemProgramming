//http://msdn.microsoft.com/ru-ru/windows/desktop/aa365802%28v=vs.85%29

#include <windows.h>
#include <stdio.h>

LPTSTR SlotName = TEXT("\\\\.\\mailslot\\sample_mailslot");

BOOL WriteSlot(HANDLE hSlot, LPTSTR lpszMessage)
{
	BOOL fResult;
	DWORD cbWritten;

	fResult = WriteFile(hSlot,
		lpszMessage,
		(DWORD)(lstrlen(lpszMessage) + 1)*sizeof(TCHAR),
		&cbWritten,
		(LPOVERLAPPED)NULL);

	if (!fResult)
	{
		printf("WriteFile failed with %d.\n", GetLastError());
		return FALSE;
	}

	printf("Slot written to successfully.\n");

	return TRUE;
}

int main()
{
	HANDLE hFile;

	hFile = CreateFile(SlotName,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed with %d.\n", GetLastError());
		return FALSE;
	}

	WriteSlot(hFile, TEXT("Message one for mailslot."));
	WriteSlot(hFile, TEXT("Message two for mailslot."));

	Sleep(5000);

	WriteSlot(hFile, TEXT("Message three for mailslot."));

	CloseHandle(hFile);

	return TRUE;
}