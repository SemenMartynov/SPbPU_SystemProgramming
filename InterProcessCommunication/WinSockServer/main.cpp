// http://navendus.tripod.com

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

struct CLIENT_INFO
{
	SOCKET hClientSocket;
	struct sockaddr_in clientAddr;
};

char szServerIPAddr[] = "127.0.0.1"; // server IP
int nServerPort = 5050;  // server port
// clients to talk with the server

bool InitWinSock2_0();
BOOL WINAPI ClientThread(LPVOID lpData);

int main()
{
	if (!InitWinSock2_0())
	{
		std::cout << "Unable to Initialize Windows Socket environment" << WSAGetLastError() << std::endl;
		return -1;
	}

	SOCKET hServerSocket;

	hServerSocket = socket(
		AF_INET,        // The address family. AF_INET specifies TCP/IP
		SOCK_STREAM,    // Protocol type. SOCK_STREM specified TCP
		0               // Protoco Name. Should be 0 for AF_INET address family
		);
	if (hServerSocket == INVALID_SOCKET)
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

	// Bind the Server socket to the address & port
	if (bind(hServerSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cout << "Unable to bind to " << szServerIPAddr << " port " << nServerPort << std::endl;
		// Free the socket and cleanup the environment initialized by WSAStartup()
		closesocket(hServerSocket);
		WSACleanup();
		return -1;
	}

	// Put the Server socket in listen state so that it can wait for client connections
	if (listen(hServerSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cout << "Unable to put server in listen state" << std::endl;
		// Free the socket and cleanup the environment initialized by WSAStartup()
		closesocket(hServerSocket);
		WSACleanup();
		return -1;
	}

	// Start the infinite loop
	while (true)
	{
		// As the socket is in listen mode there is a connection request pending.
		// Calling accept( ) will succeed and return the socket for the request.
		SOCKET hClientSocket;
		struct sockaddr_in clientAddr;
		int nSize = sizeof(clientAddr);

		hClientSocket = accept(hServerSocket, (struct sockaddr *) &clientAddr, &nSize);
		if (hClientSocket == INVALID_SOCKET)
		{
			std::cout << "accept( ) failed" << std::endl;
		}
		else
		{
			HANDLE hClientThread;
			struct CLIENT_INFO clientInfo;
			DWORD dwThreadId;

			clientInfo.clientAddr = clientAddr;
			clientInfo.hClientSocket = hClientSocket;

			std::cout << "Client connected from " << inet_ntoa(clientAddr.sin_addr) << std::endl;

			// Start the client thread
			hClientThread = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)ClientThread,
				(LPVOID)&clientInfo, 0, &dwThreadId);
			if (hClientThread == NULL)
			{
				std::cout << "Unable to create client thread" << std::endl;
			}
			else
			{
				CloseHandle(hClientThread);
			}
		}
	}

	closesocket(hServerSocket);
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

BOOL WINAPI ClientThread(LPVOID lpData)
{
	CLIENT_INFO *pClientInfo = (CLIENT_INFO *)lpData;
	char szBuffer[1024];
	int nLength;

	while (1)
	{
		nLength = recv(pClientInfo->hClientSocket, szBuffer, sizeof(szBuffer), 0);
		if (nLength > 0)
		{
			szBuffer[nLength] = '\0';
			std::cout << "Received " << szBuffer << " from " << inet_ntoa(pClientInfo->clientAddr.sin_addr) << std::endl;

			// Convert the string to upper case and send it back, if its not QUIT
			_strdup(szBuffer);
			if (strcmp(szBuffer, "QUIT") == 0)
			{
				closesocket(pClientInfo->hClientSocket);
				return TRUE;
			}
			// send( ) may not be able to send the complete data in one go.
			// So try sending the data in multiple requests
			int nCntSend = 0;
			char *pBuffer = szBuffer;

			while ((nCntSend = send(pClientInfo->hClientSocket, pBuffer, nLength, 0) != nLength))
			{
				if (nCntSend == -1)
				{
					std::cout << "Error sending the data to " << inet_ntoa(pClientInfo->clientAddr.sin_addr) << std::endl;
					break;
				}
				if (nCntSend == nLength)
					break;

				pBuffer += nCntSend;
				nLength -= nCntSend;
			}
		}
		else
		{
			std::cout << "Error reading the data from " << inet_ntoa(pClientInfo->clientAddr.sin_addr) << std::endl;
		}
	}

	return TRUE;
}
