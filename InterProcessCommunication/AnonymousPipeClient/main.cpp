#include <stdio.h>
#include <Windows.h>
int main(int argc, char* argv[])
{
	//������ ��� ��������
	char strtosend[100];
	//����� ������
	char getbuf[100];
	//����� ���������� � �������� ����
	DWORD bytessended, bytesreaded;
	
	for (int i = 0; i < 10; i++)
	{
		//������������ ������ ��� ��������
		bytessended = sprintf_s(strtosend, "message num %d", i + 1);
		strtosend[bytessended] = 0;
		fprintf(stderr, "client sended: \"%s\"\n", strtosend);
		if (!WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), strtosend, bytessended + 1, &bytesreaded, NULL)
			) //�������� ������
		{
			fprintf(stderr, "Error with writeFile\n Wait 5 sec GetLastError=%d\n", GetLastError());
			Sleep(5000);
			return 1000;
		}
		if (!ReadFile(GetStdHandle(STD_INPUT_HANDLE), getbuf, 100, &bytesreaded, NULL))
			//����� ������ �� �������
		{
			fprintf(stderr, "Error with readFile\n Wait 5 sec GetLastError=%d\n", GetLastError());
			Sleep(5000);
			return 1001;
		}
		fprintf(stderr, "Get msg from server: \"%s\"\n", getbuf);
	}
	fprintf(stderr, "client ended work\n");
	getchar();
	return 0;
}