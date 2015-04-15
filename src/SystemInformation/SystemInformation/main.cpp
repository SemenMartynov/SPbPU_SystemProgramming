#include <tchar.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include "MySystem.h"

int _tmain(int argc, _TCHAR* argv[]) {
	std::locale::global(std::locale(""));
	MySystem systm;
	std::wcout << ">> General Information" << std::endl;
	std::wcout << "User time: \t" << systm.GetUserTime() << std::endl;
	std::wcout << "UTC time: \t" << systm.GetUTCTime() << std::endl;
	std::wcout << "User name: \t" << systm.GetFUserName() << std::endl;
	std::wcout << "Hostname: \t" << systm.GetHostname() << std::endl;
	std::wcout << std::endl;

	std::wcout << "Windows: \t" << systm.GetWindowsVersion() << " (build " << systm.GetWindowsBuild() << ")" << std::endl;
	std::wcout << "Role: \t\t" << systm.GetWindowsRole() << std::endl;
	std::wcout << "Uptime: \t" << systm.GetUptimeInformation() << std::endl;
	std::wcout << std::endl;

	std::wcout << "CPU: \t\t" << systm.GetCPUVendor() << " (" << systm.GetCPUNumber() << " cores)" << std::endl;
	std::wcout << "Physical RAM: \t" << systm.GetTotalMemory() << " (GB)" << std::endl;
	std::wcout << "Available RAM: \t" << systm.GetFreeMemory() << " (GB)" << std::endl;
	std::wcout << "Pagefile: \t" << systm.GetPagefileMemory() << " (GB)" << std::endl;
	std::wcout << std::endl;

	std::wcout << ">> Video System Information" << std::endl << systm.GetVideoInformation() << std::endl;
	std::wcout << ">> Hard Disk Drive Information" << std::endl << systm.GetVolumesInformation();
	std::wcout << ">> Network Interface Information" << std::endl << systm.GetConnectionInformation();
	std::wcout << std::endl;

	std::multimap<std::wstring, HardwareInfo> results;
	systm.GetConnectedHardwareList(results);
	std::wcout << ">> Devices" << std::endl;

	std::wstring devclass = _T("none");
	for (std::multimap<std::wstring, HardwareInfo>::const_iterator it = results.begin(); it != results.end(); ++it) {
		if (devclass.compare(it->first)) {
			std::wcout << it->first << L":" << std::endl;
			devclass.assign(it->first);
		}
		std::wcout << L"\tDescription: " << it->second.devDescrition << std::endl;
		std::wcout << L"\tInstance ID: " << it->second.devInstanceID << std::endl;
		std::wcout << L"\tManufacturer: " << it->second.hardwareMFG << std::endl << std::endl;
	}

	//std::wcout << L"Done!" << std::endl;
	getwchar();
	exit(0);
}