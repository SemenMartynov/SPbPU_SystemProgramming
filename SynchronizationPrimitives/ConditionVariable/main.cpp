#include<windows.h>
#include<string.h>
#include<stdio.h>
#include<conio.h>

#include"thread.h"
#include"utils.h"

//глобальные переменные:
struct FIFOQueue queue; //структура очереди
struct Configuration config; //конфигурация программы
bool isDone = false; //Признак завершения
HANDLE *allhandlers; //массив всех создаваемых потоков

//критическая секция общая и для писателей и для читателей
CRITICAL_SECTION crs;
//условная переменная для потоков-писателей
CONDITION_VARIABLE condread;
//условная переменная для потоков-читателей
CONDITION_VARIABLE condwrite;

int main(int argc, char* argv[]) {
	if (argc < 2) {
		//используем конфигурацию по-умолчанию
		SetConfig(NULL, &config);
	}
	else {
		char filename[30]; //имя файла конфигурации
		strcpy_s(filename, argv[1]);
		// Передаем имя читаемого файла и заполняемую стуктуру
		SetConfig(filename, &config);
	}

	//создаем необходимые потоки без их запуска
	CreateAllThreads(&config);

	//Инициализируем очередь
	queue.full = 0;
	queue.readindex = 0;
	queue.writeindex = 0;
	queue.size = config.sizeOfQueue;
	queue.data = new char*[config.sizeOfQueue];
	//инициализируем средство синхронизации
	InitializeCriticalSection(&crs);
	InitializeConditionVariable(&condread);
	InitializeConditionVariable(&condwrite);

	//запускаем потоки на исполнение
	for (int i = 0; i < config.numOfReaders + config.numOfWriters + 1; i++)
		ResumeThread(allhandlers[i]);

	//ожидаем завершения всех потоков
	WaitForMultipleObjects(config.numOfReaders + config.numOfWriters + 1,
		allhandlers, TRUE, 5000);

	//закрываем handle потоков
	for (int i = 0; i < config.numOfReaders + config.numOfWriters + 1; i++)
		CloseHandle(allhandlers[i]);
	//удаляем объект синхронизации
	DeleteCriticalSection(&crs);

	printf("all is done\n");
	_getch();
	return 0;
}
