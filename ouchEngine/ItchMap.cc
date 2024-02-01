#include "header/ItchMap.h"
#include <stdio.h>
#include <string.h>

COrderBookDirectoryMap::~COrderBookDirectoryMap()
{
	Release();
}

void COrderBookDirectoryMap::Release()
{
	ReleaseOrderBookMap();
	ReleaseFuturesMap();
	ReleaseOptionsMap();
	ReleaseIndexMap();
}

void COrderBookDirectoryMap::ReleaseOrderBookMap()
{
	for (_ORDERBOOK_MAP_::const_iterator pos = oMap.begin(); pos != oMap.end(); ++pos)
		delete static_cast<ITCH_ORDERBOOK_DIRECTORY*>(pos->second);
	oMap.clear();
}

void COrderBookDirectoryMap::AddOrderBook(ITCH_ORDERBOOK_DIRECTORY *o)
{
	_ORDERBOOK_MAP_::const_iterator i = oMap.find(o->orderbookId);
	if (i == oMap.end())
	{
		ITCH_ORDERBOOK_DIRECTORY *ob = new ITCH_ORDERBOOK_DIRECTORY;
		memcpy(ob, o, SZ_ITCH_ORDERBOOK_DIRECTORY);
		oMap.insert(_ORDERBOOK_MAP_::value_type(ob->orderbookId, ob));
	}
	else
	{
		ITCH_ORDERBOOK_DIRECTORY *ob = i->second;
		memcpy(ob, o, SZ_ITCH_ORDERBOOK_DIRECTORY);
	}
}

ITCH_ORDERBOOK_DIRECTORY *COrderBookDirectoryMap::GetOrderBook(uint32_t orderBookId)
{
	_ORDERBOOK_MAP_::const_iterator i = oMap.find(orderBookId);
	return (i == oMap.end()) ? NULL : i->second;
}

void COrderBookDirectoryMap::ReleaseFuturesMap()
{
	for (_FUTURES_MAP_::const_iterator pos = fMap.begin(); pos != fMap.end(); ++pos)
		delete static_cast<ITCH_FUTURES_DIRECTORY*>(pos->second);
	fMap.clear();
}

void COrderBookDirectoryMap::AddFutures(ITCH_FUTURES_DIRECTORY *o)
{
	_FUTURES_MAP_::const_iterator i = fMap.find(o->orderbookId);
	if (i == fMap.end())
	{
		ITCH_FUTURES_DIRECTORY *ob = new ITCH_FUTURES_DIRECTORY;
		memcpy(ob, o, SZ_ITCH_FUTURES_DIRECTORY);
		fMap.insert(_FUTURES_MAP_::value_type(ob->orderbookId, ob));
	}
	else
	{
		ITCH_FUTURES_DIRECTORY *ob = i->second;
		memcpy(ob, o, SZ_ITCH_FUTURES_DIRECTORY);
	}
}

ITCH_FUTURES_DIRECTORY *COrderBookDirectoryMap::GetFutures(uint32_t orderBookId)
{
	_FUTURES_MAP_::const_iterator i = fMap.find(orderBookId);
	return (i == fMap.end()) ? NULL : i->second;
}

void COrderBookDirectoryMap::ReleaseOptionsMap()
{
	for (_OPTIONS_MAP_::const_iterator pos = optMap.begin(); pos != optMap.end(); ++pos)
		delete static_cast<ITCH_OPTIONS_DIRECTORY*>(pos->second);
	optMap.clear();
}

void COrderBookDirectoryMap::AddOptions(ITCH_OPTIONS_DIRECTORY *o)
{
	_OPTIONS_MAP_::const_iterator i = optMap.find(o->orderbookId);
	if (i == optMap.end())
	{
		ITCH_OPTIONS_DIRECTORY *ob = new ITCH_OPTIONS_DIRECTORY;
		memcpy(ob, o, SZ_ITCH_OPTIONS_DIRECTORY);
		optMap.insert(_OPTIONS_MAP_::value_type(ob->orderbookId, ob));
	}
	else
	{
		ITCH_OPTIONS_DIRECTORY *ob = i->second;
		memcpy(ob, o, SZ_ITCH_OPTIONS_DIRECTORY);
	}
}

ITCH_OPTIONS_DIRECTORY *COrderBookDirectoryMap::GetOptions(uint32_t orderBookId)
{
	_OPTIONS_MAP_::const_iterator i = optMap.find(orderBookId);
	return (i == optMap.end()) ? NULL : i->second;
}

void COrderBookDirectoryMap::ReleaseIndexMap()
{
	for (_INDEX_MAP_::const_iterator pos = iMap.begin(); pos != iMap.end(); ++pos)
		delete static_cast<ITCH_INDEX_DIRECTORY*>(pos->second);
	iMap.clear();
}

void COrderBookDirectoryMap::AddIndex(ITCH_INDEX_DIRECTORY *o)
{
	_INDEX_MAP_::const_iterator i = iMap.find(o->orderbookId);
	if (i == iMap.end())
	{
		ITCH_INDEX_DIRECTORY *ob = new ITCH_INDEX_DIRECTORY;
		memcpy(ob, o, SZ_ITCH_INDEX_DIRECTORY);
		iMap.insert(_INDEX_MAP_::value_type(ob->orderbookId, ob));
	}
	else
	{
		ITCH_INDEX_DIRECTORY *ob = i->second;
		memcpy(ob, o, SZ_ITCH_INDEX_DIRECTORY);
	}
}

ITCH_INDEX_DIRECTORY *COrderBookDirectoryMap::GetIndex(uint32_t orderBookId)
{
	_INDEX_MAP_::const_iterator i = iMap.find(orderBookId);
	return (i == iMap.end()) ? NULL : i->second;
}

