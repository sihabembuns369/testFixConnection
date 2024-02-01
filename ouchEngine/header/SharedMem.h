#ifndef __SHAREDMEM_H__
#define __SHAREDMEM_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

template<typename _T> class CSharedMem
{
public:
	CSharedMem()
	{
		shmId = 0;
		pdata = NULL;
		shmkey = 0;
		m_datasize = 0;
		flags = 0x666;
	};
	~CSharedMem()
	{
		DetachMem();
	};
	int Create(key_t key, size_t segmentsize, int fl = 0644 | IPC_CREAT)
	{
		shmkey = key;
		flags = fl;
		m_datasize = segmentsize;
		shmId = shmget(shmkey, m_datasize, flags);
		return shmId;
	};
	int Destroy()
	{
		return shmctl(shmId, IPC_RMID, 0);
	}
	_T *AttachMem()
	{
		pdata = (_T*)shmat(shmId, 0, 0);
		if (pdata == (void*)-1)
			pdata = NULL;
		return pdata;
	};
	_T *GetMemPtr() const {return pdata;};
	void DetachMem()
	{
		if (pdata)
			shmdt((void*)pdata);
	};

private:
	int flags;
	_T *pdata;
	int shmId;
	key_t shmkey;
	size_t m_datasize;
};

#endif
