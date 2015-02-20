#pragma once

#include "Logger.h"

// Структура с конфигурацией
struct Configuration {
	int numOfReaders; //число потоков-читателей
	int numOfWriters; //число потоков-писателей
	int sizeOfQueue; //размер очереди
	int readersDelay; //задержка на работу читателей (в миллисек)
	int writersDelay; //задержка на работу писателей (в миллисек)
	int ttl; //"время жизни"
};

//Структура описывающая FIFO очередь
struct FIFOQueue {
	_TCHAR **data; //массив сообщений
	int writeindex; //индекс записи
	int readindex; //индекс чтения
	int size; //размер очереди
	short full; //очередь заполнена
};

//создание, установка и запуск таймера
HANDLE CreateAndStartWaitableTimer(int sec);

//создание всех потоков
void CreateAllThreads(struct Configuration* config, Logger* log);

//функция установки конфигурации
void SetConfig(_TCHAR* path, struct Configuration* config, Logger* log);

//функция установки конфигурации по умолчанию
void SetDefaultConfig(struct Configuration* config, Logger* log);


