#include "header/orderBook.h"
// #include "header/AdvancedOrder.h"
#include <stdio.h>
#include <string.h>
//

COrderBookMap::COrderDetail::COrderDetail(OL_UINT64 orderId, OL_UINT8 cmd, OL_UINT32 price, OL_UINT64 volume, OL_UINT64 balance, OL_UINT8 status, OL_UINT64 linkedId) : command(cmd), lPrice(price), lVolume(volume), lBalance(balance), lStatus(status)
{
	lOrderId = orderId;
	LinkedId = linkedId;
	pparent = NULL;
}

COrderBookMap::COrderBookDetail::COrderBookDetail(const char *stockCode, const char *board) : lPrice(0), lVolume(0), lNQueue(0)
{
	strcpy(szStockCode, stockCode);
	strcpy(szBoard, board);
}

COrderBookMap::COrderBookDetail::~COrderBookDetail()
{
	Release();
}

COrderBookMap::COrderDetail *COrderBookMap::COrderBookDetail::AddNewOrder(OL_UINT64 orderId, OL_UINT8 cmd, OL_UINT32 price, OL_INT64 volume, OL_INT64 balance, OL_UINT64 linkedId)
{
	COrderDetail *po = order[orderId];
	if (NULL == po)
	{
		po = new COrderDetail(orderId, cmd, price, volume, balance, 0, linkedId);
		order[orderId] = po;
		lPrice = price;
		lVolume += volume;
		++lNQueue;
		po->pparent = this;
	}
	else
	{
		po->command = cmd;
		po->lPrice = price;
		po->lVolume = volume;
		po->lBalance = balance;
		po->lStatus = 0;
		po->LinkedId = linkedId;
	}
	return po;
}

COrderBookMap::COrderDetail *COrderBookMap::COrderBookDetail::WithdrawOrder(OL_UINT64 orderId, OL_INT64 volume, OL_INT64 balance)
{
	COrderDetail *po = order[orderId];
	if (po)
	{
		po->lBalance = balance;
		po->lStatus = 1;
		lVolume -= balance;
		--lNQueue;
	}
	return po;
}

void COrderBookMap::COrderBookDetail::Release()
{
	for (_ORDER::iterator i = order.begin(); i != order.end(); ++i)
		delete static_cast<COrderDetail *>(i->second);
	order.clear();
}

COrderBookMap::COrderBook::COrderBook(const char *stockCode, const char *board)
{
	strcpy(szStockCode, stockCode);
	strcpy(szBoard, board);
}

COrderBookMap::COrderBook::~COrderBook()
{
	Release();
}

void COrderBookMap::COrderBook::Release()
{
	for (_OBMAP::iterator i = bid.begin(); i != bid.end(); ++i)
		delete static_cast<COrderBookDetail *>(i->second);
	bid.clear();
	for (_OBMAP::iterator i = offer.begin(); i != offer.end(); ++i)
		delete static_cast<COrderBookDetail *>(i->second);
	offer.clear();
	omap.clear();
}

COrderBookMap::COrderBookDetail *COrderBookMap::COrderBook::AddNewBid(OL_UINT64 orderId, OL_UINT32 price, OL_INT64 volume, OL_INT64 balance, OL_UINT64 linkedId)
{
	if (linkedId)
	{
		COrderDetail *po = omap[linkedId];
		if (po)
		{
			if (po->lPrice == price)
			{
				po->lPrice = price;
				po->lVolume = volume;
				po->lBalance = balance;
				po->LinkedId = linkedId;
				po->lOrderId = orderId;
				omap[orderId] = po;
				return po->pparent;
			}
		}
	}
	COrderBookDetail *pb = bid[price];
	if (NULL == pb)
	{
		pb = new COrderBookDetail(szStockCode, szBoard);
		bid[price] = pb;
	}
	COrderDetail *pd = pb->AddNewOrder(orderId, 0, price, volume, balance, 0);
	if (pd)
		omap[orderId] = pd;
	return pb;
}

COrderBookMap::COrderBookDetail *COrderBookMap::COrderBook::AddNewOffer(OL_UINT64 orderId, OL_UINT32 price, OL_INT64 volume, OL_INT64 balance, OL_UINT64 linkedId)
{
	if (linkedId)
	{
		COrderDetail *po = omap[linkedId];
		if (po)
		{
			if (po->lPrice == price)
			{
				po->lPrice = price;
				po->lVolume = volume;
				po->lBalance = balance;
				po->LinkedId = linkedId;
				po->lOrderId = orderId;
				omap[orderId] = po;
				return po->pparent;
			}
		}
	}
	COrderBookDetail *pb = offer[price];
	if (NULL == pb)
	{
		pb = new COrderBookDetail(szStockCode, szBoard);
		offer[price] = pb;
	}
	COrderDetail *pd = pb->AddNewOrder(orderId, 1, price, volume, balance, 0);
	if (pd)
		omap[orderId] = pd;
	return pb;
}

COrderBookMap::COrderBookDetail *COrderBookMap::COrderBook::WithdrawOrder(OL_UINT64 orderId, OL_INT64 balance)
{
	COrderDetail *po = omap[orderId];
	if (po)
	{
		po->status = 1;
		po->lBalance = balance;
		return po->pparent;
	}
	return NULL;
}

// COrderBookMap::COrderBookDetail *COrderBookMap::TradedOrder(OL_UINT32 price, OL_INT64 volume, OL_UINT64 bOrderId, OL_UINT64 sOrderId)
// {
// 	COrderDetail *po = omap[bOrderId]; // Buy Order
// 	if (po)
// 	{
// 	}
// 	po = omap[sOrderId]; // Sell Order
// 	if (po)
// 	{
// 	}
// }

COrderBookMap::COrderBookMap()
{
}

COrderBookMap::~COrderBookMap()
{
	Release();
}

void COrderBookMap::Release()
{
}

int COrderBookMap::AddNewBidOrder(const char *stockCode, const char *board, OL_UINT64 orderId, OL_UINT32 price, OL_INT64 volume, OL_INT64 balance, OL_UINT64 linkedId)
{
	char szCode[64];
	sprintf(szCode, "%s.%s", stockCode, board);
	COrderBook *po = sm[szCode];
	if (NULL == po)
	{
		po = new COrderBook(stockCode, board);
		sm[szCode] = po;
	}
	return po->AddNewBid(orderId, price, volume, balance, linkedId)->lNQueue;
}

int COrderBookMap::AddNewOfferOrder(const char *stockCode, const char *board, OL_UINT64 orderId, OL_UINT32 price, OL_INT64 volume, OL_INT64 balance, OL_UINT64 linkedId)
{
	char szCode[64];
	sprintf(szCode, "%s.%s", stockCode, board);
	COrderBook *po = sm[szCode];
	if (NULL == po)
	{
		po = new COrderBook(stockCode, board);
		sm[szCode] = po;
	}
	return po->AddNewOffer(orderId, price, volume, balance, linkedId)->lNQueue;
}

int COrderBookMap::WithdrawOrder(const char *stockCode, const char *board, OL_UINT64 orderId, OL_INT64 balance)
{
	char szCode[64];
	sprintf(szCode, "%s.%s", stockCode, board);
	COrderBook *po = sm[szCode];
	if (po)
		return po->WithdrawOrder(orderId, balance)->lNQueue;
	return 0;
}

COrderBookMap::COrderBookDetail *COrderBookMap::GetBidOrderBookDetail(const char *stockCode, const char *board, OL_UINT32 price)
{
	char szKey[64];
	sprintf(szKey, "%s.%s", stockCode, board);
	COrderBook *pob = sm[szKey];
	if (pob)
		return pob->bid[price];
	return NULL;
}

COrderBookMap::COrderBookDetail *COrderBookMap::GetOfferOrderBookDetail(const char *stockCode, const char *board, OL_UINT32 price)
{
	char szKey[64];
	sprintf(szKey, "%s.%s", stockCode, board);
	COrderBook *pob = sm[szKey];
	if (pob)
		return pob->offer[price];
	return NULL;
}

int COrderBookMap::TradedOrder(const char *stockCode, const char *board, OL_UINT32 price, OL_INT64 volume, OL_UINT64 bOrderId, OL_UINT64 sOrderId)
{
	char szKey[64];
	sprintf(szKey, "%s.%s", stockCode, board);
	COrderBook *pob = sm[szKey];
	if (pob)
		return pob->TradedOrder(price, volume, bOrderId, sOrderId);
	return 0;
}
