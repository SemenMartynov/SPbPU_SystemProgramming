#pragma once

#include <winsock2.h>
#include <iphlpapi.h>
#include <string>
#include <windows.h>
#include <stdio.h>

// Link with secur32.lib
#define SECURITY_WIN32
#include <Security.h> 

#pragma comment(lib, "Kernel32.lib")


#pragma comment(lib, "IPHLPAPI.lib")

#define SECURITY_WIN32
#include <Security.h>
#pragma comment( lib, "Secur32.lib" )

class MySystem {
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
public:
	MySystem();
	~MySystem();

	std::wstring GetUserTime();
	std::wstring GetUTCTime();
	std::wstring GetFUserName();
	std::wstring GetHostname();
	std::wstring GetCPUVendor();
	int GetCPUNumber();
	std::wstring GetVolumesInformation();
	double GetTotalMemory();
	double GetFreeMemory();
	double GetPagefileMemory();
	std::wstring GetVideoInformation();
	std::wstring GetWindowsVersion();
	double GetWindowsBuild();
	std::wstring GetWindowsRole();
	std::wstring GetConnectionInformation();
	std::wstring GetUptimeInformation();

private:
	const double OneGB;

	SYSTEMTIME stLocal;
	SYSTEMTIME stSystem;
	SYSTEM_INFO sysInfo;
	MEMORYSTATUSEX memoryStatus;
	OSVERSIONINFOEX osvInfo;
};

