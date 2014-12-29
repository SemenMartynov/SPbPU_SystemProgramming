#include <windows.h>
#include <stdio.h>
#include <conio.h>
#define BUF_SIZE 256
TCHAR szName[] = TEXT("MyFileMappingObject");
TCHAR szMsg[] = TEXT("Message from first process");
HANDLE mutex;
void main()
{
	HANDLE hMapFile;
	LPCTSTR pBuf;
	mutex = CreateMutex(NULL, false, TEXT("SyncMutex"));
	// create a memory, wicth two proccess will be working
	hMapFile = CreateFileMapping(
		// использование файла подкачки
		INVALID_HANDLE_VALUE,
		// защита по умолчанию
		NULL,
		// доступ к чтению/записи
		PAGE_READWRITE,
		// макс. размер объекта
		0,
		// размер буфера
		BUF_SIZE,
		// имя отраженного в памяти объекта
		szName);

	if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE)
	{
		printf("Не может создать отраженный в памяти объект (%d).\n",
			GetLastError());
		return;
	}
	pBuf = (LPTSTR)MapViewOfFile(
		//дескриптор проецируемого в памяти объекта
		hMapFile,
		// разрешение чтения/записи(режим доступа)
		FILE_MAP_ALL_ACCESS,
		//Старшее слово смещения файла, где начинается отображение
		0,
		//Младшее слово смещения файла, где начинается отображение
		0,
		//Число отображаемых байтов файла	
		BUF_SIZE);
	
	if (pBuf == NULL)
	{
		printf("Представление проецированного файла невозможно (%d).\n",
			GetLastError());
		return;
	}

	int i = 0;
	while (true)
	{
		i = rand();
		itoa(i, (char *)szMsg, 10);
		WaitForSingleObject(mutex, INFINITE);
		CopyMemory((PVOID)pBuf, szMsg, sizeof(szMsg));
		printf("write message: %s\n", (char *)pBuf);
		Sleep(1000); //необходимо только для отладки - для удобства представления и анализа
		//результатов
		ReleaseMutex(mutex);
	}
	// освобождение памяти и закрытие описателя handle
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);
	CloseHandle(mutex);
}