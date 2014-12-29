#include <windows.h>
#include <stdio.h>
#include <conio.h>
#define BUF_SIZE 256
#define TIME 15
// number of reading operation in this process
TCHAR szName[] = TEXT("MyFileMappingObject");
HANDLE mutex;
void main()
{
	HANDLE hMapFile;
	LPCTSTR pBuf;
	mutex = OpenMutex(
		// request full access
		MUTEX_ALL_ACCESS,
		// handle not inheritable
		FALSE,
		// object name
		TEXT("SyncMutex"));
	if (mutex == NULL)
		printf("OpenMutex error: %d\n", GetLastError());
	else printf("OpenMutex successfully opened the mutex.\n");
	hMapFile = OpenFileMapping(
		// ������ � ������/������
		FILE_MAP_ALL_ACCESS,
		// ��� �� �����������
		FALSE,
		// ��� "������������� " �������
		szName);
	if (hMapFile == NULL)
	{
		printf("���������� ������� ������ �������� ����� (%d).\n", GetLastError());
		return;
	}
	pBuf = (LPTSTR)MapViewOfFile(hMapFile,
		// ���������� "�������������" �������
		FILE_MAP_ALL_ACCESS, // ���������� ������/������
		0,
		0,
		BUF_SIZE);
	if (pBuf == NULL)
	{
		printf("������������� ��������������� ����� (%d) ���������� .\n",
			GetLastError());
		return;
	}
	for (int i = 0; i < TIME; i++)
	{
		WaitForSingleObject(mutex, INFINITE);
		printf("read message: %s\n", (char *)pBuf);
		ReleaseMutex(mutex);
	}
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);
}