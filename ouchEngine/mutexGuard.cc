#include "header/mutexGuard.h"
#include <errno.h>
#include <stdio.h>
#include <pthread.h>

void InitRWMutex(RW_MUTEX *p)
{
	if (1 == p->isInitialize)
		return;
	else if (p->isInitialize > 0) printf("InitRWMutex : %u\n", p->isInitialize);
	pthread_rwlockattr_t attr;
	pthread_rwlockattr_init(&attr);
//#ifdef _POSIX_THREAD_PROCESS_SHARED
        pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
//#else
//        #error This implementation does not support POSIX_THREAD_PROCESS_SHARED
//#endif
        p->isInitialize = (0 == pthread_rwlock_init(&p->rwmutex, &attr))?1:0;
        pthread_rwlockattr_destroy(&attr);
}

void DestroyRWMutex(RW_MUTEX *p)
{
	if (0 == p->isInitialize)
		return;
	pthread_rwlock_destroy(&p->rwmutex);
	p->isInitialize = 0;
}

CMutexGuard::CMutexGuard(pthread_mutex_t *p):pl(p)
{
	pthread_mutex_lock(pl);
}

CMutexGuard::~CMutexGuard()
{
	pthread_mutex_unlock(pl);
}

CWriteMutexGuard::CWriteMutexGuard(RW_MUTEX *p):pl(p)
{
	if (0 == pl->isInitialize)
		InitRWMutex(pl);
	if (1 == pl->isInitialize)
	{
		int ret = pthread_rwlock_wrlock(&(pl->rwmutex));
		switch (ret)
		{
		case EBUSY : printf("EBUSY - WriteMutexLock\n"); break;
		case EINVAL : printf("EINVAL - WriteMutexLock\n"); break;
		case EDEADLK : printf("EDEADLK - WriteMutexLock\n"); break;
		default : 
			//printf("WriteMutex Locked.\n");
			break;
		}
	}
	else
		printf("Uninitialize Write Mutex Lock : %u\n", pl->isInitialize);
}
CWriteMutexGuard::~CWriteMutexGuard()
{
	if (1 == pl->isInitialize)
	{
		int ret = pthread_rwlock_unlock(&(pl->rwmutex));
		switch (ret)
		{
		case EINVAL : printf("EINVAL -- WriteMutex Unlock\n"); break;
		case EPERM : printf("EPERM -- WriteMutex Unlock\n"); break;
		default :
			//printf("WriteMutex Unlocked. - ret : %ld\n", ret);
			break;
		}
	}
	else
		printf("Uninitialized Write Mutex Unlock : %u\n", pl->isInitialize);
}

CReadMutexGuard::CReadMutexGuard(RW_MUTEX *p):pl(p)
{
	if (0 == pl->isInitialize)
		InitRWMutex(pl);
	pthread_rwlock_rdlock(&(pl->rwmutex));
}

CReadMutexGuard::~CReadMutexGuard()
{
	pthread_rwlock_unlock(&(pl->rwmutex));
}
