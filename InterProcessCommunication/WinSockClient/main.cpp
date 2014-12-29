#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

char szServerIPAddr[20]; // server IP
int nServerPort; // server port

bool InitWinSock2_0();

int main()
{
	std::cout << "Enter the server IP Address: ";
	std::cin >> szServerIPAddr;
	std::cout << "Enter the server port number: ";
	std::cin >> nServerPort;

	if (!InitWinSock2_0())
	{
		std::cout << "Unable to Initialize Windows Socket environment" << WSAGetLastError() << std::endl;
		return -1;
	}

	SOCKET hClientSocket;

	hClientSocket = socket(
		AF_INET,        // The address family. AF_INET specifies TCP/IP
		SOCK_STREAM,    // Protocol type. SOCK_STREM specified TCP
		0               // Protoco Name. Should be 0 for AF_INET address family
		);
	if (hClientSocket == INVALID_SOCKET)
	{
		std::cout << "Unable to create Server socket" << std::endl;
		// Cleanup the environment initialized by WSAStartup()
		WSACleanup();
		return -1;
	}


	// Create the structure describing various Server parameters
	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;     // The address family. MUST be AF_INET
	serverAddr.sin_addr.s_addr = inet_addr(szServerIPAddr);
	serverAddr.sin_port = htons(nServerPort);

	// Connect to the server
	if (connect(hClientSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
	{
		std::cout << "Unable to connect to " << szServerIPAddr << " on port " << nServerPort << std::endl;
		closesocket(hClientSocket);
		WSACleanup();
		return -1;
	}

	char szBuffer[1024] = "";

	while (strcmp(szBuffer, "QUIT") != 0)
	{
		std::cout << "Enter the string to send (QUIT) to stop: ";
		std::cin >> szBuffer;

		int nLength = strlen(szBuffer);

		// send( ) may not be able to send the complete data in one go.
		// So try sending the data in multiple requests
		int nCntSend = 0;
		char *pBuffer = szBuffer;

		while ((nCntSend = send(hClientSocket, pBuffer, nLength, 0) != nLength))
		{
			if (nCntSend == -1)
			{
				std::cout << "Error sending the data to server" << std::endl;
				break;
			}
			if (nCntSend == nLength)
				break;

			pBuffer += nCntSend;
			nLength -= nCntSend;
		}

		_strdup(szBuffer);
		if (strcmp(szBuffer, "QUIT") == 0)
		{
			break;
		}

		nLength = recv(hClientSocket, szBuffer, sizeof(szBuffer), 0);
		if (nLength > 0)
		{
			szBuffer[nLength] = '\0';
			std::cout << "Received " << szBuffer << " from server" << std::endl;
		}
	}

	closesocket(hClientSocket);
	WSACleanup();
	return 0;
}

bool InitWinSock2_0()
{
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 0);

	if (!WSAStartup(wVersion, &wsaData))
		return true;

	return false;
}
