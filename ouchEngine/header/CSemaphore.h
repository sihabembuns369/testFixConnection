#ifndef __CSEMAPHORE_H__
#define __CSEMAPHORE_H__

#include <sys/sem.h>
#include <sys/types.h>

union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
				   (Linux-specific) */
};

class CSemaphore
{
public:
	CSemaphore ();
	CSemaphore(key_t key);
	~CSemaphore();

	int CreateSemaphore(key_t key);
	void ReleaseSemaphore();

	int SemLock();
	int SemUnlock();
	
private:
	int sid;	// semaphore id
	key_t skey;	// semaphore global key
};

class CSemaphoreGuard
{
public:
	CSemaphoreGuard(CSemaphore *p)
	{
		ps = p;
		ps->SemLock();
	};
	~CSemaphoreGuard()
	{
		ps->SemUnlock();
	};
private:
	CSemaphore *ps;
};

#endif
