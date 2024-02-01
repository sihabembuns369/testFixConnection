#include "header/OuchQueue.h"
#include "header/Ouch.h"
#include "header/OuchDefine.h"
#include "header/mutexGuard.h"

template<typename _T> COuchQueue<_T>::COuchQueue() : CGQueue<_T>::CGQueue(), LOTS(100), rgsi(NULL)
{
	pthread_mutex_init(&imutex, NULL);
};

template<typename _T> COuchQueue<_T>::COuchQueue(uint32_t lot_size, CStockInfo *p) :
	CGQueue<_T>::CGQueue(), LOTS(lot_size), rgsi(p) 
{
	pthread_mutex_init(&imutex, NULL);
};

template<typename _T> COuchQueue<_T>::~COuchQueue()
{
	//Release();
	ReleaseImmediately();
	pthread_mutex_destroy(&imutex);
};

template<typename _T> void COuchQueue<_T>::PushQueue(_TQUEUE_ &q)
{
	COuchEnterOrder *msg = NULL;
	char szMarket[8] = {}, szStockCode[32] = {};
	CMutexGuard rg(&CGQueue<_T>::rmutex), pg(&CGQueue<_T>::pmutex);
	while (!q.empty())
	{
		msg = (COuchEnterOrder *)q.front();
		q.pop();
		if (NULL == msg)
			continue;
		if (OUCH_TYPE_ENTER_ORDER_MSG == msg->GetMessageType())
		{
			sscanf(msg->GetStockCode(), "%[^.].%", szStockCode, szMarket);
			if (0 == strcmp(szMarket, BOARD_RG))
			{
				uint32_t flags = rgsi->GetStockFlags(szStockCode);
				if (flags & CStockInfo::SI_PREOPENING)
					CGQueue<_T>::preoplist.push_back(msg);
				else
					CGQueue<_T>::rglist.push_back(msg);
			}
			else
				CGQueue<_T>::rglist.push_back(msg);
		}
	}
}

template<typename _T> void COuchQueue<_T>::RequeuePreopening(_TQUEUE_ &q, bool bCallAuction)
{
	char szMarket[8] = {}, szStockCode[32] = {};
	CMutexGuard rg(&CGQueue<_T>::rmutex), pg(&CGQueue<_T>::pmutex);
	for (typename CGQueue<_T>::_GLIST::iterator i = CGQueue<_T>::preoplist.begin(); i != CGQueue<_T>::preoplist.end();)
	{
		if (*i)
		{
			if (OUCH_TYPE_ENTER_ORDER_MSG == (*i)->GetMessageType())
			{
				COuchEnterOrder *o = (COuchEnterOrder *)*i;
				sscanf(o->GetStockCode(), "%[^.].%", szStockCode, szMarket);
				uint32_t flags = rgsi->GetStockFlags(szStockCode);
				bool bMove = bCallAuction ? (0 != (flags & CStockInfo::SI_WATCH_CALL)) : (0 == (flags & CStockInfo::SI_WATCH_CALL));
				if (bMove)
				{
					i = CGQueue<_T>::preoplist.erase(i);
					q.push(o);
					continue;
				}
				else
				{
					if (0 == strcmp(szMarket, BOARD_RG))
					{
						if (0 == (flags & CStockInfo::SI_PREOPENING))
						{
							i = CGQueue<_T>::preoplist.erase(i);
							CGQueue<_T>::rglist.push_back(o);
							continue;
						}
					}
				}
			}
		}
		++i;
	}
	for (typename CGQueue<_T>::_GLIST::iterator i = CGQueue<_T>::rglist.begin(); i != CGQueue<_T>::rglist.end();)
	{
		if (*i)
		{
			if (OUCH_TYPE_ENTER_ORDER_MSG == (*i)->GetMessageType())
			{
				COuchEnterOrder *o = (COuchEnterOrder *)*i;
				sscanf(o->GetStockCode(), "%[^.].%", szStockCode, szMarket);
				uint32_t flags = rgsi->GetStockFlags(szStockCode);
				bool bMove = bCallAuction ? (0 != (flags & CStockInfo::SI_WATCH_CALL)) : (0 == (flags & CStockInfo::SI_WATCH_CALL));
				if (bMove)
				{
					i = CGQueue<_T>::rglist.erase(i);
					q.push(o);
					continue;
				}
				else
				{
					if (0 == strcmp(szMarket, BOARD_RG))
					{
						if (flags & CStockInfo::SI_PREOPENING)
						{
							i = CGQueue<_T>::rglist.erase(i);
							CGQueue<_T>::preoplist.push_back(o);
							continue;
						}
					}
				}
			}
		}
		++i;
	}
}

template <typename _T> bool COuchQueue<_T>::WithdrawOrderInQueue(const char *szOrderId, bool isPreop)
{
	if (isPreop)
	{
		CMutexGuard cl(&CGQueue<_T>::pmutex);
		for (typename CGQueue<_T>::_GLIST::iterator i = CGQueue<_T>::preoplist.begin(); i != CGQueue<_T>::preoplist.end(); ++i)
		{
			if (NULL == *i)
				continue;
			if (OUCH_TYPE_ENTER_ORDER_MSG != (*i)->GetMessageType())
				continue;
			COuchEnterOrder *o = (COuchEnterOrder *)*i;
			if (0 == strcmp(o->GetOrderId(), szOrderId))
			{
				CGQueue<_T>::preoplist.erase(i);
				delete o; o = NULL;
				return true;
			}
		}
	}
	{
		CMutexGuard cl(&CGQueue<_T>::rmutex);
		for (typename CGQueue<_T>::_GLIST::iterator i = CGQueue<_T>::rglist.begin(); i != CGQueue<_T>::rglist.end(); ++i)
		{
			if (NULL == *i)
				continue;
			if (OUCH_TYPE_ENTER_ORDER_MSG != (*i)->GetMessageType())
				continue;
			COuchEnterOrder *o = (COuchEnterOrder *)*i;
			if (0 == strcmp(o->GetOrderId(), szOrderId))
			{
				CGQueue<_T>::rglist.erase(i);
				delete o; o = NULL;
				return true;
			}
		}
	}
	return false;
}
template<typename _T> bool COuchQueue<_T>::AmendOrderInQueue(const char *szOrderId, uint32_t newPrice, int64_t newVolume, bool isPreop)
{
	if (isPreop)
	{
		CMutexGuard cl(&CGQueue<_T>::pmutex);
		for (typename CGQueue<_T>::_GLIST::iterator i = CGQueue<_T>::preoplist.begin(); i != CGQueue<_T>::preoplist.end(); ++i)
		{
			if (NULL == *i)
				continue;
			if (OUCH_TYPE_ENTER_ORDER_MSG != (*i)->GetMessageType())
				continue;
			COuchEnterOrder *o = (COuchEnterOrder *)*i;
			if (0 == strcmp(o->GetOrderId(), szOrderId))
			{
				o->SetPrice(newPrice);
				o->SetQuantity(newVolume / LOTS);
				return true;
			}
		}
	}
	{
		CMutexGuard cl(&CGQueue<_T>::rmutex);
		for (typename CGQueue<_T>::_GLIST::iterator i = CGQueue<_T>::rglist.begin(); i != CGQueue<_T>::rglist.end(); ++i)
		{
			if (NULL == *i)
				continue;
			if (OUCH_TYPE_ENTER_ORDER_MSG != (*i)->GetMessageType())
				continue;
			COuchEnterOrder *o = (COuchEnterOrder *)*i;
			if (0 == strcmp(o->GetOrderId(), szOrderId))
			{
				o->SetPrice(newPrice);
				o->SetQuantity(newVolume / LOTS);
				return true;
			}
		}
	}
	return false;
}

template <typename _T> _T *COuchQueue<_T>::GetImmediately()
{
	_T *p = NULL;
    pthread_mutex_lock(&imutex);
	if (!immediateList.empty()) {
		p = immediateList.front();
		immediateList.pop_front();
	};
    pthread_mutex_unlock(&imutex);
	return p;
}

template<typename _T> void COuchQueue<_T>::AddImmediately(_T *p, typename CGQueue<_T>::QUEUE_PRIOR prior /* = PRIOR_TAIL */)
{
	if (NULL == p)
		return;
    pthread_mutex_lock(&imutex);
	if (CGQueue<_T>::PRIOR_TAIL == prior)
		immediateList.push_back(p);
	else 
		immediateList.push_front(p);
    pthread_mutex_unlock(&imutex);
}

template<typename _T> void COuchQueue<_T>::ReleaseImmediately()
{
    pthread_mutex_lock(&imutex);
	while (!immediateList.empty()) {
		_T *p = immediateList.back();
		immediateList.pop_back();
		if (p)
			delete p;
	}
    pthread_mutex_unlock(&imutex);
}
