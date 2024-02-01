#ifndef __ORDERBOOK_H__
#define __ORDERBOOK_H__

#include <map>
#include <stdio.h>
#include <string>
// #include "header/main.h"
typedef int OL_INT32;
typedef unsigned int OL_UINT32;
typedef unsigned long long OL_UINT64; // untuk OL_UINT64
typedef unsigned char OL_UINT8;
typedef long long OL_INT64;

class COrderBookMap
{
	class COrderBookDetail;
	class COrderDetail
	{
	public:
		COrderDetail(OL_UINT64 orderId, OL_UINT8 cmd, OL_UINT32 price, OL_UINT64 lVolume, OL_UINT64 lBalance, OL_UINT8 status, OL_UINT64 linkedId = 0);
		~COrderDetail();

		OL_UINT8 command; // 0 : Bid; 1 : Offer;
		OL_UINT32 lPrice;
		OL_UINT64 lOrderId;
		OL_UINT64 LinkedId; // References to old Order Id
		OL_UINT64 lVolume;
		OL_UINT64 lBalance;
		OL_UINT8 lStatus; // 0:Open; 1:Withdraw/Amend; 2:Matched
		OL_UINT8 status;
		COrderBookDetail *pparent;
	};
	class COrderBookDetail
	{
	public:
		COrderBookDetail(const char *stockCode, const char *board);
		~COrderBookDetail();

		char szStockCode[32];
		char szBoard[6];
		OL_UINT32 lPrice;
		OL_INT64 lVolume;
		OL_INT32 lNQueue;
		// OL_INT32 status;

		void Release();
		COrderDetail *AddNewOrder(OL_UINT64 orderId, OL_UINT8 cmd, OL_UINT32 price, OL_INT64 volume, OL_INT64 balance, OL_UINT64 linkedId = 0);
		COrderDetail *WithdrawOrder(OL_UINT64 orderId, OL_INT64 volume, OL_INT64 balance);

	private:
		typedef std::map<OL_UINT64, COrderDetail *> _ORDER;
		_ORDER order;
	};

	class COrderBook
	{
	public:
		COrderBook(const char *stockCode, const char *board);
		virtual ~COrderBook();

		void Release();

		char szStockCode[32];
		char szBoard[8];
		char offer[8];

		COrderBookDetail *AddNewBid(OL_UINT64 orderId, OL_UINT32 price, OL_INT64 volume, OL_INT64 balance, OL_UINT64 linkedId);
		COrderBookDetail *AddNewOffer(OL_UINT64 orderId, OL_UINT32 price, OL_INT64 volume, OL_INT64 balance, OL_UINT64 linkedId);

		COrderBookDetail *WithdrawOrder(OL_UINT64 orderId, OL_INT64 balance);

		COrderBookDetail *TradedOrder(OL_UINT32 price, OL_INT64 volume, OL_UINT64 bOrderId, OL_UINT64 sOrderd);

	private:
		typedef std::map<OL_UINT64, COrderDetail *> _ORDERMAP;
		typedef std::map<OL_UINT32, COrderBookDetail *> _OBMAP;
		_OBMAP bid;
		_OBMAP offer;
		_ORDERMAP omap;
	};

public:
	COrderBookMap();
	virtual ~COrderBookMap();

	void Release();

	COrderBookDetail *GetBidOrderBookDetail(const char *stockCode, const char *board, OL_UINT32 price);
	COrderBookDetail *GetOfferOrderBookDetail(const char *stockCode, const char *board, OL_UINT32 price);

	int AddNewBidOrder(const char *stockCode, const char *board, OL_UINT64 orderId, OL_UINT32 price, OL_INT64 volume, OL_INT64 balance, OL_UINT64 linkedId);
	int AddNewOfferOrder(const char *stockCode, const char *board, OL_UINT64 orderId, OL_UINT32 price, OL_INT64 volume, OL_INT64 balance, OL_UINT64 linkedId);

	int WithdrawOrder(const char *stockCode, const char *board, OL_UINT64 orderId, OL_INT64 balance);

	int TradedOrder(const char *stockCode, const char *board, OL_UINT32 price, OL_INT64 volume, OL_UINT64 bOrderId, OL_UINT64 sOrderId);

private:
	typedef std::map<std::string, COrderBook *> _SMAP;
	_SMAP sm;
};

#endif
