#pragma once
#include <set>
#include <string.h>

class COrderCheck
{
public:
	COrderCheck()
	{
		oset.clear();
		UpdateCurDate();
		pthread_mutex_init(&mutex, NULL);
	};
	~COrderCheck() {
		clear();
		pthread_mutex_destroy(&mutex);
	};
	bool insert(const char *id)
	{
		if (NULL == strstr(id, szCurDate))
			return false;
		pthread_mutex_lock(&mutex);
		bool bRet = oset.insert(id).second;
		pthread_mutex_unlock(&mutex);
		return bRet; // Return TRUE if Success insert / id not found, FALSE if id found and insert failed
	};
	void erase(const char *id)
	{
		pthread_mutex_lock(&mutex);
		oset.erase(id);
		pthread_mutex_unlock(&mutex);
	};
	void clear()
	{
		pthread_mutex_lock(&mutex);
		oset.clear();
		pthread_mutex_unlock(&mutex);
	};
	void UpdateCurDate()
	{
		time_t ltime = time(NULL);
		struct tm *ptm = localtime(&ltime);
		sprintf(szCurDate, "%02d%02d%02d", ptm->tm_year % 100, ptm->tm_mon + 1, ptm->tm_mday);
	}
	bool IsExists(const char *id)
	{
		pthread_mutex_lock(&mutex);
		_OSET_::const_iterator i = oset.find(id);
		bool b = i != oset.end();
		pthread_mutex_unlock(&mutex);
		return b;
	};
	bool isValid(const char *szId)
	{
		pthread_mutex_lock(&mutex);
		_OSET_::const_iterator i = oset.find(szId);
		bool b = i != oset.end();
		pthread_mutex_unlock(&mutex);
		return b;
	};

private:
	char szCurDate[32];
	pthread_mutex_t mutex;
	typedef std::set<std::string> _OSET_;
	_OSET_ oset;
};
