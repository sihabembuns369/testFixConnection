#include "header/GQueue.h"

// template<typename _T> CGQueue<_T>::CGQueue()
// {
// 	rglist.clear();
// 	preoplist.clear();
// 	pthread_mutex_init(&rmutex, NULL);
// 	pthread_mutex_init(&pmutex, NULL);
// };

// template<typename _T> CGQueue<_T>::~CGQueue()
// {
// 	Release();
// 	pthread_mutex_destroy(&rmutex);
// 	pthread_mutex_destroy(&pmutex);
// };

// template<typename _T>
// CGQueue<_T>::CGQueue():evdata(FALSE, TRUE)
// {
// 	pdl.clear();
// 	event_init(&evdata, 1, 0);
// 	mutex = PTHREAD_MUTEX_INITIALIZER;
// }

// template<typename _T>
// CGQueue<_T>::~CGQueue()
// {
// 	Release();
// 	event_destroy(&evdata);
// 	pthread_mutex_destroy(&mutex);
// }

// template<typename _T>
// void CGQueue<_T>::Release()
// {
// 	_T *p;
// 	pthread_mutex_lock(&mutex);
// 	while (!pdl.empty())
// 	{
// 		p = pdl.back();
// 		pdl.pop_back();
// 		delete static_cast<_T*>(p); p = NULL;
// 	}
// 	event_reset(&evdata);
// 	pthread_mutex_unlock(&mutex);
// }

// template<typename _T>
// void CGQueue<_T>::Suspend(OL_UINT32 dwMilliseconds)
// {
// 	ACE_Time_Value evtime(0, deMilliseconds);
// 	event_timedwait(&evdata, &evtime);
// }

// template<typename _T>
// void CGQueue<_T>::Resume()
// {
// 	event_signal(&evdata);
// }

// template<typename _T>
// void CGQueue<_T>::AddtoQueue(_T *p, QUEUE_PRIOR prior)
// {
// 	dLock.Lock();
// 	pthread_mutex_lock(&mutex);
// 	if (PRIOR_HEAD == prior)
// 		pdl.push_front(p);
// 	else
// 		pdl.push_back(p);
// 	event_signal(&evdata);
// 	pthread_mutex_unlock(&mutex);
// };

// template<typename _T>
// _T *CGQueue<_T>::GetfromQueue(QUEUE_PRIOR prior)
// {
// 	_T *p;
// 	pthread_mutex_lock(&mutex);
// 	if (pdl.empty())
// 	{
// 		p = NULL;
// 		event_reset(&evdata);
// 	}
// 	else
// 	{
// 		if (PRIOR_HEAD == prior)
// 		{
// 			p = pdl.front();
// 			pdl.pop_front();
// 		}
// 		else
// 		{
// 			p = pdl.back();
// 			pdl.pop_back();
// 		}
// 	}
// 	pthread_mutex_unlock(&mutex);
// 	return p;
// };
