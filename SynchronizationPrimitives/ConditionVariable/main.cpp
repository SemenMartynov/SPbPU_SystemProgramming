#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#include "thread.h"
#include "utils.h"
#include "Logger.h"

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

int _tmain(int argc, _TCHAR* argv[]) {
	Logger log(_T("ConditionVariable"));

	if (argc < 2)
		// Используем конфигурацию по-умолчанию
		SetDefaultConfig(&config, &log);
	else
		// Загрузка конфига из файла
		SetConfig(argv[1], &config, &log);

	//создаем необходимые потоки без их запуска
	CreateAllThreads(&config, &log);

	//Инициализируем очередь
	queue.full = 0;
	queue.readindex = 0;
	queue.writeindex = 0;
	queue.size = config.sizeOfQueue;
	queue.data = new _TCHAR*[config.sizeOfQueue];
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

	// Очистка памяти
	for (size_t i = 0; i != config.sizeOfQueue; ++i)
		if (queue.data[i])
			free(queue.data[i]); // _wcsdup использует calloc
	delete[] queue.data;

	// Завершение работы
	log.loudlog(_T("All is done!"));
	_getch();
	return 0;
}
