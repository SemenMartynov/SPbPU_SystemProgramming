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

int _tmain(int argc, _TCHAR* argv[]) {
	Logger log(argv[0]);

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
	//здесь размещаем код применения выбранного средства синхронизации
	// . . .

	//запускаем потоки на исполнение
	for (int i = 0; i < config.numOfReaders + config.numOfWriters + 1; i++)
		ResumeThread(allhandlers[i]);

	//ожидаем завершения всех потоков
	WaitForMultipleObjects(config.numOfReaders + config.numOfWriters + 1,
		allhandlers, TRUE, INFINITE);
	//закрываем handle потоков
	for (int i = 0; i < config.numOfReaders + config.numOfWriters + 1; i++)
		CloseHandle(allhandlers[i]);
	//удаляем объект синхронизации
	// . . .

	// Очистка памяти
	for (size_t i = 0; i != config.sizeOfQueue; ++i)
		if (queue.data[i])
			free(queue.data[i]); // _wcsdup использует calloc
	delete[] queue.data;

	// Завершение работы
	log.loudlog(_T("All tasks are done!"));
	_getch();
	return 0;
}
