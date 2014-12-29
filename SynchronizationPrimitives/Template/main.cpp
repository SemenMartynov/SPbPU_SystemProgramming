#include<windows.h>
#include<string.h>
#include<stdio.h>
#include<conio.h>

#include"thread.h"
#include"utils.h"

//���������� ����������:
struct FIFOQueue queue; //��������� �������
struct Configuration config; //������������ ���������
bool isDone = false; //������� ����������
HANDLE *allhandlers; //������ ���� ����������� �������

int main(int argc, char* argv[]) {
	if (argc < 2) {
		//���������� ������������ ��-���������
		SetConfig(NULL, &config);
	}
	else {
		char filename[30]; //��� ����� ������������
		strcpy_s(filename, argv[1]);
		// �������� ��� ��������� ����� � ����������� ��������
		SetConfig(filename, &config);
	}

	//������� ����������� ������ ��� �� �������
	CreateAllThreads(&config);

	//�������������� �������
	queue.full = 0;
	queue.readindex = 0;
	queue.writeindex = 0;
	queue.size = config.sizeOfQueue;
	queue.data = new char*[config.sizeOfQueue];
	//�������������� �������� �������������
	//����� ��������� ��� ���������� ���������� �������� �������������
	// . . .

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
	// . . .

	printf("all is done\n");
	_getch();
	return 0;
}