#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include "logger.h"

#pragma comment(lib, "Ws2_32.lib")

_TCHAR szServerIPAddr[] = _T("127.0.0.1"); // server IP
int nServerPort = 5050;  // server port
// clients to talk with the server

#define DATA_BUFSIZE	1024
#define EMPTY_MSG		_T("...")

typedef struct{
	OVERLAPPED Overlapped;
	////////////////////
	WSABUF DataBuf;
	CHAR Buffer[DATA_BUFSIZE];
	DWORD BytesSend;
	DWORD BytesRecv;
	DWORD TotalBytes;
	SOCKADDR_IN client;
} PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;


typedef struct{
	SOCKET Socket;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;


DWORD WINAPI ClientThread(LPVOID CompletionPortID);

int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);

	SOCKADDR_IN server;
	SOCKADDR_IN client;

	SOCKET Socket;
	SOCKET Accept;

	HANDLE CompletionPort;
	SYSTEM_INFO SysInfo;
	HANDLE Thread;

	LPPER_HANDLE_DATA PerHandleData;
	LPPER_IO_OPERATION_DATA PerIoData;

	DWORD SendBytes;
	DWORD Flags;
	DWORD ThreadID;
	WSADATA wsaData;

	// инициализируем WinSock:
	if ((WSAStartup(0x0202, &wsaData)) != 0){
		double errorcode = WSAGetLastError();
		writelog(_T("Unable to Initialize Windows Socket environment, GLE=%d"), errorcode);
		_tprintf(_T("Unable to Initialize Windows Socket environment, GLE=%d"), errorcode);
		closelog();
		exit(1);
	}
	writelog(_T("Windows Socket environment ready"));

	// Создаём порт завершения:
	if ((CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL){
		double errorcode = WSAGetLastError();
		writelog(_T("CreateIoCompletionPort failed, GLE=%d"), errorcode);
		_tprintf(_T("CreateIoCompletionPort failed, GLE=%d"), errorcode);
		closelog();
		exit(2);
	}
	writelog(_T("Io Completion Port created"));

	// Получаем информацию о системы:
	GetSystemInfo(&SysInfo);
	// создаём два потока на процессор:.
	for (size_t i = 0; i < SysInfo.dwNumberOfProcessors * 2; i++){
		// создаём рабочий поток, в качестве параметра передаём ей порт завершения
		if ((Thread = CreateThread(NULL, 0, ClientThread, CompletionPort, 0, &ThreadID)) == NULL) {
			double errorcode = WSAGetLastError();
			writelog(_T("CreateThread() failed, GLE=%d"), errorcode);
			_tprintf(_T("CreateThread() failed, GLE=%d"), errorcode);
			closelog();
			exit(3);
		}
		CloseHandle(Thread);
	}

	// Создаём слушающий сокет:
	if ((Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		double errorcode = WSAGetLastError();
		writelog(_T("WSASocket() failed, GLE=%d"), errorcode);
		_tprintf(_T("WSASocket() failed, GLE=%d"), errorcode);
		closelog();
		exit(4);
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(nServerPort);

	if (bind(Socket, (PSOCKADDR)&server, sizeof(server)) == SOCKET_ERROR){
		double errorcode = WSAGetLastError();
		writelog(_T("bind() failed, GLE=%d"), errorcode);
		_tprintf(_T("bind() failed, GLE=%d"), errorcode);
		closelog();
		exit(5);
	}

	if (listen(Socket, 5) == SOCKET_ERROR){
		double errorcode = WSAGetLastError();
		writelog(_T("listen() failed, GLE=%d"), errorcode);
		_tprintf(_T("listen() failed, GLE=%d"), errorcode);
		closelog();
		exit(6);
	}
	writelog(_T("Ready for connection"));
	_tprintf(_T("Ready for connection\n"));

	// принимаем соединения и передаём их порту завершения:
	while (TRUE){
		// принимаем соединение:
		if ((Accept = WSAAccept(Socket, (PSOCKADDR)&client, NULL, NULL, 0)) == SOCKET_ERROR){
			double errorcode = WSAGetLastError();
			writelog(_T("WSAAccept() failed, GLE=%d"), errorcode);
			_tprintf(_T("WSAAccept() failed, GLE=%d"), errorcode);
			continue;
		}

		// Выделяем память под структуру, которая будет хранить информацию о сокете:
		if ((PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA))) == NULL){
			double errorcode = WSAGetLastError();
			writelog(_T("GlobalAlloc() failed with error %d"), errorcode);
			_tprintf(_T("GlobalAlloc() failed with error %d"), errorcode);
			closelog();
			exit(7);
		}
		writelog(_T("Socket %d connected\n"), Accept);
		_tprintf(_T("Socket %d connected\n"), Accept);
		
		PerHandleData->Socket = Accept; // сохраняем описатель сокета

		//привязываем сокет к порту завершения:
		if (CreateIoCompletionPort((HANDLE)Accept, CompletionPort, (DWORD)PerHandleData, 0) == NULL){
			double errorcode = WSAGetLastError();
			writelog(_T("CreateIoCompletionPort() failed with error %d"), errorcode);
			_tprintf(_T("CreateIoCompletionPort() failed with error %d"), errorcode);
			closelog();
			exit(8);
		}

		// выделяем память под данные операции ввода вывода:
		if ((PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATION_DATA))) == NULL){
			double errorcode = WSAGetLastError();
			writelog(_T("GlobalAlloc() failed with error %d"), errorcode);
			_tprintf(_T("GlobalAlloc() failed with error %d"), errorcode);
			closelog();
			exit(9);
		}

		ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED));

		// задаём изначальные данные для операции ввода вывода:
		PerIoData->BytesSend = 0;
		PerIoData->BytesRecv = 0;
		PerIoData->DataBuf.len = (wcslen(EMPTY_MSG) + 1) * sizeof(_TCHAR);
		PerIoData->DataBuf.buf = (char*) EMPTY_MSG;
		PerIoData->client = client;
		PerIoData->TotalBytes = 0;

		Flags = 0;
		
		// отправляем welcome message
		// остальные операции будут выполняться в рабочем потоке
		if (WSASend(Accept, &(PerIoData->DataBuf), 1, &SendBytes, 0, &(PerIoData->Overlapped), NULL) == SOCKET_ERROR){
			if (WSAGetLastError() != ERROR_IO_PENDING){
				double errorcode = WSAGetLastError();
				writelog(_T("WSASend() failed with error %d"), errorcode);
				_tprintf(_T("WSASend() failed with error %d\n"), errorcode);
				closelog();
				exit(10);
			}
		}
	}
	closelog();
	exit(0);
}

DWORD WINAPI ClientThread(LPVOID CompletionPortID) {
	HANDLE CompletionPort = (HANDLE)CompletionPortID;
	DWORD BytesTransferred;
	LPPER_HANDLE_DATA PerHandleData;
	LPPER_IO_OPERATION_DATA PerIoData;

	DWORD SendBytes, RecvBytes;
	DWORD Flags;

	while (TRUE){
		// ожидание завершения ввода-вывода на любом из сокетов
		// которые связанны с портом завершения:
		if (GetQueuedCompletionStatus(CompletionPort, &BytesTransferred,
			(LPDWORD)&PerHandleData, (LPOVERLAPPED *)&PerIoData, INFINITE) == 0){
			double errorcode = GetLastError();
			writelog(_T("WSASend() failed with error %d"), errorcode);
			_tprintf(_T("WSASend() failed with error %d\n"), errorcode);
			return 0;
		}

		// проверяем на ошибки. Если была - значит надо закрыть сокет и очистить память за собой:
		if (BytesTransferred == 0){
			// тк не было переданно ни одного байта - значит сокет закрыли на той стороне
			// мы должны сделать то же самое:
			writelog(_T("Closing socket %d"), PerHandleData->Socket);
			writelog(_T("Total bytes:%d\n"), PerIoData->TotalBytes);
			_tprintf(_T("Closing socket %d\nTotal bytes:%d\n"), PerHandleData->Socket, PerIoData->TotalBytes);

			// закрываем сокет:
			if (closesocket(PerHandleData->Socket) == SOCKET_ERROR){
				double errorcode = WSAGetLastError();
				writelog(_T("closesocket() failed with error %d"), errorcode);
				_tprintf(_T("closesocket() failed with error %d\n"), errorcode);
				return 0;
			}

			// очищаем память:
			GlobalFree(PerHandleData);
			GlobalFree(PerIoData);

			// ждём следующую операцию
			continue;
		}

		PerIoData->TotalBytes += BytesTransferred;

		// Проверим значение BytesRecv - если оно равно нулю - значит мы получили данные от клиента:
		if (PerIoData->BytesRecv == 0){
			PerIoData->BytesRecv = BytesTransferred;
			PerIoData->BytesSend = 0;
		}
		else{
			PerIoData->BytesSend += BytesTransferred;
		}

		// мы должны отослать все принятые байты назад:
		if (PerIoData->BytesRecv > PerIoData->BytesSend){
			// Шлём данные через WSASend - тк всё сразу может не отослаться 
			// необходимо слать до упора.
			// Теоретически, за один вызов WSASend все данные могут не отправится!
			ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED));

			PerIoData->DataBuf.buf = PerIoData->Buffer + PerIoData->BytesSend;
			PerIoData->DataBuf.len = PerIoData->BytesRecv - PerIoData->BytesSend;

			// Convert the string to upper case and send it back, if its not QUIT
			if (wcscmp((_TCHAR*)PerIoData->Buffer, _T("QUIT")) == 0) {
				_tprintf(_T("RCV %s\n"), PerIoData->Buffer);
				closesocket(PerHandleData->Socket);
				return TRUE;
			}
			if (WSASend(PerHandleData->Socket, &(PerIoData->DataBuf), 1, &SendBytes, 0,
				&(PerIoData->Overlapped), NULL) == SOCKET_ERROR) {
				if (WSAGetLastError() != ERROR_IO_PENDING) {
					double errorcode = WSAGetLastError();
					writelog(_T("WSASend() failed with error %d"), errorcode);
					_tprintf(_T("WSASend() failed with error %d\n"), errorcode);
					return 0;
				}
			}
		}
		else{
			PerIoData->BytesRecv = 0;

			// ожидаем ещё данные от пользователя:
			Flags = 0;
			ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED));

			PerIoData->DataBuf.len = DATA_BUFSIZE;
			PerIoData->DataBuf.buf = PerIoData->Buffer;

			if (WSARecv(PerHandleData->Socket, &(PerIoData->DataBuf), 1, &RecvBytes, &Flags,
				&(PerIoData->Overlapped), NULL) == SOCKET_ERROR){
				if (WSAGetLastError() != ERROR_IO_PENDING){
					double errorcode = WSAGetLastError();
					writelog(_T("WSARecv() failed with error %d"), errorcode);
					_tprintf(_T("WSASend() failed with error %d\n"), errorcode);
					return 0;
				}
			}
			writelog(_T("Get task from %d: %s"), PerHandleData->Socket, PerIoData->Buffer);
			_tprintf(_T("Get task from %d: %s\n"), PerHandleData->Socket, PerIoData->Buffer);
		}
	}
}
