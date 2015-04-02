#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include "Logger.h"

#pragma comment(lib, "Ws2_32.lib")

_TCHAR szServerIPAddr[20];			// server IP
int nServerPort;					// server port

bool InitWinSock2_0();

//Init log
Logger mylog(_T("NetReaderWriterClient"), GetCurrentProcessId());

int _tmain(int argc, _TCHAR* argv[]) {
	_tprintf(_T("Enter the server IP Address: "));
	wscanf_s(_T("%19s"), szServerIPAddr, _countof(szServerIPAddr));
	_tprintf(_T("Enter the server port number: "));
	wscanf_s(_T("%i"), &nServerPort);

	if (!InitWinSock2_0()) {
		double errorcode = WSAGetLastError();
		mylog.loudlog(_T("Unable to Initialize Windows Socket environment, GLE=%d"), errorcode);
		exit(1);
	}
	mylog.quietlog(_T("Windows Socket environment ready"));

	SOCKET hClientSocket;
	hClientSocket = socket(
		AF_INET,        // The address family. AF_INET specifies TCP/IP
		SOCK_STREAM,    // Protocol type. SOCK_STREM specified TCP
		0);             // Protoco Name. Should be 0 for AF_INET address family

	if (hClientSocket == INVALID_SOCKET) {
		mylog.loudlog(_T("Unable to create socket"));
		// Cleanup the environment initialized by WSAStartup()
		WSACleanup();
		exit(2);
	}
	mylog.quietlog(_T("Client socket created"));

	// Create the structure describing various Server parameters
	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;     // The address family. MUST be AF_INET
	size_t   convtd;
	char *pMBBuffer = new char[20];
	wcstombs_s(&convtd, pMBBuffer, 20, szServerIPAddr, 20);
	serverAddr.sin_addr.s_addr = inet_addr(pMBBuffer);
	delete[] pMBBuffer;
	serverAddr.sin_port = htons(nServerPort);

	// Connect to the server
	if (connect(hClientSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
		mylog.loudlog(_T("Unable to connect to %s on port %d"), szServerIPAddr, nServerPort);
		closesocket(hClientSocket);
		WSACleanup();
		exit(3);
	}
	mylog.quietlog(_T("Connect"));

	// Main loop:
	while (1) {
		_TCHAR szBuffer[1024];
		int nLength = recv(hClientSocket, (char *)szBuffer, sizeof(szBuffer), 0);

		if (nLength > 0) {
			szBuffer[nLength] = '\0';
			mylog.loudlog(_T("%s"), szBuffer);
			_tprintf(_T(">> "));
		}
	}

	closesocket(hClientSocket);
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
