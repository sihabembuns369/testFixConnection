#ifndef __ITCH_MAP_H__
#define __ITCH_MAP_H__

#include <map>
#include <string>
#include "ItchDefine.h"

class COrderBookDirectoryMap
{
public:
	COrderBookDirectoryMap() {};
	virtual ~COrderBookDirectoryMap();

	void Release();
	void ReleaseOrderBookMap();
	void AddOrderBook(ITCH_ORDERBOOK_DIRECTORY *o);
	ITCH_ORDERBOOK_DIRECTORY *GetOrderBook(uint32_t orderBookId);

	void ReleaseFuturesMap();
	void AddFutures(ITCH_FUTURES_DIRECTORY *o);
	ITCH_FUTURES_DIRECTORY *GetFutures(uint32_t orderBookId);

	void ReleaseOptionsMap();
	void AddOptions(ITCH_OPTIONS_DIRECTORY *o);
	ITCH_OPTIONS_DIRECTORY *GetOptions(uint32_t orderBookId);

	void ReleaseIndexMap();
	void AddIndex(ITCH_INDEX_DIRECTORY *o);
	ITCH_INDEX_DIRECTORY *GetIndex(uint32_t orderbookId);

private:
	typedef std::map<uint32_t, ITCH_INDEX_DIRECTORY*> _INDEX_MAP_;
	typedef std::map<uint32_t, ITCH_FUTURES_DIRECTORY*> _FUTURES_MAP_;
	typedef std::map<uint32_t, ITCH_OPTIONS_DIRECTORY*> _OPTIONS_MAP_;
	typedef std::map<uint32_t, ITCH_ORDERBOOK_DIRECTORY*> _ORDERBOOK_MAP_;
	_INDEX_MAP_ iMap;
	_FUTURES_MAP_ fMap;
	_OPTIONS_MAP_ optMap;
	_ORDERBOOK_MAP_ oMap;
};

#endif
