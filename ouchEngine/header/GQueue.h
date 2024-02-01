#ifndef __CGQUEUE_H__
#define __CGQUEUE_H__

#include <list>
#include <pthread.h>

template<typename _T> class CGQueue
{
public:
	enum QUEUE_PRIOR
	{
		PRIOR_HEAD,
		PRIOR_TAIL
	};
public:
	CGQueue()
	{
		rglist.clear();
		preoplist.clear();
		pthread_mutex_init(&rmutex, NULL);
		pthread_mutex_init(&pmutex, NULL);
	};
	virtual ~CGQueue()
	{
		Release();
		pthread_mutex_destroy(&rmutex);
		pthread_mutex_destroy(&pmutex);
	};

	void Release()
	{
		_T *p = NULL;
		pthread_mutex_lock(&rmutex);
		while (!rglist.empty())
		{
			p = rglist.back();
			rglist.pop_back();
			delete static_cast<_T*>(p); p = NULL;
		}
		pthread_mutex_unlock(&rmutex);

		pthread_mutex_lock(&pmutex);
		while (!preoplist.empty())
		{
			p = preoplist.back();
			preoplist.pop_back();
			delete static_cast<_T*>(p); p = NULL;
		}
		pthread_mutex_unlock(&pmutex);
	};
	
	void Resume()
	{
	};
	
	void Suspend(){}; // default is infinite

	inline void AddPreoptoQueue(_T *p, QUEUE_PRIOR prior = PRIOR_TAIL)
	{
		if (NULL == p)
			return;
		pthread_mutex_lock(&pmutex);
		if (PRIOR_HEAD == prior)
			preoplist.push_front(p);
		else
			preoplist.push_back(p);
		pthread_mutex_unlock(&pmutex);
	};
	
	inline void AddtoQueue(_T *p, QUEUE_PRIOR prior = PRIOR_TAIL)
	{
		if (NULL == p)
			return;
		pthread_mutex_lock(&rmutex);
		if (PRIOR_HEAD == prior)
			rglist.push_front(p);
		else
			rglist.push_back(p);
		pthread_mutex_unlock(&rmutex);
	};
	
	inline _T *GetfromQueue(QUEUE_PRIOR prior = PRIOR_HEAD)
	{
		_T *p = NULL;
		pthread_mutex_lock(&rmutex);
		if (!rglist.empty())
		{
			if (PRIOR_HEAD == prior)
			{
				p = rglist.front();
				rglist.pop_front();
			}
			else
			{
				p = rglist.back();
				rglist.pop_back();
			}
		}
		pthread_mutex_unlock(&rmutex);
		if (NULL == p)
		{
			pthread_mutex_lock(&pmutex);
			if (!preoplist.empty())
			{
				if (PRIOR_HEAD == prior)
				{
					p = preoplist.front();
					preoplist.pop_front();
				}
				else
				{
					p = preoplist.back();
					preoplist.pop_back();
				}
			}
			pthread_mutex_unlock(&pmutex);
		}
		return p;
	};

	inline _T *GetPreopFromQueue(bool bResumeRG = false)
	{
		_T *p = NULL;
		pthread_mutex_lock(&pmutex);
		if (!preoplist.empty())
		{
			p = preoplist.front();
			preoplist.pop_front();
		}
		pthread_mutex_unlock(&pmutex);
		if ((NULL == p) && bResumeRG)
		{
			pthread_mutex_lock(&rmutex);
			if (!rglist.empty())
			{
				p = rglist.front();
				rglist.pop_front();
			}
			pthread_mutex_unlock(&rmutex);
		}
		return p;
	};
	
protected:
	typedef std::list<_T*> _GLIST;
	_GLIST rglist, preoplist;
	pthread_mutex_t rmutex, pmutex;
};

#endif
