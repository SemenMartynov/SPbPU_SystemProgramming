#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

//port number to listen
#define SERVER_PORT 7710

enum
{
	SEND_DONE = 0,
	RECV_DONE = 1,
};

typedef struct _USER_DATA
{
	SOCKET sock;
	//more to go
} USER_DATA;

typedef struct _USER_IO
{
	WSAOVERLAPPED ov; //should be first in the struct
	WSABUF buf;
	DWORD optype;
	DWORD flags;
	DWORD bytes;
} USER_IO;

void WorkingThread(HANDLE iocp)
{
	DWORD Bytes;
	USER_DATA * user;
	USER_IO * io;
	while (1)
	{
		if (!GetQueuedCompletionStatus(
			// хендл порта, к пулу которого следует подключиться
			iocp,
			// количество переданных байт
			&Bytes,
			// указатель на ключ завершения
			(PULONG_PTR)&user,
			// указатель на OVERLAPPED, ассоциированную с IO-транзакцией
			(LPOVERLAPPED *)&io,
			// время, на которое поток может уснуть
			INFINITE))
			//error
			break;

		if (!Bytes)
		{
			//descriptor closed
			printf("Disconnected user %d from server.\n", user->sock);
			shutdown(user->sock, SD_BOTH);
			closesocket(user->sock);
			continue;
		}

		switch (io->optype)
		{
		case(SEND_DONE) :
			//just requesting new recv
			WSARecv(user->sock, &io->buf, 1, &io->bytes, &io->flags, &io->ov, 0);
			io->optype = RECV_DONE;
			printf("Welcome to user %d sent.\n", user->sock);
			continue;
		case(RECV_DONE) :
			//	here can be done processing or sheduling
			printf("Got packet from client %d, length %d.\n", user->sock, Bytes);
			WSARecv(user->sock, &io->buf, 1, &io->bytes, &io->flags, &io->ov, 0);
			continue;
		}
	}

	printf("Error %d.\n", GetLastError());
};

int main(int argc, char * argv[])
{
	WSADATA wsadata;
	SOCKADDR_IN listenaddr;
	SOCKET listensocket;
	HANDLE iocp;
	int i;

	WSAStartup(0x0202, &wsadata);

	//making listening socket overlapped
	listensocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	listenaddr.sin_family = AF_INET;
	listenaddr.sin_port = htons(SERVER_PORT);
	listenaddr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listensocket, (SOCKADDR *)&listenaddr, sizeof(listenaddr));
	listen(listensocket, 5);

	// создать новый объект порта завершения
	iocp = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE, // аргумент означает новый порт
		0, 0, // всегда 0
		0); // кол-во потоков для порта одновременно (по умолчанию кол-во CPU)
	if (!iocp)
	{
		printf("Can't create IOCP: error %d", GetLastError());
		return 0;
	}

	// Кол-во потоков в пуле
	for (i = 1; i <= 2; ++i)
	{
		HANDLE thr = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&WorkingThread, (LPVOID)iocp, 0, 0);
		CloseHandle(thr);
	}

	// Клиент
	while (1)
	{
		SOCKET clientsocket;
		SOCKADDR clientaddr;
		USER_DATA * user;
		USER_IO * io;
		int clientsize = sizeof(clientaddr);

		user = (USER_DATA *)malloc(sizeof(USER_DATA));
		io = (USER_IO *)malloc(sizeof(USER_IO));
		memset(user, 0, sizeof(USER_DATA));
		memset(io, 0, sizeof(USER_IO));

		// возвращает нам сокет клиента, в который можно писать и читать
		clientsocket = WSAAccept(listensocket, (SOCKADDR *)&clientaddr, &clientsize, 0, 0);
		printf("Accepted new client %d.\n", clientsocket);

		user->sock = clientsocket;
		io->buf.buf = (char *)malloc(1024);
		strcpy_s(io->buf.buf, 19, "You are connected!");

		io->buf.len = 19;
		io->optype = SEND_DONE;

		// связываем дескрипторы сокета и порта
		CreateIoCompletionPort((HANDLE)clientsocket, iocp, (ULONG_PTR)user, 0);

		//sending welcome message to the client
		WSASend(user->sock, &io->buf, 1, &io->bytes, 0, (LPOVERLAPPED)io, 0);
	}
};