#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include "logger.h"

#pragma comment(lib, "Ws2_32.lib")

_TCHAR szServerIPAddr[20]; // server IP
int nServerPort; // server port

bool InitWinSock2_0();

int _tmain(int argc, _TCHAR* argv[]) {
	//Init log
	initlog(argv[0]);

	_tprintf(_T("Enter the server IP Address: "));
	wscanf_s(_T("%19s"), szServerIPAddr, _countof(szServerIPAddr));
	_tprintf(_T("Enter the server port number: "));
	wscanf_s(_T("%i"), &nServerPort);

	if (!InitWinSock2_0()) {
		double errorcode = WSAGetLastError();
		writelog(_T("Unable to Initialize Windows Socket environment, GLE=%d"), errorcode);
		_tprintf(_T("Unable to Initialize Windows Socket environment, GLE=%d"), errorcode);
		closelog();
		exit(1);
	}
	writelog(_T("Windows Socket environment ready"));

	SOCKET hClientSocket;
	hClientSocket = socket(
		AF_INET,        // The address family. AF_INET specifies TCP/IP
		SOCK_STREAM,    // Protocol type. SOCK_STREM specified TCP
		0);             // Protoco Name. Should be 0 for AF_INET address family

	if (hClientSocket == INVALID_SOCKET) {
		writelog(_T("Unable to create Server socket"));
		_tprintf(_T("Unable to create Server socket"));
		// Cleanup the environment initialized by WSAStartup()
		WSACleanup();
		closelog();
		exit(2);
	}
	writelog(_T("Client socket created"));

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
		writelog(_T("Unable to connect to %s on port %d"), szServerIPAddr, nServerPort);
		_tprintf(_T("Unable to connect to %s on port %d"), szServerIPAddr, nServerPort);
		closesocket(hClientSocket);
		WSACleanup();
		closelog();
		exit(3);
	}
	writelog(_T("Connect"));

	_TCHAR szBuffer[1024] = _T("");

	while (wcscmp(szBuffer, _T("QUIT")) != 0) {
		_tprintf(_T("Enter the string to send (QUIT) to stop: "));
		wscanf_s(_T("%1023s"), szBuffer, _countof(szBuffer));

		int nLength = (wcslen(szBuffer) + 1) * sizeof(_TCHAR);

		// send( ) may not be able to send the complete data in one go.
		// So try sending the data in multiple requests
		int nCntSend = 0;
		_TCHAR *pBuffer = szBuffer;

		while ((nCntSend = send(hClientSocket, (char *)pBuffer, nLength, 0) != nLength)) {
			if (nCntSend == -1) {
				writelog(_T("Error sending the data to server"));
				_tprintf(_T("Error sending the data to server\n"));
				break;
			}
			if (nCntSend == nLength)
				break;

			pBuffer += nCntSend;
			nLength -= nCntSend;
		}

		_wcsdup(szBuffer);
		if (wcscmp(szBuffer, _T("QUIT")) == 0) {
			break;
		}

		nLength = recv(hClientSocket, (char *)szBuffer, sizeof(szBuffer), 0);
		if (nLength > 0) {
			szBuffer[nLength] = '\0';
			writelog(_T("Received %s from server"), szBuffer);
			_tprintf(_T("Received %s from server\n"), szBuffer);
		}
	}

	closesocket(hClientSocket);
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
