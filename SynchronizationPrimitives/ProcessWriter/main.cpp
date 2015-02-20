#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#include "thread.h"
#include "utils.h"
#include "Logger.h"

//глобальные переменные
struct Configuration config; //конфигурация программы
bool isDone = false; //флаг завершения
HANDLE *allhandlers; //массив всех создаваемых потоков

//события для синхронизации:
HANDLE canReadEvent; //писатель записал сообщение (ручной сброс);
HANDLE canWriteEvent; //все читатели готовы к приему следующего (автосброс);
HANDLE allReadEvent; //все читатели прочитали сообщение (ручной сброс);
HANDLE changeCountEvent; //разрешение работы со счетчиком (автосброс);
HANDLE exitEvent; //завершение программы (ручной сброс);

//переменные для синхронизации работы потоков:
int countread = 0;  //число потоков, которое уже прочитали данные
//                    (устанавливается писателем и изменяется
//                     читателями после прочтения сообщения)
int countready = 0; //число потоков, готовых для чтения сообщения
//                    (ожидающих сигнала от писателя)

//имя разделяемой памяти
wchar_t shareFileName[] = L"$$MyVerySpecialShareFileName$$";

HANDLE hFileMapping; //объект-отображение файла
LPVOID lpFileMapForWriters; // указатели на отображаемую память

int _tmain(int argc, _TCHAR* argv[]) {
	Logger log(_T("ProcessWriter"));

	if (argc < 2)
		// Используем конфигурацию по-умолчанию
		SetDefaultConfig(&config, &log);
	else
		// Загрузка конфига из файла
		SetConfig(argv[1], &config, &log);

	//создаем необходимые потоки без их запуска
	//потоки-читатели запускаются сразу (чтобы они успели дойти до функции ожидания)
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

	//инициализируем 2 переменные в общей памяти (readready и readcount)
	*((int *)lpFileMapForWriters) = 0;
	*(((int *)lpFileMapForWriters) + 1) = config.numOfReaders;

	//инициализируем средства синхронизации
	// (атрибуты защиты, автосброс, начальное состояние, имя):
	//событие "окончание записи" (можно читать), ручной сброс, изначально занято
	canReadEvent = CreateEvent(NULL, true, false, L"$$My_canReadEvent$$");
	//событие - "можно писать",автосброс(разрешаем писать только одному), изначально свободно
	canWriteEvent = CreateEvent(NULL, false, false, L"$$My_canWriteEvent$$");
	//событие "все прочитали"
	allReadEvent = CreateEvent(NULL, true, true, L"$$My_allReadEvent$$");
	//событие для изменения счетчика (сколько клиентов еще не прочитало сообщение)
	changeCountEvent = CreateEvent(NULL, false, true, L"$$My_changeCountEvent$$");
	//событие "завершение работы программы", ручной сброс, изначально занято
	exitEvent = CreateEvent(NULL, true, false, L"$$My_exitEvent$$");

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
	CloseHandle(canReadEvent);
	CloseHandle(canWriteEvent);
	CloseHandle(allReadEvent);
	CloseHandle(changeCountEvent);
	CloseHandle(exitEvent);

	UnmapViewOfFile(lpFileMapForWriters); //закрываем handle общего ресурса
	CloseHandle(hFileMapping); //закрываем объект "отображаемый файл"

	log.loudlog(_T("All is done!"));
	_getch();
	return 0;
}
