#pragma once

#include "Logger.h"

// ��������� � �������������
struct Configuration {
	int numOfReaders; //����� �������-���������
	int numOfWriters; //����� �������-���������
	int sizeOfQueue; //������ �������
	int readersDelay; //�������� �� ������ ��������� (� ��������)
	int writersDelay; //�������� �� ������ ��������� (� ��������)
	int ttl; //"����� �����"
};

//��������� ����������� FIFO �������
struct FIFOQueue {
	_TCHAR **data; //������ ���������
	int writeindex; //������ ������
	int readindex; //������ ������
	int size; //������ �������
	short full; //������� ���������
};

//��������, ��������� � ������ �������
HANDLE CreateAndStartWaitableTimer(int sec);

//�������� ���� �������
void CreateAllThreads(struct Configuration* config, Logger* log);

//������� ��������� ������������
void SetConfig(_TCHAR* path, struct Configuration* config, Logger* log);

//������� ��������� ������������ �� ���������
void SetDefaultConfig(struct Configuration* config, Logger* log);


