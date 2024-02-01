#ifndef __IDX_DB_H__
#define __IDX_DB_H__

#include "main.h"
/*
#include <db_cxx.h>

int compare_dbkey_int32(Db *dbp, const Dbt *k1, const Dbt *k2)
{
	OL_INT32 a, b;
	memcpy(&a, k1->get_data(), sizeof(OL_INT32));
	memcpy(&b, k2->get_data(), sizeof(OL_INT32));
	return (int)(a - b);
};
*/
typedef struct __IDX_NEWS__
{
	OL_UINT32 ldate;
	OL_UINT32 ltime;
	OL_UINT32 lseq;
	OL_UINT32 lNewsId;
	char szSubject[13];
	char szHeadline[41];
	char szContent[2001];
	
} IDX_NEWS;
#define SZ_IDX_NEWS sizeof(IDX_NEWS)

typedef struct __IDX_DAILY_INDEX__
{
	OL_UINT32 ldate;
	OL_UINT64 lIndex;	// 4d
	OL_UINT64 lPrevIndex;	// 4d
	OL_UINT64 lOpenIndex;	// 4d
	OL_UINT64 lHighIndex;	// 4d
	OL_UINT64 lLowIndex;
	OL_INT32  lChange;	// 4d
	OL_INT32  lChangeRate;	// 2d
	OL_UINT32 lFreq;
	OL_UINT64 lVolume;
	OL_UINT64 lValue;
	OL_UINT32 lFgBuyFreq;
	OL_UINT32 lFgSellFreq;
	OL_UINT64 lFgBuyVolume;
	OL_UINT64 lFgSellVolume;
	OL_UINT64 lFgBuyValue;
	OL_UINT64 lFgSellValue;
} IDX_DAILY_INDEX;
#define SZ_IDX_DAILY_INDEX sizeof(IDX_DAILY_INDEX)

typedef struct __IDX_STOCK_DAILY__
{
	OL_UINT32 lPrevPrice;
	OL_UINT32 lOpenPrice;
	OL_UINT32 lHighPrice;
	OL_UINT32 lLowPrice;
	OL_UINT32 lClosePrice;
	OL_UINT32 lAvgPrice;
	OL_INT32  lChange;
	OL_INT32  lChangeRate;
	OL_INT32  lPrevChange;
	OL_INT32  lPrevChangeRate;
	OL_UINT32 lFreq;
	OL_UINT64 lVolume;
	OL_UINT64 lValue;
	OL_UINT64 lShareIndex;
	OL_UINT32 lFgBuyFreq;
	OL_UINT32 lFgSellFreq;
	OL_UINT64 lFgBuyVolume;
	OL_UINT64 lFgSellVolume;
	OL_UINT64 lFgBuyValue;
	OL_UINT64 lFgSellValue;
	OL_UINT32 lSuspendedDate;	// 0 : Unsuspend
} IDX_STOCK_DAILY;
#define SZ_IDX_STOCK_DAILY sizeof(IDX_STOCK_DAILY)

typedef struct __IDX_BROKER_SUM_DAILY__
{
	OL_UINT32 lDate;
	OL_UINT32 lBuyFreq;
	OL_UINT32 lSellFreq;
	OL_UINT64 lBuyVolume;
	OL_UINT64 lSellVolume;
	OL_UINT64 lBuyValue;
	OL_UINT64 lSellValue;
	OL_UINT32 lFgBuyFreq;
	OL_UINT32 lFgSellFreq;
	OL_UINT64 lFgBuyVolume;
	OL_UINT64 lFgSellVolume;
	OL_UINT64 lFgBuyValue;
	OL_UINT64 lFgSellValue;
	OL_UINT32 lSuspendedDate;
} IDX_BROKER_SUM_DAILY;
#define SZ_IDX_BROKER_SUM_DAILY sizeof(IDX_BROKER_SUM_DAILY)

typedef struct __IDX_SECTOR_DAILY__
{
	OL_UINT32 lDate;
	OL_UINT32 lTotalFreq;
	OL_UINT64 lTotalVolume;
	OL_UINT64 lTotalValue;
	OL_UINT32 lFgBuyFreq;
	OL_UINT64 lFgBuyVolume;
	OL_UINT64 lFgBuyValue;
	OL_UINT32 lFgSellFreq;
	OL_UINT64 lFgSellVolume;
	OL_UINT64 lFgSellValue;
} IDX_SECTOR_DAILY;
#define SZ_IDX_SECTOR_DAILY sizeof(IDX_SECTOR_DAILY)

typedef struct __IDX_SECTOR_DAILY__ IDX_BOARD_DAILY;
#define SZ_IDX_BOARD_DAILY sizeof(IDX_BOARD_DAILY)

typedef struct __IDX_STOCK_ORDER_TICK__
{
	OL_UINT32 lOrderTime;
	OL_UINT64 lOrderId;
	OL_UINT8  command;	// 0:Buy; 1:Sell; 2:Amend/Cancel Bid; 3:Amend/Cancel Offer;
	OL_UINT32 lPrice;
	OL_UINT64 lVolume;
	OL_UINT64 lBalance;
	OL_UINT8  invType;	// 'D':Domestic; 'F':Foreign;
	OL_UINT64 lLinkedId;
	OL_UINT32 lCancelTime;
	char szStockCode[13];
	char szBoard[6];
	char szBrokerId[6];
} IDX_STOCK_ORDER_TICK;
#define SZ_IDX_STOCK_ORDER_TICK sizeof(IDX_STOCK_ORDER_TICK)

typedef struct __IDX_STOCK_TRADE_TICK__
{
	OL_UINT32 lTradeTime;
	OL_UINT64 lTradeId;
	OL_UINT64 lBOrderId;
	OL_UINT64 lSOrderId;
	OL_UINT32 lPrice;
	OL_UINT64 lVolume;
	OL_INT32  lChange;
	OL_INT32  lChangeRate;
	OL_UINT8  command;	// 0:Matched; 1:Withdrawn;
	char szStockCode[13];
	char szBoard[6];
	char szBBrokerId[6];	// Buyer Broker Id
	char szSBrokerId[6];	// Seller Broker Id
	char bInvType;		// Buyer Investor Type
	char sInvType;		// Seller Investor Type
} IDX_STOCK_TRADE_TICK;
#define SZ_IDX_STOCK_TRADE_TICK sizeof(IDX_STOCK_TRADE_TICK)

typedef struct __IDX_INDEX_TICK__
{
	OL_UINT32 lTime;
	OL_UINT64 lIndex;	// 4d
	OL_INT32  lChange;	// 4d
	OL_INT32  lChangeRate;	// 4d
	OL_UINT64 lOpen;	// 4d
	OL_UINT64 lHigh;	// 4d
	OL_UINT64 lLow;		// 4d
	OL_UINT64 lBaseValue;
	OL_UINT64 lMarketValue;
} IDX_INDEX_TICK;
#define SZ_IDX_INDEX_TICK sizeof(IDX_INDEX_TICK)

#endif
