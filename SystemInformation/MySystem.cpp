#include "MySystem.h"

#include <sstream>
#include <iostream>
#include <Lmcons.h> // UNLEN

#include <VersionHelpers.h>

#include <iphlpapi.h>

MySystem::MySystem() : OneGB(1024 * 1024 * 1024) {
	// Fix localtime
	GetLocalTime(&stLocal);
	// Fix systime
	GetSystemTime(&stSystem);
	// System information (CPU number)
	GetSystemInfo(&sysInfo);
	// Everything about memory
	memoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memoryStatus);
	// Windows version
	osvInfo.dwOSVersionInfoSize = sizeof(osvInfo);
	GetVersionEx((OSVERSIONINFO*)&osvInfo);
}

MySystem::~MySystem() {
}

std::wstring MySystem::GetUserTime(){
	// format: YYYY-MM-DD, HH:MM:SS.ms
	std::wstringstream ss;
	ss << stLocal.wYear << "-" << stLocal.wMonth << "-" << stLocal.wDay << " " << stLocal.wHour << ":" << stLocal.wMinute << ":" << stLocal.wSecond << "." << stLocal.wMilliseconds;

	return ss.str();
}

std::wstring MySystem::GetUTCTime(){
	// format: YYYY-MM-DD, HH:MM:SS.ms
	std::wstringstream ss;
	ss << stSystem.wYear << "-" << stSystem.wMonth << "-" << stSystem.wDay << " " << stSystem.wHour << ":" << stSystem.wMinute << ":" << stSystem.wSecond << "." << stSystem.wMilliseconds;
	return ss.str();
}

std::wstring MySystem::GetFUserName(){
	// Full user's name
	TCHAR userName[UNLEN + 1];
	DWORD nULen = UNLEN;
	GetUserNameEx(NameSamCompatible, userName, &nULen);

	return std::wstring(userName);
}
std::wstring MySystem::GetHostname(){
	//Computer name can be long
	TCHAR scComputerName[MAX_COMPUTERNAME_LENGTH * 2 + 1];
	DWORD lnNameLength = MAX_COMPUTERNAME_LENGTH * 2;
	GetComputerNameEx(ComputerNameNetBIOS, scComputerName, &lnNameLength);

	return std::wstring(scComputerName);
}

std::wstring MySystem::GetCPUVendor(){
	int regs[4] = { 0 };
	char vendor[13];
	__cpuid(regs, 0);              // mov eax,0; cpuid
	memcpy(vendor, &regs[1], 4);   // copy EBX
	memcpy(vendor + 4, &regs[2], 4); // copy ECX
	memcpy(vendor + 8, &regs[3], 4); // copy EDX
	vendor[12] = '\0';

	std::string tmp(vendor);
	return std::wstring(tmp.begin(), tmp.end());
}

int MySystem::GetCPUNumber(){
	return sysInfo.dwNumberOfProcessors;
}

std::wstring MySystem::GetVolumesInformation(){
	// see http://www.codeproject.com/Articles/115061/Determine-Information-about-System-User-Processes
	std::wstringstream ss;
	TCHAR szVolume[MAX_PATH + 1];
	TCHAR szFileSystem[MAX_PATH + 1];

	DWORD dwSerialNumber;
	DWORD dwMaxLen;
	DWORD dwSystemFlags;

	TCHAR szDrives[MAX_PATH + 1];
	DWORD dwLen = GetLogicalDriveStrings(MAX_PATH, szDrives);
	TCHAR* pLetter = szDrives;

	BOOL bSuccess;

	while (*pLetter) {
		bSuccess = GetVolumeInformation(pLetter, // The source
			szVolume, MAX_PATH,   // Volume Label (LABEL)
			&dwSerialNumber, &dwMaxLen, // Serial Number (VOL)
			&dwSystemFlags,
			szFileSystem, MAX_PATH); // File System (NTFS, FAT...)

		if (bSuccess) {
			ss << *pLetter << ":" << std::endl;

			// LABEL command
			ss << "\tLabel:\t" << szVolume << std::endl;

			// Standard formal to display serial number (VOL command)
			ss << "\tNumbr:\t" << HIWORD(dwSerialNumber) << "-" << LOWORD(dwSerialNumber) << std::endl;

			// File-System
			ss << "\tFSysm:\t" << szFileSystem << std::endl << std::endl << std::endl;
		}
		else {
			ss << "No data for " << pLetter << std::endl << std::endl << std::endl;
		}

		while (*++pLetter);  // Notice Semi-colon!
		pLetter++;
	}
	return ss.str();
}

double MySystem::GetTotalMemory(){
	return memoryStatus.ullTotalPhys / OneGB;
}

double MySystem::GetFreeMemory(){
	return memoryStatus.ullAvailPhys / OneGB;
}

double MySystem::GetPagefileMemory(){
	return memoryStatus.ullTotalPageFile / OneGB;
}

std::wstring MySystem::GetVideoInformation(){
	std::wstringstream ss;
	int deviceIndex = 0;
	int result;

	do {
		PDISPLAY_DEVICE displayDevice = new DISPLAY_DEVICE();
		displayDevice->cb = sizeof(DISPLAY_DEVICE);

		result = EnumDisplayDevices(NULL, deviceIndex++, displayDevice, 0);

		if (displayDevice->StateFlags & DISPLAY_DEVICE_ACTIVE) {
			PDISPLAY_DEVICE monitor = new DISPLAY_DEVICE();
			monitor->cb = sizeof(DISPLAY_DEVICE);

			EnumDisplayDevices(displayDevice->DeviceName, 0, monitor, 0);

			ss << "Display Device:\t" << displayDevice->DeviceName << std::endl;
			ss << "Display String:\t" << displayDevice->DeviceString << std::endl;
			ss << "Display ID:\t" << displayDevice->DeviceID << std::endl << std::endl;;

			ss << "\tMonitor Device:\t" << monitor->DeviceName << std::endl;
			ss << "\tMonitor String:\t" << monitor->DeviceString << std::endl;
			ss << "\tMonitor ID:\t" << monitor->DeviceID << std::endl;

			PDEVMODE dm = new DEVMODE();
			if (EnumDisplaySettings(displayDevice->DeviceName, ENUM_CURRENT_SETTINGS, dm)) {
				ss << std::endl;
				ss << "\tFreq.: \t" << dm->dmDisplayFrequency << std::endl;
				ss << "\tBPP: \t" << dm->dmBitsPerPel << std::endl;
				ss << "\tWidth: \t" << dm->dmPelsWidth << std::endl;
				ss << "\tHeig.: \t" << dm->dmPelsHeight << std::endl;
			}
		}

	} while (result);
	
	//ss << nWidth << "x" << nHeight;
	return ss.str();
}

std::wstring MySystem::GetWindowsVersion(){
	// See https://msdn.microsoft.com/en-us/library/ms724429%28VS.85%29.aspx
	DWORD dwWinVer = GetVersion();
	std::wstringstream ss;

	if (IsWindows8Point1OrGreater()) {
		ss << "Windows 8.1";
	}
	else if (IsWindows8OrGreater()) {
		ss << "Windows 8";
	}
	else if (IsWindows7SP1OrGreater()) {
		ss << "Windows 7 SP1";
	}
	else if (IsWindows7OrGreater()) {
		ss << "Windows 7";
	}
	else if (IsWindowsVistaSP2OrGreater()) {
		ss << "Vista SP2";
	}
	else if (IsWindowsVistaSP1OrGreater()) {
		ss << "Vista SP1";
	}
	else if (IsWindowsVistaOrGreater()) {
		ss << "Vista";
	}
	else if (IsWindowsXPSP3OrGreater()) {
		ss << "XP SP3";
	}
	else if (IsWindowsXPSP2OrGreater()) {
		ss << "XP SP2";
	}
	else if (IsWindowsXPSP1OrGreater()) {
		ss << "XP SP1";
	}
	else if (IsWindowsXPOrGreater()) {
		ss << "XP";
	}
	ss << ", " << LOBYTE(LOWORD(dwWinVer)) << "." << HIBYTE(LOWORD(dwWinVer));

	return ss.str();
}

double MySystem::GetWindowsBuild(){
	return osvInfo.dwBuildNumber;
}

std::wstring MySystem::GetWindowsRole(){
	std::wstringstream ss;

	switch (osvInfo.wProductType) {
	case VER_NT_WORKSTATION:
		ss << "Workstation";
		break;
	case VER_NT_SERVER:
		ss << "Server";
		break;
	case VER_NT_DOMAIN_CONTROLLER:
		ss << "Domain Controller";
		break;
	default:
		ss << "Unknown";
	}

	return ss.str();
}

std::wstring MySystem::GetConnectionInformation(){
	// See https://msdn.microsoft.com/en-us/library/ms724429%28VS.85%29.aspx
	std::wstringstream ss;
	PIP_ADAPTER_INFO pAdapterInfo;
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(sizeof(IP_ADAPTER_INFO));
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
		PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
		while (pAdapter) {
			ss << "Adapter " << pAdapter->AdapterName << " /" << pAdapter->Description << "/" << std::endl;
			ss << "\tMAC addr:\t";
			for (UINT i = 0; i < pAdapter->AddressLength; i++) {
				if (i == (pAdapter->AddressLength - 1))
					ss << (int)pAdapter->Address[i] << std::endl;
				else
					ss << (int)pAdapter->Address[i] << "-";
			}
			ss << "\tIP Address:\t " << pAdapter->IpAddressList.IpAddress.String << std::endl;
			ss << "\tIP Mask:\t " << pAdapter->IpAddressList.IpMask.String << std::endl;
			ss << "\tGateway:\t " << pAdapter->GatewayList.IpAddress.String << std::endl;
			if (pAdapter->DhcpEnabled) {
				ss << "\tDHCP Enabled:\t Yes" << std::endl;
				ss << "\tDHCP Server:\t " << pAdapter->DhcpServer.IpAddress.String << std::endl;
			}
			else
				ss << "\tDHCP Enabled: No" << std::endl;

			pAdapter = pAdapter->Next;
		}
	}
	FREE(pAdapterInfo);
	return ss.str();
}
std::wstring MySystem::GetUptimeInformation(){
	std::wstringstream ss;

	unsigned long uptime = (unsigned long)GetTickCount64();
	unsigned int days = uptime / (24 * 60 * 60 * 1000);
	uptime %= (24 * 60 * 60 * 1000);
	unsigned int hours = uptime / (60 * 60 * 1000);
	uptime %= (60 * 60 * 1000);
	unsigned int minutes = uptime / (60 * 1000);
	uptime %= (60 * 1000);
	unsigned int seconds = uptime / (1000);

	ss << days << " days, " << hours << ":" << minutes << ":" << seconds;

	return ss.str();
}