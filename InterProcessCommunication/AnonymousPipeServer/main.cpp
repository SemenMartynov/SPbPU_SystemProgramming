#include <windows.h>
#include <stdio.h>
int main(int argc, char* argv[])
{
	//дескрипторы канала для передачи от сервера клиенту
	HANDLE hReadPipeFromServToClient, hWritePipeFromServToClient;
	//дескрипторы канала для передачи от сервера клиенту
	HANDLE hReadPipeFromClientToServ, hWritePipeFromClientToServ;

	//чтобы сделать дескрипторы наследуемыми
	//создаем канал для передачи от сервера клиенту, сразу делаем дескрипторы наследуемыми
	SECURITY_ATTRIBUTES PipeSA = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	if (CreatePipe(&hReadPipeFromServToClient, &hWritePipeFromServToClient, &PipeSA, 0) == 0)
	{
		printf("impossible to create anonymous pipe from serv to client\n");
		getchar();
		return 1000;
	}
	//создаем канал для передачи от клиента серверу, сразу делаем дескрипторы наследуемыми
	if (CreatePipe(&hReadPipeFromClientToServ, &hWritePipeFromClientToServ, &PipeSA, 0) == 0)
	{
		printf("impossible to create anonymous pipe from client to serv\n");
		getchar();
		return 1001;
	}

	PROCESS_INFORMATION processInfo_Client; // информация о процессе-клиенте
	STARTUPINFO startupInfo_Client;
	//структура, которая описывает внешний вид основного
	//окна и содержит дескрипторы стандартных устройств нового процесса, используем для установки
	//процесс-клиент будет иметь те же параметры запуска, что и сервер, за исключением
	//дескрипторов ввода, вывода и ошибок
	GetStartupInfo(&startupInfo_Client);
	startupInfo_Client.hStdInput = hReadPipeFromServToClient;
	//устанавливаем поток ввода
	startupInfo_Client.hStdOutput = hWritePipeFromClientToServ;
	//установим поток вывода
	startupInfo_Client.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	//установим поток ошибок
	startupInfo_Client.dwFlags = STARTF_USESTDHANDLES; //устанавливаем наследование
	//создаем процесс клиента
	CreateProcess(NULL, L"..\\..\\anonymousPipesSonProc\\Release\\anonymousPipesSonProc.exe", NULL,
		NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL,
		&startupInfo_Client, &processInfo_Client);
	CloseHandle(processInfo_Client.hThread); //закрываем дескрипторы созданного процесса и его
	//потока
	CloseHandle(processInfo_Client.hProcess);
	//закрываем ненужные дескрипторы каналов, которые не использует сервер
	CloseHandle(hReadPipeFromServToClient);
	CloseHandle(hWritePipeFromClientToServ);
#define BUF_SIZE 100
	//размер буфера для сообщений
	BYTE buf[BUF_SIZE];
	//буфер приема/передачи
	DWORD readbytes, writebytes; //число прочитанных/переданных байт
	for (int i = 0; i < 10; i++)
	{
		//читаем данные из канала от клиента
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
		//пишем данные в канал клиенту
	}
	//закрываем HANDLE каналов
	CloseHandle(hReadPipeFromClientToServ);
	CloseHandle(hWritePipeFromServToClient);
	printf("server ended work\n Press any key");
	getchar();
	return 0;
}