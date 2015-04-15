#pragma once

#include <winsock2.h>
#include <iphlpapi.h>
#include <string>
#include <windows.h>
#include <stdio.h>
#include <regstr.h>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <locale>

#include <cfgmgr32.h> // for MAX_DEVICE_ID_LEN
#pragma comment(lib, "Setupapi.lib")
#include <Setupapi.h>

// Link with secur32.lib
#define SECURITY_WIN32
#include <Security.h> 

#pragma comment(lib, "Kernel32.lib")


#pragma comment(lib, "IPHLPAPI.lib")

#define SECURITY_WIN32
#include <Security.h>
#pragma comment( lib, "Secur32.lib" )

struct HardwareInfo {
	std::wstring devDescrition;
	std::wstring devInstanceID;
	std::wstring hardwareMFG;
};

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
	void GetConnectedHardwareList(std::multimap<std::wstring, HardwareInfo>& result);
private:
	const double OneGB;

	SYSTEMTIME stLocal;
	SYSTEMTIME stSystem;
	SYSTEM_INFO sysInfo;
	MEMORYSTATUSEX memoryStatus;
	OSVERSIONINFOEX osvInfo;

	void FillHardwareInfo(HDEVINFO& di, SP_DEVINFO_DATA& did, HardwareInfo& hd);
};

