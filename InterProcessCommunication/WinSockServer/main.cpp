#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include "logger.h"

#pragma comment(lib, "Ws2_32.lib")

struct CLIENT_INFO
{
	SOCKET hClientSocket;
	struct sockaddr_in clientAddr;
};

_TCHAR szServerIPAddr[] = _T("127.0.0.1"); // server IP
int nServerPort = 5050;  // server port
// clients to talk with the server

bool InitWinSock2_0();
BOOL WINAPI ClientThread(LPVOID lpData);

int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);

	if (!InitWinSock2_0()) {
		double errorcode = WSAGetLastError();
		writelog(_T("Unable to Initialize Windows Socket environment, GLE=%d"), errorcode);
		_tprintf(_T("Unable to Initialize Windows Socket environment, GLE=%d"), errorcode);
		closelog();
		exit(1);
	}
	writelog(_T("Windows Socket environment ready"));

	SOCKET hServerSocket;
	hServerSocket = socket(
		AF_INET,        // The address family. AF_INET specifies TCP/IP
		SOCK_STREAM,    // Protocol type. SOCK_STREM specified TCP
		0               // Protoco Name. Should be 0 for AF_INET address family
		);

	if (hServerSocket == INVALID_SOCKET) {
		writelog(_T("Unable to create Server socket"));
		_tprintf(_T("Unable to create Server socket"));
		// Cleanup the environment initialized by WSAStartup()
		WSACleanup();
		closelog();
		exit(2);
	}
	writelog(_T("Server socket created"));

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
		writelog(_T("Unable to bind to %s on port %d"), szServerIPAddr, nServerPort);
		_tprintf(_T("Unable to bind to %s on port %d"), szServerIPAddr, nServerPort);
		// Free the socket and cleanup the environment initialized by WSAStartup()
		closesocket(hServerSocket);
		WSACleanup();
		closelog();
		exit(3);
	}
	writelog(_T("Bind"));

	// Put the Server socket in listen state so that it can wait for client connections
	if (listen(hServerSocket, SOMAXCONN) == SOCKET_ERROR) {
		writelog(_T("Unable to put server in listen state"));
		_tprintf(_T("Unable to put server in listen state"));
		// Free the socket and cleanup the environment initialized by WSAStartup()
		closesocket(hServerSocket);
		WSACleanup();
		closelog();
		exit(4);
	}
	writelog(_T("Ready for connection"));
	_tprintf(_T("Ready for connection\n"));

	// Start the infinite loop
	while (true) {
		// As the socket is in listen mode there is a connection request pending.
		// Calling accept( ) will succeed and return the socket for the request.
		CLIENT_INFO *pClientInfo = new CLIENT_INFO;
		int nSize = sizeof(pClientInfo->clientAddr);

		pClientInfo->hClientSocket = accept(hServerSocket, (struct sockaddr *) &pClientInfo->clientAddr, &nSize);
		if (pClientInfo->hClientSocket == INVALID_SOCKET) {
			writelog(_T("accept() failed"));
			_tprintf(_T("accept() failed\n"));
		}
		else {
			HANDLE hClientThread;
			DWORD dwThreadId;

			wchar_t* sin_addr = new wchar_t[20];
			size_t   convtd;
			mbstowcs_s(&convtd, sin_addr, 20, inet_ntoa(pClientInfo->clientAddr.sin_addr), 20);
			writelog(_T("Client connected from %s:%d"), sin_addr, pClientInfo->clientAddr.sin_port);
			_tprintf(_T("Client connected from %s:%d\n"), sin_addr, pClientInfo->clientAddr.sin_port);
			delete[] sin_addr;

			// Start the client thread
			hClientThread = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)ClientThread,
				(LPVOID)pClientInfo, 0, &dwThreadId);
			if (hClientThread == NULL) {
				writelog(_T("Unable to create client thread"));
				_tprintf(_T("Unable to create client thread\n"));
			}
			else {
				CloseHandle(hClientThread);
			}
		}
	}

	closesocket(hServerSocket);
	WSACleanup();
	closelog();
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
	CLIENT_INFO *pClientInfo = (CLIENT_INFO *)lpData;
	_TCHAR szBuffer[1024];
	int nLength;

	while (1) {
		nLength = recv(pClientInfo->hClientSocket, (char *)szBuffer, sizeof(szBuffer), 0);
		wchar_t* sin_addr = new wchar_t[20];
		size_t   convtd;
		mbstowcs_s(&convtd, sin_addr, 20, inet_ntoa(pClientInfo->clientAddr.sin_addr), 20);
		if (nLength > 0) {
			szBuffer[nLength] = '\0';
			writelog(_T("Received %s from %s:%d"), szBuffer, sin_addr, pClientInfo->clientAddr.sin_port);
			_tprintf(_T("Received %s from %s:%d\n"), szBuffer, sin_addr, pClientInfo->clientAddr.sin_port);

			// Convert the string to upper case and send it back, if its not QUIT
			//_wcsdup(szBuffer);
			if (wcscmp(szBuffer, _T("QUIT")) == 0) {
				closesocket(pClientInfo->hClientSocket);
				delete pClientInfo;
				return TRUE;
			}
			// send() may not be able to send the complete data in one go.
			// So try sending the data in multiple requests
			int nCntSend = 0;
			_TCHAR *pBuffer = szBuffer;

			while ((nCntSend = send(pClientInfo->hClientSocket, (char *)pBuffer, nLength, 0) != nLength)) {
				if (nCntSend == -1) {
					writelog(_T("Error sending the data to %s:%d"), sin_addr, pClientInfo->clientAddr.sin_port);
					_tprintf(_T("Error sending the data to %s:%d\n"), sin_addr, pClientInfo->clientAddr.sin_port);
					break;
				}
				if (nCntSend == nLength)
					break;

				pBuffer += nCntSend;
				nLength -= nCntSend;
			}
		}
		else {
			writelog(_T("Error reading the data from %s:%d"), sin_addr, pClientInfo->clientAddr.sin_port);
			_tprintf(_T("Error reading the data from %s:%d\n"), sin_addr, pClientInfo->clientAddr.sin_port);
		}
		delete[] sin_addr;
	}

	return TRUE;
}
