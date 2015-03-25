#pragma once

#include <tchar.h>

class Logger
{
public:
	Logger(const _TCHAR* prog, int tid = -1);
	~Logger();

	void quietlog(_TCHAR* format, ...);
	void loudlog(_TCHAR* format, ...);
private:
	FILE* logfile;
};
