#ifndef UTILS_H_
#define UTILS_H_

// —труктура с конфигурацией
struct Configuration {
	int numOfReaders; //число потоков-читателей
	int numOfWriters; //число потоков-писателей
	int sizeOfQueue; //размер очереди
	int readersDelay; //задержка на работу читателей (в миллисек)
	int writersDelay; //задержка на работу писателей (в миллисек)
	int ttl; //"врем€ жизни"
};

//—труктура описывающа€ FIFO очередь
struct FIFOQueue {
	char **data; //массив сообщений
	int writeindex; //индекс записи
	int readindex; //индекс чтени€
	int size; //размер очереди
	short full; //очередь заполнена
};

//создание, установка и запуск таймера
HANDLE CreateAndStartWaitableTimer(int sec);

//создание всех потоков
void CreateAllThreads(struct Configuration * config);

//функци€ установки конфигурации
void SetConfig(char * filename, struct Configuration * config);

#endif /* UTILS_H_ */
