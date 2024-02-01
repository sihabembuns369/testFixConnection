#ifndef __MUTEXGUARD_H__
#define __MUTEXGUARD_H__

#include "constants.h"
#include <pthread.h>

typedef struct _RW_MUTEX_
{
	OL_UINT8 isInitialize;
	pthread_rwlock_t rwmutex;	
} RW_MUTEX;

void InitRWMutex(RW_MUTEX *p);
void DestroyRWMutex(RW_MUTEX *p);

class CMutexGuard
{
public:
	CMutexGuard(pthread_mutex_t *p);
	~CMutexGuard();
private:
	pthread_mutex_t *pl;
};

class CWriteMutexGuard
{
public:
	CWriteMutexGuard(RW_MUTEX *p);
	~CWriteMutexGuard();
private:
	RW_MUTEX *pl;
};

class CReadMutexGuard
{
public:
	CReadMutexGuard(RW_MUTEX *p);
	~CReadMutexGuard();
private:
	RW_MUTEX *pl;
};

#endif
