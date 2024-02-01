#pragma once
#include <map>
#include <string>
#include <pthread.h>

class CTEOrderPackage;
class COrderBookDirectory
{
public:
	COrderBookDirectory(void);
	virtual ~COrderBookDirectory(void);

	void Release() {
		pthread_mutex_lock(&mutex);
		idmap.clear();
		codemap.clear();
		pthread_mutex_unlock(&mutex);
	};
	void ProcessOrderBookPackage(CTEOrderPackage *pkg);
	uint32_t GetOrderBookId(const char *stockCode);			//	StockCode.Board
	bool GetStockCodeFromId(uint32_t id, char *stockCode);	//	StockCode.Board
	bool SetOrderBook(uint32_t id, const char *stockCode, const char *board);

private:
	pthread_mutex_t mutex;
	typedef std::map<uint32_t, std::string> _ID_MAP_;
	typedef std::map<std::string, uint32_t> _CODE_MAP_;
	_ID_MAP_ idmap;
	_CODE_MAP_ codemap;
};

class CKeyMap
{
public:
	CKeyMap();
	~CKeyMap();

	void Release() {
		pthread_mutex_lock(&mutex);
		idmap.clear();
		pthread_mutex_unlock(&mutex);
	};
	bool GetValue(uint32_t key, char *sval);
	bool SetValue(uint32_t key, const char *sval);
	bool GetValue(uint32_t key, char *szVal, size_t slen);

private:
	pthread_mutex_t mutex;
	typedef std::map<uint32_t, std::string> _ID_MAP;
	_ID_MAP idmap;
};
