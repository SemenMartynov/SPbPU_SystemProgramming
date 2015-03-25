#include <windows.h>
#include <conio.h>
#include <tchar.h>

#include "Logger.h"

#define N         5			// всего философов
#define LEFT      (id+N-1)%N// левый сосед
#define RIGHT     (id+1)%N	// правый сосед

#define THINKING  0			// состояние размышления
#define HUNGRY    1			// состояние голода
#define EATING    2			// философ ест

int philosopher_state[N];	// состояние философов
CRITICAL_SECTION crs[N];	// Объявление критических секций

DWORD WINAPI philosopherThread(LPVOID prm);
void take_forks(int id);
void put_forks(int id);
void think();
void eat();
void wait();

//Init log
Logger mylog(_T("DiningPhilosophersProblem"));

// Размышления
void think() {
	Sleep(100 + rand() % 500);
}

// Еда
void eat() {
	Sleep(50 + rand() % 450);
}

// Голодное ожидание
void wait() {
	Sleep(50 + rand() % 150);
}

// симуляция жизни философа
DWORD WINAPI philosopherThread(LPVOID prm) {
	int phil_id = (int)prm;
	while (true) {
		think();
		// Либо обе вилки, либо блокировка
		take_forks(phil_id);
		eat();
		// Вернуть вилки на стол
		put_forks(phil_id);
	}
}

void take_forks(int id) {
	bool done = false;
	while (!done){
		if (rand() % 2) { // left hand first
			mylog.quietlog(_T("Waining for Critical Section"));
			EnterCriticalSection(&crs[id]);
			mylog.quietlog(_T("Get Critical Section"));

			philosopher_state[id] = HUNGRY;
			mylog.loudlog(_T("Philosopher %d status HUNGRY"), id);

			if (TryEnterCriticalSection(&crs[LEFT])) {
				if (philosopher_state[LEFT] != EATING) {
					if (TryEnterCriticalSection(&crs[RIGHT])) {
						if (philosopher_state[RIGHT] != EATING) {
							philosopher_state[id] = EATING;
							mylog.loudlog(_T("Philosopher %d status EATING"), id);
							done = true;
						}
						LeaveCriticalSection(&crs[RIGHT]);
					}
				}
				LeaveCriticalSection(&crs[LEFT]);
			}
			mylog.quietlog(_T("Leave Critical Section"));
			LeaveCriticalSection(&crs[id]);
		}
		else { // right hand first
			mylog.quietlog(_T("Waining for Critical Section"));
			EnterCriticalSection(&crs[id]);
			mylog.quietlog(_T("Get Critical Section"));

			philosopher_state[id] = HUNGRY;
			mylog.loudlog(_T("Philosopher %d status HUNGRY"), id);

			if (TryEnterCriticalSection(&crs[RIGHT])) {
				if (philosopher_state[RIGHT] != EATING) {
					if (TryEnterCriticalSection(&crs[LEFT])) {
						if (philosopher_state[LEFT] != EATING) {
							philosopher_state[id] = EATING;
							mylog.loudlog(_T("Philosopher %d status EATING"), id);
							done = true;
						}
						LeaveCriticalSection(&crs[LEFT]);
					}
				}
				LeaveCriticalSection(&crs[RIGHT]);
			}
			mylog.quietlog(_T("Leave Critical Section"));
			LeaveCriticalSection(&crs[id]);
		}

		if (!done)
			wait();
	}
}

void put_forks(int id) {
	mylog.quietlog(_T("Waining for Critical Section"));
	EnterCriticalSection(&crs[id]);
	mylog.quietlog(_T("Get Critical Section"));

	philosopher_state[id] = THINKING;
	mylog.loudlog(_T("Philosopher %d status THINKING"), id);

	mylog.quietlog(_T("Leave Critical Section"));
	LeaveCriticalSection(&crs[id]);
}

int _tmain(int argc, _TCHAR* argv[]) {
	// Массив потоков
	HANDLE allhandlers[N];

	//создаем потоки-читатели
	mylog.loudlog(_T("Create threads"));
	for (int i = 0; i != N; ++i) {
		mylog.loudlog(_T("Count = %d"), i);
		//создаем потоки-читатели, которые пока не стартуют
		if ((allhandlers[i] = CreateThread(NULL, 0, philosopherThread, (LPVOID)i, CREATE_SUSPENDED, NULL)) == NULL) {
			mylog.loudlog(_T("Impossible to create thread-reader, GLE = %d"), GetLastError());
			exit(8000);
		}
	}

	//инициализируем средство синхронизации
	for (int i = 0; i != N; ++i) {
		InitializeCriticalSection(&crs[i]);
	}

	//запускаем потоки на исполнение
	for (int i = 0; i < N; ++i)
		ResumeThread(allhandlers[i]);

	//ожидаем завершения всех потоков
	WaitForMultipleObjects(N, allhandlers, TRUE, INFINITE);
	//закрываем handle потоков
	for (int i = 0; i < N; ++i)
		CloseHandle(allhandlers[i]);

	//удаляем объект синхронизации
	for (int i = 0; i != N; ++i) {
		DeleteCriticalSection(&crs[i]);
	}

	// Завершение работы
	mylog.loudlog(_T("All tasks are done!"));
	_getch();
	return 0;
}