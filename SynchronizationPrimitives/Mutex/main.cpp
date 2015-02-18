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
HANDLE mutex; // ��������� ��������

int _tmain(int argc, _TCHAR* argv[]) {
	Logger log(_T("Mutex"));

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
	mutex = CreateMutex(NULL, FALSE, L"");
	//     NULL - ��������� ������������
	//     FALSE - ����������� ������� ������ ���������� �� �����������
	//     "" - ��� ��������

	//��������� ������ �� ����������
	for (int i = 0; i < config.numOfReaders + config.numOfWriters + 1; i++)
		ResumeThread(allhandlers[i]);

	//������� ���������� ���� �������
	WaitForMultipleObjects(config.numOfReaders + config.numOfWriters + 1,
		allhandlers, TRUE, INFINITE);
	//��������� handle �������
	for (int i = 0; i < config.numOfReaders + config.numOfWriters + 1; i++)
		CloseHandle(allhandlers[i]);
	//������� ������ �������������
	CloseHandle(mutex);

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
