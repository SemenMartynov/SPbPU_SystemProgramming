//http://msdn.microsoft.com/ru-ru/windows/desktop/aa365802%28v=vs.85%29

#include <windows.h> 
#include <stdio.h> 
#include <conio.h>
#include <tchar.h>
#include <strsafe.h>

#define BUFSIZE 512

DWORD WINAPI InstanceThread(LPVOID);

int _tmain(int argc, TCHAR *argv[])
{
	_tprintf(TEXT("Server is started.\n\n"));

	BOOL   fConnected = FALSE; // ���� ������� ������������ ��������
	DWORD  dwThreadId = 0; // ����� �������������� ������
	HANDLE hPipe = INVALID_HANDLE_VALUE; // ������������� ������
	HANDLE hThread = NULL; // ������������� �������������� ������
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\$$MyPipe$$"); // ��� ������������ ������ Pipe

	// ���� ������� �������� � ������ ��� ��� ������ ���������
	for (;;)
	{
		_tprintf(TEXT("Try to create named pipe on %s\n"), lpszPipename);

		// ������� �����:
		hPipe = CreateNamedPipe(
			// ��� ������,
			lpszPipename,
			// ����� ������� ������ - ���������������,
			PIPE_ACCESS_DUPLEX,
			// ������ ������������ � ����� � ���� ������ ���������,
			PIPE_TYPE_MESSAGE |
			// ������ ����������� � ���� ������ ���������,
			PIPE_READMODE_MESSAGE |
			// ������� �������� � ������ ����������� �� �� ���������,
			PIPE_WAIT,
			// ������������ ����� ����������� ������� �� ����������,
			PIPE_UNLIMITED_INSTANCES,
			//������� ��������� � �������� ������� ������,
			BUFSIZE,
			BUFSIZE,
			// 5 ������ - ������������ ��� ������� WaitNamedPipe,
			5000,
			// ���������� ������������ �� ���������.
			NULL);

		// ���� �������� ������, ������� �� ��� � ���������	������ ����������
		if (hPipe == INVALID_HANDLE_VALUE)
		{
			_tprintf(TEXT("CreateNamedPipe failed, GLE=%d.\n"), GetLastError());
			_getch();
			return -1;
		}
		_tprintf(TEXT("Named pipe created successfully!\n\n"));

		// ������� ���������� �� ������� �������
		_tprintf(TEXT("Waiting for connect...\n"));
		fConnected = ConnectNamedPipe(hPipe, NULL) ?
			TRUE :
			(GetLastError() == ERROR_PIPE_CONNECTED);

		// ���� ��������� ����������
		if (fConnected)
		{
			_tprintf(TEXT("Client connected!\n\nCreating a processing thread...\n"));

			// ������ ����� ��� ������������ ������� 
			hThread = CreateThread(
				NULL,              // ���������� ������ 
				0,                 // ��������� ������ �����
				InstanceThread,    // ������� ������
				(LPVOID)hPipe,     // �������� ������
				0,                 // ����� ��������
				&dwThreadId);      // ����� ������

			// ���� ����� ������� �� ������� - �������� �� ������
			if (hThread == NULL)
			{
				_tprintf(TEXT("CreateThread failed, GLE=%d.\n"), GetLastError());
				_getch();
				return -1;
			}
			else CloseHandle(hThread);
		}
		else
			// ���� ������� �� ������� ������������, ��������� �����
			CloseHandle(hPipe);
	}

	return 0;
}

DWORD WINAPI InstanceThread(LPVOID lpvParam)
{
	_tprintf(TEXT("Thread started!\n"));
	HANDLE hPipe = (HANDLE)lpvParam; // ������������� ������
	// ����� ��� �������� ����������� � ������������� ���������
	TCHAR* chBuf = (TCHAR*)HeapAlloc(GetProcessHeap(), 0, BUFSIZE * sizeof(TCHAR));
	DWORD readbytes, writebytes; // ����� ���� ����������� � ����������

	while (1)
	{
		// �������� ��������� ������� ����� ����� Pipe
		if (ReadFile(hPipe, chBuf, BUFSIZE*sizeof(TCHAR), &readbytes, NULL))
		{
			// �������� ��� ������� ������� ����������� ����������
			if (!WriteFile(hPipe, chBuf, (lstrlen(chBuf) + 1)*sizeof(TCHAR), &writebytes, NULL))
				break;
			// ������� �������� ������� �� �������
			_tprintf(TEXT("Get client msg: %s\n"), chBuf);
			// ���� ������ ������� "exit", ��������� ������ ����������
			if (!_tcsncmp(chBuf, L"exit", 4))
				break;
		}
		else
		{
			_tprintf(TEXT("ReadFile: Error %ld\n"), GetLastError());
			_getch();
			break;
		}
	}

	// ������������ �������� 
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	HeapFree(hPipe, 0, chBuf);

	_tprintf(TEXT("InstanceThread exitting.\n"));
	return 1;
}
