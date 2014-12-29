#include <windows.h>
#include <stdio.h>
#include <conio.h>
#define BUF_SIZE 256
TCHAR szName[] = TEXT("MyFileMappingObject");
TCHAR szMsg[] = TEXT("Message from first process");
HANDLE mutex;
void main()
{
	HANDLE hMapFile;
	LPCTSTR pBuf;
	mutex = CreateMutex(NULL, false, TEXT("SyncMutex"));
	// create a memory, wicth two proccess will be working
	hMapFile = CreateFileMapping(
		// ������������� ����� ��������
		INVALID_HANDLE_VALUE,
		// ������ �� ���������
		NULL,
		// ������ � ������/������
		PAGE_READWRITE,
		// ����. ������ �������
		0,
		// ������ ������
		BUF_SIZE,
		// ��� ����������� � ������ �������
		szName);

	if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE)
	{
		printf("�� ����� ������� ���������� � ������ ������ (%d).\n",
			GetLastError());
		return;
	}
	pBuf = (LPTSTR)MapViewOfFile(
		//���������� ������������� � ������ �������
		hMapFile,
		// ���������� ������/������(����� �������)
		FILE_MAP_ALL_ACCESS,
		//������� ����� �������� �����, ��� ���������� �����������
		0,
		//������� ����� �������� �����, ��� ���������� �����������
		0,
		//����� ������������ ������ �����	
		BUF_SIZE);
	
	if (pBuf == NULL)
	{
		printf("������������� ��������������� ����� ���������� (%d).\n",
			GetLastError());
		return;
	}

	int i = 0;
	while (true)
	{
		i = rand();
		itoa(i, (char *)szMsg, 10);
		WaitForSingleObject(mutex, INFINITE);
		CopyMemory((PVOID)pBuf, szMsg, sizeof(szMsg));
		printf("write message: %s\n", (char *)pBuf);
		Sleep(1000); //���������� ������ ��� ������� - ��� �������� ������������� � �������
		//�����������
		ReleaseMutex(mutex);
	}
	// ������������ ������ � �������� ��������� handle
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);
	CloseHandle(mutex);
}