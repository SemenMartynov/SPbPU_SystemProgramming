#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <list>

#include "Logger.h"

#include <time.h>
#include <stdio.h>
#include <strsafe.h>

#pragma comment(lib, "Ws2_32.lib")

struct CLIENT_INFO
{
	SOCKET hClientSocket;				// Сокет
	struct sockaddr_in clientAddr;
};

_TCHAR szServerIPAddr[] = _T("127.0.0.1");// server IP
int nServerPort = 5050;					// server port

std::list<CLIENT_INFO *> clients;		// Все клиенты
CRITICAL_SECTION csСlients;				// Защита списка

bool InitWinSock2_0();
BOOL WINAPI ClientThread(LPVOID lpData);
void sendToAll(_TCHAR *pBuffer);

//Init log
Logger mylog(_T("NetReaderWriterServer"));

int _tmain(int argc, _TCHAR* argv[]) {
	if (!InitWinSock2_0()) {
		double errorcode = WSAGetLastError();
		mylog.loudlog(_T("Unable to Initialize Windows Socket environment, GLE=%d"), errorcode);
		exit(1);
	}
	mylog.loudlog(_T("Windows Socket environment ready"));

	SOCKET hServerSocket;
	hServerSocket = socket(
		AF_INET,        // The address family. AF_INET specifies TCP/IP
		SOCK_STREAM,    // Protocol type. SOCK_STREM specified TCP
		0               // Protoco Name. Should be 0 for AF_INET address family
		);

	if (hServerSocket == INVALID_SOCKET) {
		mylog.loudlog(_T("Unable to create Server socket"));
		// Cleanup the environment initialized by WSAStartup()
		WSACleanup();
		exit(2);
	}
	mylog.loudlog(_T("Server socket created"));

	// Create the structure describing various Server parameters
	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;     // The address family. MUST be AF_INET
	size_t   convtd;
	char *pMBBuffer = new char[20];
	wcstombs_s(&convtd, pMBBuffer, 20, szServerIPAddr, 20);
	//serverAddr.sin_addr.s_addr = inet_addr(pMBBuffer);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	delete[] pMBBuffer;
	serverAddr.sin_port = htons(nServerPort);

	// Bind the Server socket to the address & port
	if (bind(hServerSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		mylog.loudlog(_T("Unable to bind to %s on port %d"), szServerIPAddr, nServerPort);
		// Free the socket and cleanup the environment initialized by WSAStartup()
		closesocket(hServerSocket);
		WSACleanup();
		exit(3);
	}
	mylog.loudlog(_T("Bind completed"));

	// Put the Server socket in listen state so that it can wait for client connections
	if (listen(hServerSocket, SOMAXCONN) == SOCKET_ERROR) {
		mylog.loudlog(_T("Unable to put server in listen state"));
		// Free the socket and cleanup the environment initialized by WSAStartup()
		closesocket(hServerSocket);
		WSACleanup();
		exit(4);
	}
	mylog.loudlog(_T("Ready for connection on %s:%d"), szServerIPAddr, nServerPort);

	HANDLE hClientThread[2];
	DWORD dwThreadId[2];
	for (int i = 0; i != 2; ++i) {
		// Start the client thread
		hClientThread[i] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ClientThread,
			0, 0, &dwThreadId[i]);
		if (hClientThread[i] == NULL) {
			mylog.loudlog(_T("Unable to create client thread"));
		}
		else {
			CloseHandle(hClientThread);
		}
	}


	//инициализируем средство синхронизации
	InitializeCriticalSection(&csСlients);

	// Start the infinite loop
	while (true) {
		// As the socket is in listen mode there is a connection request pending.
		// Calling accept( ) will succeed and return the socket for the request.
		CLIENT_INFO* pClientInfo = new CLIENT_INFO;

		int nSize = sizeof(pClientInfo->clientAddr);
		pClientInfo->hClientSocket = accept(hServerSocket, (struct sockaddr *) &pClientInfo->clientAddr, &nSize);
		if (pClientInfo->hClientSocket == INVALID_SOCKET) {
			mylog.loudlog(_T("accept() failed"));
		}
		else {
			wchar_t* sin_addr = new wchar_t[20];
			size_t   convtd;
			mbstowcs_s(&convtd, sin_addr, 20, inet_ntoa(pClientInfo->clientAddr.sin_addr), 20);
			mylog.loudlog(_T("Client connected from %s:%d"), sin_addr, pClientInfo->clientAddr.sin_port);
			delete[] sin_addr;

			EnterCriticalSection(&csСlients);
			clients.push_front(pClientInfo); // Добавить нового клиента в список
			LeaveCriticalSection(&csСlients);
		}
	}

	//удаляем объект синхронизации
	DeleteCriticalSection(&csСlients);

	closesocket(hServerSocket);
	WSACleanup();
	exit(0);
}

bool InitWinSock2_0() {
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 0);

	if (!WSAStartup(wVersion, &wsaData))
		return true;

	return false;
}

BOOL WINAPI ClientThread(LPVOID lpData) {
	// Chat loop:
	while (1) {
		_TCHAR szBuffer[124];
		swprintf_s(szBuffer, _T("%d"), GetCurrentThreadId() % rand());
		mylog.loudlog(_T("Server %d: %s"), GetCurrentThreadId(), szBuffer);
		sendToAll(szBuffer);
		Sleep(100);
	}

	return 0;
}

void sendToAll(_TCHAR *pBuffer) {
	// Пока мы обрабатываем список, его ни кто не должен менять!
	EnterCriticalSection(&csСlients);
	std::list<CLIENT_INFO *>::iterator client;
	for (client = clients.begin(); client != clients.end(); ++client) {
		int nLength = (lstrlen(pBuffer) + 1) * sizeof(_TCHAR);
		int nCntSend = 0;

		while ((nCntSend = send((*client)->hClientSocket, (char *)pBuffer, nLength, 0) != nLength)) {
			if (nCntSend == -1) {
				mylog.loudlog(_T("Error sending the client"));
				break;
			}
			if (nCntSend == nLength)
				break;

			pBuffer += nCntSend;
			nLength -= nCntSend;
		}
	}
	LeaveCriticalSection(&csСlients);
}