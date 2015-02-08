#include <stdio.h>
#include <Windows.h>
int main(int argc, char* argv[])
{
	//строка для передачи
	char strtosend[100];
	//буфер приема
	char getbuf[100];
	//число переданных и принятых байт
	DWORD bytessended, bytesreaded;
	
	for (int i = 0; i < 10; i++)
	{
		//формирование строки для передачи
		bytessended = sprintf_s(strtosend, "message num %d", i + 1);
		strtosend[bytessended] = 0;
		fprintf(stderr, "client sended: \"%s\"\n", strtosend);
		if (!WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), strtosend, bytessended + 1, &bytesreaded, NULL)
			) //передача данных
		{
			fprintf(stderr, "Error with writeFile\n Wait 5 sec GetLastError=%d\n", GetLastError());
			Sleep(5000);
			return 1000;
		}
		if (!ReadFile(GetStdHandle(STD_INPUT_HANDLE), getbuf, 100, &bytesreaded, NULL))
			//прием ответа от сервера
		{
			fprintf(stderr, "Error with readFile\n Wait 5 sec GetLastError=%d\n", GetLastError());
			Sleep(5000);
			return 1001;
		}
		fprintf(stderr, "Get msg from server: \"%s\"\n", getbuf);
	}
	fprintf(stderr, "client ended work\n Wait 5 sec");
	Sleep(5000);
	return 0;
}