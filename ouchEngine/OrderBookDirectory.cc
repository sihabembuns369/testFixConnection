#include "header/OrderBookDirectory.h"
#include "header/TEOrderPackage.h"

COrderBookDirectory::COrderBookDirectory(void)
{
	pthread_mutex_init(&mutex, NULL);
}

COrderBookDirectory::~COrderBookDirectory(void)
{
	pthread_mutex_destroy(&mutex);
}

void COrderBookDirectory::ProcessOrderBookPackage(CTEOrderPackage *pkg)
{
	uint32_t nTot = pkg->GetDWord();
	if (0 == nTot)
		return;
	pthread_mutex_lock(&mutex);
	idmap.clear();
	codemap.clear();
	char szStockCode[32] = {}, szBoard[8] = {}, szKey[64] = {};
	for (uint32_t i = 0; i < nTot; ++i)
	{
		uint32_t id = pkg->GetDWord();
		uint16_t slen = pkg->GetWord(); pkg->GetString(slen, szStockCode);
		slen = pkg->GetWord(); pkg->GetString(slen, szBoard);
		sprintf(szKey, "%s.%s", szStockCode, szBoard);
		idmap.insert(_ID_MAP_::value_type(id, szKey));
		codemap.insert(_CODE_MAP_::value_type(szKey, id));
	}
	pthread_mutex_unlock(&mutex);
}

bool COrderBookDirectory::SetOrderBook(uint32_t id, const char *stockCode, const char *board)
{
	char szStockCode[64] = {};
	sprintf(szStockCode, "%s.%s", stockCode, board);
	pthread_mutex_lock(&mutex);
	std::pair<_ID_MAP_::iterator, bool> idret = idmap.insert(_ID_MAP_::value_type(id, szStockCode));
	std::pair<_CODE_MAP_::iterator, bool> coderet = codemap.insert(_CODE_MAP_::value_type(szStockCode, id));
	if (!idret.second)
		idret.first->second = szStockCode;
	if (!coderet.second)
		coderet.first->second = id;
	bool b = idret.second && coderet.second;
	pthread_mutex_unlock(&mutex);
	return b;
}

uint32_t COrderBookDirectory::GetOrderBookId(const char *stockCode)
{
	pthread_mutex_lock(&mutex);
	_CODE_MAP_::const_iterator i = codemap.find(stockCode);
	uint32_t id = (i == codemap.end())? 0 : i->second;	
	pthread_mutex_unlock(&mutex);
	return id;
}

bool COrderBookDirectory::GetStockCodeFromId(uint32_t id, char *stockCode)
{
	pthread_mutex_lock(&mutex);
	_ID_MAP_::const_iterator i = idmap.find(id);
	bool b = i != idmap.end();
	if (b)
		strcpy(stockCode, i->second.c_str());
	else
		stockCode[0] = 0x00;
	pthread_mutex_unlock(&mutex);
	return b;
}

CKeyMap::CKeyMap()
{
	pthread_mutex_init(&mutex, NULL);
}

CKeyMap::~CKeyMap()
{
	pthread_mutex_destroy(&mutex);
}

bool CKeyMap::GetValue(uint32_t key, char *sval)
{
	pthread_mutex_lock(&mutex);
	_ID_MAP::const_iterator i = idmap.find(key);
	bool b = i != idmap.end();
	if (b)
		strcpy(sval, i->second.c_str());
	else
		sprintf(sval, "%u", key);
	pthread_mutex_unlock(&mutex);
	return b;
}

bool CKeyMap::GetValue(uint32_t key, char *szVal, size_t slen)
{
	memset(szVal, 0x00, slen);
	pthread_mutex_lock(&mutex);
	_ID_MAP::const_iterator i = idmap.find(key);
	bool b = i != idmap.end();
	if (b)
		strncpy(szVal, i->second.c_str(), slen - 1);
	else
		sprintf(szVal, "%u", key);
	pthread_mutex_unlock(&mutex);
	return b;
}

bool CKeyMap::SetValue(uint32_t key, const char *sval)
{
	pthread_mutex_lock(&mutex);
	_ID_MAP::iterator i = idmap.find(key);
	bool b = i == idmap.end();
	if (b)
		idmap.insert(_ID_MAP::value_type(key, sval));
	else
		i->second = sval;
	pthread_mutex_unlock(&mutex);
	return b;
}
