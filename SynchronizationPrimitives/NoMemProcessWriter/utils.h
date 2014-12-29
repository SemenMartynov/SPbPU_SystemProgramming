#ifndef UTILS_H_
#define UTILS_H_

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
	char **data; //������ ���������
	int writeindex; //������ ������
	int readindex; //������ ������
	int size; //������ �������
	short full; //������� ���������
};

//��������, ��������� � ������ �������
HANDLE CreateAndStartWaitableTimer(int sec);

//�������� ���� �������
void CreateAllThreads(struct Configuration * config);

//������� ��������� ������������
void SetConfig(char * filename, struct Configuration * config);

#endif /* UTILS_H_ */
