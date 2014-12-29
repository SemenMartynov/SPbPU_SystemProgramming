#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <strsafe.h>

#define BUFSIZE 512

int _tmain(int argc, TCHAR *argv[])
{
	_tprintf(TEXT("Client is started!\n\n"));

	HANDLE hPipe = INVALID_HANDLE_VALUE; // ������������� ������
	LPTSTR lpszPipename = TEXT("\\\\192.168.124.2\\pipe\\$$MyPipe$$"); // ��� ������������ ������ Pipe
	TCHAR chBuf[BUFSIZE]; // ����� ��� �������� ������ ����� �����
	DWORD readbytes, writebytes; // ����� ���� ����������� � ����������

	// �������� SECURITY_ATTRIBUTES � SECURITY_DESCRIPTOR ��������
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	// ������������� SECURITY_DESCRIPTOR ���������� ��-���������
	if (InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) == 0)
	{
		_tprintf(TEXT("InitializeSecurityDescriptor failed with error %d\n"), GetLastError());
		_getch();
		return -1;
	}
	// ��������� ���� DACL � SECURITY_DESCRIPTOR � NULL
	if (SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE) == 0)
	{
		_tprintf(TEXT("SetSecurityDescriptorDacl failed with error %d\n"), GetLastError());
		_getch();
		return -1;
	}
	// ��������� SECURITY_DESCRIPTOR � ��������� SECURITY_ATTRIBUTES
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = FALSE; //���������� ������������

	_tprintf(TEXT("Try to use WaitNamedPipe...\n"));
	// �������� ������� ����������� �����, ���� ���� - ������� ��� ������������
	while (1)
	{
		// ������� ����� � ���������-��������:
		hPipe = CreateFile(
			lpszPipename, // ��� ������,
			GENERIC_READ // ������� ������ ����� ������ �� ������,
			| GENERIC_WRITE, // ������� ������ ����� ������ �� ������,
			0, // ��� �������,
			&sa, // �������� ������,
			OPEN_EXISTING, // ����������� ������������ ����,
			0, // �������� � ����� ��� �����,
			NULL); // ������� � ����� �������.

		// ���������� ������, ���� ����� ������� ������� 
		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		// �����, ���� ������ ������� �� � ������� �������. 
		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
			_getch();
			return -1;
		}

		// ���� ��� ������ ������, ��� 20 ������ 
		if (!WaitNamedPipe(lpszPipename, 20000))
		{
			_tprintf(TEXT("Could not open pipe: 20 second wait timed out."));
			_getch();
			return -1;
		}
	}

	// ������� ��������� � �������� ������
	_tprintf(TEXT("Successfully connected!\n\nInput message...\n"));
	// ���� ������ ������� � ��������� ���������
	while (1)
	{
		// ������� ����������� ��� ����� �������
		_tprintf(TEXT("cmd>"));
		// ������ ��������� ������
		_fgetts(chBuf, BUFSIZE, stdin);
		// �������� ��������� ������ ���������� �������� � �������� �������
		if (!WriteFile(hPipe, chBuf, (lstrlen(chBuf) + 1)*sizeof(TCHAR), &writebytes, NULL))
		{
			_tprintf(TEXT("connection refused\n"));
			break;
		}
		// �������� ��� �� ������� ������� �� �������
		if (ReadFile(hPipe, chBuf, BUFSIZE*sizeof(TCHAR), &readbytes, NULL))
			_tprintf(TEXT("Received from server: %s\n"), chBuf);
		// ���� ��������� ������, ������� �� ��� � ��������� ������ ����������
		else {
			_tprintf(TEXT("ReadFile: Error %ld\n"), GetLastError());
			_getch();
			break;
		}
		// � ����� �� ������� "exit" ��������� ���� ������ ������� � ��������� ���������
		if (!_tcsncmp(chBuf, L"exit", 4))
			break;
	}

	// ��������� ������������� ������
	CloseHandle(hPipe);

	_tprintf(TEXT("Press ENTER to terminate connection and exit\n"));
	_getch();
	return 0;
}