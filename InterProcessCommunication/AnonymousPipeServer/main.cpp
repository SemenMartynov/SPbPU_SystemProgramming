#include <windows.h>
#include <stdio.h>
int main(int argc, char* argv[])
{
	//����������� ������ ��� �������� �� ������� �������
	HANDLE hReadPipeFromServToClient, hWritePipeFromServToClient;
	//����������� ������ ��� �������� �� ������� �������
	HANDLE hReadPipeFromClientToServ, hWritePipeFromClientToServ;

	//����� ������� ����������� ������������
	//������� ����� ��� �������� �� ������� �������, ����� ������ ����������� ������������
	SECURITY_ATTRIBUTES PipeSA = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	if (CreatePipe(&hReadPipeFromServToClient, &hWritePipeFromServToClient, &PipeSA, 0) == 0)
	{
		printf("impossible to create anonymous pipe from serv to client\n");
		getchar();
		return 1000;
	}
	//������� ����� ��� �������� �� ������� �������, ����� ������ ����������� ������������
	if (CreatePipe(&hReadPipeFromClientToServ, &hWritePipeFromClientToServ, &PipeSA, 0) == 0)
	{
		printf("impossible to create anonymous pipe from client to serv\n");
		getchar();
		return 1001;
	}

	PROCESS_INFORMATION processInfo_Client; // ���������� � ��������-�������
	//���������, ������� ��������� ������� ��� ���������
	//���� � �������� ����������� ����������� ��������� ������ ��������, ���������� ��� ���������
	STARTUPINFOA startupInfo_Client;
	
	//�������-������ ����� ����� �� �� ��������� �������, ��� � ������, �� �����������
	//������������ �����, ������ � ������
	GetStartupInfoA(&startupInfo_Client);
	startupInfo_Client.hStdInput = hReadPipeFromServToClient;
	//������������� ����� �����
	startupInfo_Client.hStdOutput = hWritePipeFromClientToServ;
	//��������� ����� ������
	startupInfo_Client.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	//��������� ����� ������
	startupInfo_Client.dwFlags = STARTF_USESTDHANDLES; //������������� ������������
	//������� ������� �������
	CreateProcessA(NULL, "AnonymousPipeClient.exe", NULL,
		NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL,
		&startupInfo_Client, &processInfo_Client);
	//��������� ����������� ���������� �������� � ��� ������
	CloseHandle(processInfo_Client.hThread);
	CloseHandle(processInfo_Client.hProcess);
	//��������� �������� ����������� �������, ������� �� ���������� ������
	CloseHandle(hReadPipeFromServToClient);
	CloseHandle(hWritePipeFromClientToServ);
#define BUF_SIZE 100
	//������ ������ ��� ���������
	BYTE buf[BUF_SIZE];
	//����� ������/��������
	DWORD readbytes, writebytes; //����� �����������/���������� ����
	for (int i = 0; i < 10; i++)
	{
		//������ ������ �� ������ �� �������
		if (!ReadFile(hReadPipeFromClientToServ, buf, BUF_SIZE, &readbytes, NULL))
		{
			printf("impossible to use readfile\n GetLastError= %d\n", GetLastError());
			getchar();
			return 10000;
		}
		printf("get from client: \"%s\"\n", buf);
		if (!WriteFile(hWritePipeFromServToClient, buf, readbytes, &writebytes, NULL))
		{
			printf("impossible to use writefile\n GetLastError= %d\n", GetLastError());
			getchar();
			return 10001;
		}
		//����� ������ � ����� �������
	}
	//��������� HANDLE �������
	CloseHandle(hReadPipeFromClientToServ);
	CloseHandle(hWritePipeFromServToClient);
	printf("server ended work\n Press any key");
	getchar();
	return 0;
}