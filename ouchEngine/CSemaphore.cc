#include "header/CSemaphore.h"

CSemaphore::CSemaphore()
{
	sid = -1;
	skey = 0;
}

CSemaphore::CSemaphore(key_t key)
{
	CreateSemaphore(key);
}

CSemaphore::~CSemaphore()
{
	//ReleaseSemaphore();
}

int CSemaphore::CreateSemaphore(key_t key)
{
	sid = semget(skey = key, 1, 0666 | IPC_CREAT);
	if (-1 != sid)
	{
		semun usem;
		usem.val = 1;
		if (-1 == semctl(sid, 0, SETVAL, usem))
			return -1;
	}
	return sid;
}

void CSemaphore::ReleaseSemaphore()
{
	semun usem;
	semctl(sid, 0, IPC_RMID, usem);
}

int CSemaphore::SemLock()
{
	struct sembuf semb;
	semb.sem_num = 0;
	semb.sem_op = -1;	// Lock Semaphore
	semb.sem_flg = SEM_UNDO;
	return (-1 == semop(sid, &semb, 1))?0:1;
}

int CSemaphore::SemUnlock()
{
	struct sembuf semb;
	semb.sem_num = 0;
	semb.sem_op = 1;	// Unlock Sempahore
	semb.sem_flg = SEM_UNDO;
	return (-1 == semop(sid, &semb, 1))?0:1;
}
