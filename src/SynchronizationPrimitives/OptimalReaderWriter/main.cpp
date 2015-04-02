#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#include "thread.h"
#include "utils.h"
#include "Logger.h"

//глобальные переменные:
struct Configuration config; //конфигурация программы
bool isDone = false; //флаг завершения
HANDLE *allhandlers; //массив всех создаваемых потоков

// инструмент синхронизации:
SRWLOCK lock;
//условная переменная для потоков-читателей
CONDITION_VARIABLE condread;

HANDLE exitEvent; //завершение программы (ручной сброс);

//имя разделяемой памяти
wchar_t shareFileName[] = L"$$MyVerySpecialShareFileName$$";

HANDLE hFileMapping; //объект-отображение файла
// указатели на отображаемую память
LPVOID lpFileMapForWriters;
LPVOID lpFileMapForReaders;

int _tmain(int argc, _TCHAR* argv[]) {
	Logger log(_T("OptimalReaderWriter"));

	if (argc < 2)
		// Используем конфигурацию по-умолчанию
		SetDefaultConfig(&config, &log);
	else
		// Загрузка конфига из файла
		SetConfig(argv[1], &config, &log);

	//создаем необходимые потоки без их запуска
	CreateAllThreads(&config, &log);

	//Инициализируем ресурс (share memory): создаем объект "отображаемый файл"
	// будет использован системный файл подкачки (на диске файл создаваться
	// не будет), т.к. в качестве дескриптора файла использовано значение
	// равное 0xFFFFFFFF (его эквивалент - символическая константа INVALID_HANDLE_VALUE)
	if ((hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
		PAGE_READWRITE, 0, 1500, shareFileName)) == NULL) {
		// INVALID_HANDLE_VALUE - дескриптор открытого файла
		//                        (INVALID_HANDLE_VALUE - файл подкачки)
		// NULL - атрибуты защиты объекта-отображения
		// PAGE_READWRITE - озможности доступа к представлению файла при
		//                  отображении (PAGE_READWRITE - чтение/запись)
		// 0, 1500 - старшая и младшая части значения максимального
		//           размера объекта отображения файла
		// shareFileName - имя объекта-отображения.
		log.loudlog(_T("Impossible to create shareFile, GLE = %d"),
			GetLastError());
		ExitProcess(10000);
	}
	//отображаем файл на адресное пространство нашего процесса для потока-писателя
	lpFileMapForWriters = MapViewOfFile(hFileMapping, FILE_MAP_WRITE, 0, 0, 0);
	//  hFileMapping - дескриптор объекта-отображения файла
	//  FILE_MAP_WRITE - доступа к файлу
	//  0, 0 - старшая и младшая части смещения начала отображаемого участка в файле
	//         (0 - начало отображаемого участка совпадает с началом файла)
	//  0 - размер отображаемого участка файла в байтах (0 - весь файл)

	//отображаем файл на адресное пространство нашего процесса для потоков-читателей
	lpFileMapForReaders = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

	//инициализируем средства синхронизации
	//событие "завершение работы программы", ручной сброс, изначально занято
	exitEvent = CreateEvent(NULL, true, false, L"");
	InitializeSRWLock(&lock);
	InitializeConditionVariable(&condread);

	//запускаем потоки-писатели и поток-планировщик на исполнение
	for (int i = 0; i < config.numOfReaders + config.numOfWriters + 1; i++)
		ResumeThread(allhandlers[i]);

	//ожидаем завершения всех потоков
	WaitForMultipleObjects(config.numOfReaders + config.numOfWriters + 1,
		allhandlers, TRUE, INFINITE);

	//закрываем handle потоков
	for (int i = 0; i < config.numOfReaders + config.numOfWriters + 1; i++)
		CloseHandle(allhandlers[i]);

	//закрываем описатели объектов синхронизации
	CloseHandle(exitEvent);

	//закрываем handle общего ресурса
	UnmapViewOfFile(lpFileMapForReaders);
	UnmapViewOfFile(lpFileMapForWriters);

	//закрываем объект "отображаемый файл"
	CloseHandle(hFileMapping);

	// Завершение работы
	log.loudlog(_T("All tasks are done!"));
	_getch();
	return 0;
}
