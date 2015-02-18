#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#include "thread.h"
#include "utils.h"
#include "Logger.h"

//���������� ����������:
struct FIFOQueue queue; //��������� �������
struct Configuration config; //������������ ���������
bool isDone = false; //������� ����������
HANDLE *allhandlers; //������ ���� ����������� �������

//����������� ������ ����� � ��� ��������� � ��� ���������
CRITICAL_SECTION crs;
//�������� ���������� ��� �������-���������
CONDITION_VARIABLE condread;
//�������� ���������� ��� �������-���������
CONDITION_VARIABLE condwrite;

int _tmain(int argc, _TCHAR* argv[]) {
	Logger log(_T("ConditionVariable"));

	if (argc < 2)
		// ���������� ������������ ��-���������
		SetDefaultConfig(&config, &log);
	else
		// �������� ������� �� �����
		SetConfig(argv[1], &config, &log);

	//������� ����������� ������ ��� �� �������
	CreateAllThreads(&config, &log);

	//�������������� �������
	queue.full = 0;
	queue.readindex = 0;
	queue.writeindex = 0;
	queue.size = config.sizeOfQueue;
	queue.data = new _TCHAR*[config.sizeOfQueue];
	//�������������� �������� �������������
	InitializeCriticalSection(&crs);
	InitializeConditionVariable(&condread);
	InitializeConditionVariable(&condwrite);

	//��������� ������ �� ����������
	for (int i = 0; i < config.numOfReaders + config.numOfWriters + 1; i++)
		ResumeThread(allhandlers[i]);

	//������� ���������� ���� �������
	WaitForMultipleObjects(config.numOfReaders + config.numOfWriters + 1,
		allhandlers, TRUE, 5000);

	//��������� handle �������
	for (int i = 0; i < config.numOfReaders + config.numOfWriters + 1; i++)
		CloseHandle(allhandlers[i]);
	//������� ������ �������������
	DeleteCriticalSection(&crs);

	// ������� ������
	for (size_t i = 0; i != config.sizeOfQueue; ++i)
		if (queue.data[i])
			free(queue.data[i]); // _wcsdup ���������� calloc
	delete[] queue.data;

	// ���������� ������
	log.loudlog(_T("All is done!"));
	_getch();
	return 0;
}
