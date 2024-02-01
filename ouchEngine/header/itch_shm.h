#ifndef __ITCH_SHM_H__
#define __ITCH_SHM_H__

#include "constants.h"

#define SHM_KEY_ITCH 0x888
#define SEM_KEY_ITCH 0x888

#define IDX_MAX_ISSUER 2000
#define IDX_MAX_STOCK 2000	// Last Total Stocks -> 456
#define IDX_MAX_BROKER 160	// Last Total Broker -> 121
#define IDX_MAX_INDICES 64	// Last Total Indices -> 18
#define IDX_MAX_INDEX_MEMBER_DIRECTORY 2000
#define IDX_MAX_ORDER_BOOK 500
#define IDX_MAX_TRADE_BOOK 300
#define IDX_MAX_BOARD 3
#define IDX_MAX_SECTOR 200	// Last 20220207 Total Sector -> 130
#define IDX_MAX_STOCK_TYPE 50
#define IDX_MAX_CORPORATE_ACTION 4
#define IDX_MAX_ORDERBOOK_DIRECTORY 12000
#define IDX_MAX_FUTURES_DIRECTORY 120
#define IDX_MAX_OPTIONS_DIRECTORY 120
#define IDX_MAX_ORDERS 20000000
#define IDX_MAX_TRADES 20000000
//#define IDX_MAX_ORDERS 8000000
//#define IDX_MAX_TRADES 5000000

#define APP_MAX_HASH 1000

typedef struct _ISHM_PRICE_TICK_SIZE_TABLE_
{
	OL_UINT32 tickSizeTableId;
	OL_UINT32 tickSize;
	OL_UINT32 priceStart;
} ISHM_PRICE_TICK_SIZE_TABLE;
#define SZ_ISHM_PRICE_TICK_SIZE_TABLE sizeof(ISHM_PRICE_TICK_SIZE_TABLE)

typedef struct _ISHM_QTY_TICK_SIZE_TABLE_
{
	OL_UINT32 tickSizeTableId;
	OL_UINT64 tickSize;
	OL_UINT64 qtyStart;
} ISHM_QTY_TICK_SIZE_TABLE;
#define SZ_ISHM_QTY_TICK_SIZE_TABLE sizeof(ISHM_QTY_TICK_SIZE_TABLE)

typedef struct _ISHM_ISSUER_DIRECTORY_
{
	OL_UINT32 issuerId;
	char szIssuerCode[32];
	char szIssuerName[108];
} ISHM_ISSUER_DIRECTORY;
#define SZ_ISHM_ISSUER_DIRECTORY sizeof(ISHM_ISSUER_DIRECTORY)

typedef struct _ISHM_SECTOR_SUMMARY_
{
	OL_CHAR szId[8];
	OL_CHAR szSector[64];
	OL_UINT32 lAUSeq;
	OL_UINT32 lTotalFreq;
	OL_UINT64 lTotalVolume;
	OL_UINT64 lTotalValue;
	OL_UINT32 lFgBuyFreq;
	OL_UINT32 lFgSellFreq;
	OL_UINT64 lFgBuyVolume;
	OL_UINT64 lFgSellVolume;
	OL_UINT64 lFgBuyValue;
	OL_UINT64 lFgSellValue;
} ISHM_SECTOR_SUMMARY;
#define SZ_ISHM_SECTOR_SUMMARY sizeof(ISHM_SECTOR_SUMMARY)

typedef struct _ISHM_SECTOR_DIRECTORY_
{
	OL_UINT32 sectorId;
	char szSectorName[32];
	char szIndustryName[32];
	ISHM_SECTOR_SUMMARY sectorSummary;
} ISHM_SECTOR_DIRECTORY;
#define SZ_ISHM_SECTOR_DIRECTORY sizeof(ISHM_SECTOR_DIRECTORY)

typedef struct _ISHM_ORDERBOOK_DIRECTORY_
{
	OL_UINT32 orderbookId;
	OL_UINT32 issuerId;
	OL_UINT32 ipoPrice;
	OL_UINT64 tradeableShares;
	OL_UINT64 listedShares;
	OL_UINT32 sharesPerLot;
	OL_UINT64 minQty;
	OL_UINT32 qtyTickSizeTableId;
	OL_UINT32 qtyDecimals;
	OL_UINT32 priceTickSizeTableId;
	OL_UINT32 priceDecimals;
	OL_UINT32 delistingMatureDate;	//	yyyymmdd
	OL_UINT32 delistingTime;		//	hhmmss
	OL_UINT32 foreignLimit;
	OL_UINT32 sectorId;
	//const OL_UINT8 TRADING_ACTION_STATE_ACTIVE = 'T';
	//const OL_UINT8 TRADING_ACTION_STATE_SUSPENDED = 'V';
	//const OL_UINT8 TRADING_ACTION_REASON_NORMAL_TRADING = 'N';
	OL_UINT8 tradingState;
	OL_UINT8 tradingReason;
	//	------------------------------------
	char szISIN[16];
	char szSecurityCode[32];
	char szSecurityName[108];
	char szCurrency[4];
	char szGroup[12];
	char szInstrument[16];
	char szRemark0[16];
	char szRemark1[64];
} ISHM_ORDERBOOK_DIRECTORY;
#define SZ_ISHM_ORDERBOOK_DIRECTORY sizeof(ISHM_ORDERBOOK_DIRECTORY)

typedef struct _ISHM_FUTURES_DIRECTORY_
{
	OL_UINT32 orderbookId;
	OL_UINT32 underlyingId;	//	OrderbookId of the underlying security
	OL_UINT32 qtyTickSizeTableId;
	OL_UINT32 priceTickSizeTableId;
	OL_UINT32 priceDecimals;
	OL_UINT64 expiryDate;	//	yyyymmddhhmmss
	OL_UINT64 contractSize;
	//const OL_UINT8 TRADING_ACTION_STATE_ACTIVE = 'T';
	//const OL_UINT8 TRADING_ACTION_STATE_SUSPENDED = 'V';
	//const OL_UINT8 TRADING_ACTION_REASON_NORMAL_TRADING = 'N';
	OL_UINT8 tradingState;
	OL_UINT8 tradingReason;
	char szISIN[16];
	char szSecurityCode[32];
	char szCurrency[4];
	char szGroup[12];
	char szRemark0[16];
	char szRemark1[64];
} ISHM_FUTURES_DIRECTORY;
#define SZ_ISHM_FUTURES_DIRECTORY sizeof(ISHM_FUTURES_DIRECTORY)

typedef struct _ISHM_OPTIONS_DIRECTORY_
{
	OL_UINT32 orderbookId;
	OL_UINT32 underlyingId;	//	OrderbookId of the underlying security
	OL_UINT32 qtyTickSizeTableId;
	OL_UINT32 priceTickSizeTableId;
	OL_UINT32 priceDecimals;
	OL_UINT32 strikePrice;
	OL_UINT64 expiryDate;	//	yyyymmddhhmmss
	OL_UINT64 contractSize;
	OL_UINT8 verb;	//	'P'ut or 'C'all;
	//const OL_UINT8 TRADING_ACTION_STATE_ACTIVE = 'T';
	//const OL_UINT8 TRADING_ACTION_STATE_SUSPENDED = 'V';
	//const OL_UINT8 TRADING_ACTION_REASON_NORMAL_TRADING = 'N';
	OL_UINT8 tradingState;
	OL_UINT8 tradingReason;
	char szISIN[16];
	char szSecurityCode[32];
	char szCurrency[4];
	char szGroup[12];
	char szRemark0[16];
	char szRemark1[64];
} ISHM_OPTIONS_DIRECTORY;
#define SZ_ISHM_OPTIONS_DIRECTORY sizeof(ISHM_OPTIONS_DIRECTORY)

typedef struct _ISHM_INDEX_MEMBER_DIRECTORY_
{
	OL_UINT32 memberOrderbookId;
	OL_UINT32 orderbookId;
	OL_UINT64 weight;
} ISHM_INDEX_MEMBER_DIRECTORY;
#define SZ_ISHM_INDEX_MEMBER_DIRECTORY sizeof(ISHM_INDEX_MEMBER_DIRECTORY)

typedef struct _ISHM_INDEX_DIRECTORY_
{
	OL_UINT32 orderbookId;
	OL_UINT32 priceDecimals;
	char szSecurityCode[32];
	//const OL_UINT8 TRADING_ACTION_STATE_ACTIVE = 'T';
	//const OL_UINT8 TRADING_ACTION_STATE_SUSPENDED = 'V';
	//const OL_UINT8 TRADING_ACTION_REASON_NORMAL_TRADING = 'N';
	OL_UINT8 tradingState;
	OL_UINT8 tradingReason;
	OL_UINT32 lTotalIndexMemberDirectory;
	char szHashKey[34];
	ISHM_INDEX_MEMBER_DIRECTORY indexMemberDirectory[IDX_MAX_INDEX_MEMBER_DIRECTORY];
} ISHM_INDEX_DIRECTORY;
#define SZ_ISHM_INDEX_DIRECTORY sizeof(ISHM_INDEX_DIRECTORY)



typedef struct _IDX_BROKER_SUMMARY_
{
	OL_UINT32 lBuyAvgPrice;		// 2d
	OL_UINT32 lSellAvgPrice;	// 2d
	OL_UINT32 lBuyFreq;
	OL_UINT32 lSellFreq;
	OL_UINT64 lBuyVolume;		// Shares
	OL_UINT64 lSellVolume;		// Shares
	OL_UINT64 lBuyValue;
	OL_UINT64 lSellValue;
	OL_UINT32 lFgBuyAvgPrice;	// 2d
	OL_UINT32 lFgSellAvgPrice;	// 2d
	OL_UINT32 lFgBuyFreq;
	OL_UINT32 lFgSellFreq;
	OL_UINT64 lFgBuyVolume;		// Shares
	OL_UINT64 lFgSellVolume;	// Shares
	OL_UINT64 lFgBuyValue;
	OL_UINT64 lFgSellValue;
	char szBrokerId[8];
} IDX_BROKER_SUMMARY;
#define SZ_IDX_BROKER_SUMMARY sizeof(IDX_BROKER_SUMMARY)

typedef struct _ISHM_PARTICIPANT_DIRECTORY_
{
	OL_UINT32 participantId;
	OL_UINT8 participantStatus;		//	'A'ctive or 'S'uspended;
	char szParticipantCode[4];
	char szParticipantName[64];
} ISHM_PARTICIPANT_DIRECTORY;
#define SZ_ISHM_PARTICIPANT_DIRECTORY sizeof(ISHM_PARTICIPANT_DIRECTORY)





typedef struct _IDX_TRADE_BOOK_
{
	OL_UINT32 lPrice;	// 2d
	OL_UINT32 lFreq;	// (BFreq + SFreq)
	OL_UINT64 lVolume;	// Shares (BVolume + SVolume)
	OL_UINT64 lValue;
	OL_UINT32 lBFreq;
	OL_UINT32 lSFreq;
	OL_UINT64 lBVolume;
	OL_UINT64 lSVolume;
	OL_UINT64 lBValue;
	OL_UINT64 lSValue;
} IDX_TRADE_BOOK;
#define SZ_IDX_TRADE_BOOK sizeof(IDX_TRADE_BOOK)


typedef struct _IDX_ORDER_BOOK_DETAIL_
{
	OL_UINT32 lPrice;
	OL_INT32 lNQueue;	// Jumlah antrian
	OL_INT64 lVolume;
	OL_INT64 lFgVolume;
	OL_INT64 lDelta;
	OL_UINT64 lastTradedOrderId;
} IDX_ORDER_BOOK_DETAIL;
#define SZ_IDX_ORDER_BOOK_DETAIL sizeof(IDX_ORDER_BOOK_DETAIL)

typedef struct _IDX_ORDER_BOOK_
{
	OL_UINT32 lAUSeq;
	OL_UINT32 lUpdateFreq;
	OL_UINT16 lBidDepth;
	OL_UINT16 lOfferDepth;
	IDX_ORDER_BOOK_DETAIL bid[IDX_MAX_ORDER_BOOK];
	IDX_ORDER_BOOK_DETAIL offer[IDX_MAX_ORDER_BOOK];
} IDX_ORDER_BOOK;
#define SZ_IDX_ORDER_BOOK sizeof(IDX_ORDER_BOOK)

typedef struct _IDX_STOCK_DATA_
{
	OL_UINT32 lTradeTime; // 2d
	OL_UINT32 lLastPrice; // 2d
	OL_UINT32 lPrevPrice; // 2d
	OL_UINT32 lOpenPrice; // 2d
	OL_UINT32 lHighPrice; // 2d
	OL_UINT32 lLowPrice;  // 2d
	OL_UINT32 lAvgPrice;  // 2d
	OL_UINT32 lTradedFreq;
	OL_UINT32 lUpdateFreq;
	OL_UINT32 lIEPrice;		// 2d	Indicative Price
	OL_UINT32 lAUSeq;
	OL_INT32 lChange;	// 2d
	OL_INT32 lChangeRate;	// 2d
	OL_INT32 lPrevChange;
	OL_INT32 lPrevChangeRate;
	OL_UINT64 lShareIndex;
	OL_UINT64 lIEVolume;	//	Indicative Volume
	OL_UINT64 lTradedVolume;
	OL_UINT64 lTradedValue;
	OL_UINT64 lMarketCapital;
	OL_UINT32 lFgBuyFreq;
	OL_UINT32 lFgSellFreq;
	OL_UINT64 lFgBuyVolume;
	OL_UINT64 lFgSellVolume;
	OL_UINT64 lFgBuyValue;
	OL_UINT64 lFgSellValue;
	OL_UINT64 lFgTradeableShares;
	OL_UINT32 lBestBidPrice;
	OL_UINT32 lBestOfferPrice;
	OL_UINT64 lBestBidVolume;
	OL_UINT64 lBestOfferVolume;
	OL_UINT8 fStatus; // 0:Active; 1:Suspended; 2:Delisting (Manually Set);
	//const OL_UINT8 TRADING_ACTION_STATE_ACTIVE = 'T';
	//const OL_UINT8 TRADING_ACTION_STATE_SUSPENDED = 'V';
	//const OL_UINT8 TRADING_ACTION_REASON_NORMAL_TRADING = 'N';
	OL_UINT8 tradingState;
	OL_UINT8 tradingReason;
	OL_UINT32 lSuspendedDate;
	OL_UINT32 lTotalTradeBook;
	OL_UINT32 lTotalBrokerSummary;
	IDX_TRADE_BOOK tradeBook[IDX_MAX_TRADE_BOOK];
	IDX_BROKER_SUMMARY brokerSummary[IDX_MAX_BROKER];
} IDX_STOCK_DATA;
#define SZ_IDX_STOCK_DATA sizeof(IDX_STOCK_DATA)

typedef struct _IDX_CORPORATE_ACTION_
{
	OL_UINT32 caType;	// 0:Cash Dividend; 1:Bonus; 2:Stock Dividend; 3:Right Issue; 4:Stock Split; 5:Reverse Stock;
	OL_UINT32 lCumDate;
	OL_UINT32 lExDate;
	OL_UINT32 lRecordingDate;
	OL_UINT32 lDistributionDate;
	OL_UINT32 lRatio1;
	OL_UINT32 lRatio2;
	OL_UINT64 lAmount;	// 2d
} IDX_CORPORATE_ACTION;
#define SZ_CORPORATE_ACTION sizeof(IDX_CORPORATE_ACTION)

typedef struct _IDX_STOCK_
{
	OL_CHAR szStockCode[32];
	OL_CHAR szStockName[108];
	OL_CHAR szStockType[24];
	OL_CHAR fType;		// O:Ordinary; P:Preopen; R:Right; W:Warrant; M:Mutual Fund
	OL_UINT32 fStatus;	// Bit 0:Suspended; 1:Marginable; 2:Short Sell; 3:Preopening; 4:Right; 5: Warrant: 6:ETF; 7:Reverse Stock; 8:Stock Split; 31:Delisting (Manually Set)
	OL_UINT32 lSuspendedDate;
	OL_UINT32 lDelistingMatureDate;	//	yyyymmdd
	OL_UINT32 lDelistingTime;		//	hhmmss
	OL_UINT64 fIdxConstituent;	// This stock are constituent of which Index
	//OL_CHAR szSector[8];
	OL_CHAR	szISIN[16];
	OL_CHAR szInstrument[16];
	OL_CHAR szRemarks[16];
	OL_CHAR szRemarks2[64];
	OL_UINT32 lSectorId;	// 0:Undefined; 1:Agriculture; 2:Basic Industry; 3:Consumer; 4:Finance; 5:Infrastructure; 6:Manufacture; 7:Mining; 8:Misc Industry; 9:Property; 10:Trade;
	OL_UINT32 lIPOPrice;  // 2d
	OL_UINT32 lBasePrice; // 2d
	OL_UINT64 lListedShares;
	OL_UINT64 lTradeableListedShares;
	OL_UINT64 lFgTradeableShares;
	OL_UINT32 lSharesPerLot;
	OL_UINT32 orderbookId;
	IDX_ORDER_BOOK orderBook[IDX_MAX_BOARD];
	IDX_STOCK_DATA stockData[IDX_MAX_BOARD]; // 0:RG; 1:TN; 2:NG;
} IDX_STOCK;
#define SZ_IDX_STOCK sizeof(IDX_STOCK)

typedef struct _IDX_BOARD_
{
	OL_UINT32 lAUSeq;
	OL_UINT32 lTotalFreq;
	OL_UINT64 lTotalVolume;
	OL_UINT64 lTotalValue;
	OL_UINT32 lFgBuyFreq;
	OL_UINT32 lFgSellFreq;
	OL_UINT64 lFgBuyVolume;
	OL_UINT64 lFgSellVolume;
	OL_UINT64 lFgBuyValue;
	OL_UINT64 lFgSellValue;
} IDX_BOARD;
#define SZ_IDX_BOARD sizeof(IDX_BOARD)

typedef struct _BROKER_TRX_TRADE_BOOK_
{
	OL_UINT32 lPrice;
	OL_UINT32 lBuyFreq;
	OL_UINT32 lSellFreq;
	OL_UINT64 lBuyVolume;
	OL_UINT64 lSellVolume;
	OL_UINT64 lBuyValue;
	OL_UINT64 lSellValue;
} BROKER_TRX_TRADE_BOOK;
#define SZ_BROKER_TRX_TRADE_BOOK sizeof(BROKER_TRX_TRADE_BOOK)

typedef struct _BROKER_TRX_SUMM_
{
	char szStockCode[32];	// TLKM; TLKM.NG; TLKM.TN
	OL_UINT32 lBuyFreq;
	OL_UINT32 lSellFreq;
	OL_UINT32 lFgBuyFreq;
	OL_UINT32 lFgSellFreq;
	OL_UINT64 lBuyVolume;
	OL_UINT64 lBuyValue;
	OL_UINT64 lSellVolume;
	OL_UINT64 lSellValue;
	OL_UINT64 lFgBuyVolume;
	OL_UINT64 lFgSellVolume;
	OL_UINT64 lFgBuyValue;
	OL_UINT64 lFgSellValue;
	OL_UINT16 iBoard;
	OL_UINT16 lTotalTradeBook;
	BROKER_TRX_TRADE_BOOK tradeBook[IDX_MAX_TRADE_BOOK];
} BROKER_TRX_SUMM;
#define SZ_BROKER_TRX_SUMM sizeof(BROKER_TRX_SUMM)

typedef struct _IDX_BROKER_
{
	OL_CHAR szBrokerId[8];
	OL_CHAR szBrokerName[64];
	OL_UINT8 fStatus; // 0:Active; 1:Suspended
	OL_UINT32 participantId;
	OL_UINT32 lSuspendedDate;
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
	OL_UINT32 lUpdateFreq;
	OL_UINT32 lAUSeq;
	OL_UINT16 lTotalTrxSumm;
	BROKER_TRX_SUMM trxSummary[IDX_MAX_STOCK];
} IDX_BROKER;
#define SZ_IDX_BROKER sizeof(IDX_BROKER)

typedef struct _IDX_INDEX_
{
	char szIndexCode[32];
	char szIndexDesc[64];
	OL_UINT64 fConstituent;
	OL_UINT32 orderbookId;
	OL_UINT32 priceDecimals;
	OL_UINT32 lTime;
	OL_UINT32 lAUSeq;
	OL_UINT32 lSequence;
	OL_UINT64 lIndex; // 4d
	OL_UINT64 lPrevIndex; // 4d
	OL_UINT64 lOpenIndex; // 4d
	OL_UINT64 lHighIndex; // 4d
	OL_UINT64 lLowIndex;  // 4d
	OL_INT32  lChange; // 4d
	OL_INT32  lChangeRate; // 2d
	OL_UINT32 lFreq;
	OL_UINT64 lVolume;
	OL_UINT64 lValue;
	OL_UINT64 lBaseValue;
	OL_UINT64 lMarketValue;
	OL_UINT32 lUp;
	OL_UINT32 lDown;
	OL_UINT32 lUnchange;
	OL_UINT32 lNoTransaction;
	OL_UINT32 lFgBuyFreq;
	OL_UINT32 lFgSellFreq;
	OL_UINT64 lFgBuyVolume;
	OL_UINT64 lFgSellVolume;
	OL_UINT64 lFgBuyValue;
	OL_UINT64 lFgSellValue;
	//const OL_UINT8 TRADING_ACTION_STATE_ACTIVE = 'T';
	//const OL_UINT8 TRADING_ACTION_STATE_SUSPENDED = 'V';
	//const OL_UINT8 TRADING_ACTION_REASON_NORMAL_TRADING = 'N';
	OL_UINT8 tradingState;
	OL_UINT8 tradingReason;
} IDX_INDEX;
#define SZ_IDX_INDEX sizeof(IDX_INDEX)

//typedef struct _IDX_ORDER_DETAIL_
//{
//	OL_UINT32 lOrderTime;
//	OL_UINT64 lOrderId;
//	OL_UINT32 lPrice;
//	OL_UINT64 lVolume;
//	OL_UINT64 lBalance;
//	OL_UINT64 lLinkedId;
//	OL_UINT8 nationality;
//	OL_UINT8 command;	// 0:Bid; 1:Offer; 2:Cancel/Amended Bid; 3:Cancel/Amended Offer
//	char szStockCode[32];
//	char szBoard[8];
//	char szBrokerId[8];
//} IDX_ORDER_DETAIL;
//#define SZ_IDX_ORDER_DETAIL sizeof(IDX_ORDER_DETAIL)

const OL_UINT8 ORDER_STATUS_OPEN = 1;
const OL_UINT8 ORDER_STATUS_MATCHED = 2;
const OL_UINT8 ORDER_STATUS_AMENDED = 3;
const OL_UINT8 ORDER_STATUS_WITHDRAWN = 5;
typedef struct _IDX_ORDER_
{
	OL_UINT8 orderStatus;	// 1 : Open; 2 : Matched; 3 : Amended; 5 : Withdrawn;
	OL_UINT8 orderVerb;	//	'B'uy; 'S'ell;	Sett'L'ement Price Update; Blank if reference / index price update;
	OL_UINT8 domicile;	//	'I'ndonesia; 'A'sing; 'S'endiri; 'F'oreign House;
	OL_UINT32 timeStamp;//	hhmmss
	OL_UINT32 price;
	OL_UINT32 orderbookId;
	OL_UINT32 participantId;
	OL_UINT64 quantity;
	OL_UINT64 executedQty;
	OL_UINT64 orderNumber;
	OL_UINT64 oriOrderNumber;	//	Prev Order Number
} IDX_ORDER;
#define SZ_IDX_ORDER sizeof(IDX_ORDER)

typedef struct _IDX_TRADE_DETAIL_
{
	OL_UINT32 lTradeTime;
	OL_UINT64 lTradeId;
	OL_UINT64 lBOrderId;
	OL_UINT64 lSOrderId;
	OL_UINT32 lPrice;
	OL_UINT64 lVolume;
	OL_UINT64 lValue;
	OL_UINT8 bNationality;
	OL_UINT8 sNationality;
	char szStockCode[32];
	char szBoard[8];
	char szBuyer[8];
	char szSeller[8];
} IDX_TRADE_DETAIL;
#define SZ_IDX_TRADE_DETAIL sizeof(IDX_TRADE_DETAIL)

typedef struct _IDX_CURRENT_STATE_
{
	OL_UINT8 fStatus; // See IDX Doc
	OL_UINT32 idxDate;
	OL_UINT32 idxTime;
	OL_UINT32 timeStampSeconds;
	OL_UINT32 lSequence;
	OL_UINT32 lTotalIssuer;
	OL_UINT32 lTotalStock;
	OL_UINT32 lTotalOrderBook;
	OL_UINT32 lTotalFutures;
	OL_UINT32 lTotalOptions;
	OL_UINT16 lTotalBroker;
	OL_UINT16 lTotalParticipant;
	OL_UINT16 lTotalIndex;
	OL_UINT16 lTotalIndexDirectory;
	OL_UINT16 lTotalSector;
	OL_UINT32 lTotalFlawData;
	OL_UINT32 lTotalOrders;
	OL_UINT32 lTotalTrades;
	OL_UINT16 fTEStatus;
	OL_UINT32 lTotalWin64AppHashs;
	OL_CHAR szMessage[128];
	OL_CHAR szHashStockList[34];
	OL_CHAR szHashIndexList[34];
	OL_CHAR szHashSectorList[34];
	OL_CHAR szHashBrokerList[34];
	OL_CHAR szHashFuturesList[34];
	OL_CHAR szHashOptionsList[34];
	OL_CHAR szHashOrderBookList[34];
	OL_CHAR szHashPriceFractions[34];
	OL_CHAR szHashHolidays[34];
} IDX_CURRENT_STATE;
#define SZ_IDX_CURRENT_STATE sizeof(IDX_CURRENT_STATE)
/*
typedef struct _IDX_FD_CONTROL_
{
	OL_UINT32 lDfBin0_off;
	OL_UINT32 lDfBin1_off;
	OL_UINT32 lMisc_off;
	OL_UINT32 lTrade_off;
	OL_UINT32 lDbProcess_off;
} IDX_FD_CONTROL;
*/

typedef struct _APP_HASH_
{
	OL_CHAR szFileName[256];
	OL_CHAR szFileHash[34];
} APP_HASH;

typedef struct _ITCH_SHM_
{
	//IDX_FD_CONTROL fdControl;
	IDX_CURRENT_STATE idxState;
	IDX_INDEX idxIndices[IDX_MAX_INDICES];
	IDX_BROKER idxBroker[IDX_MAX_BROKER];
	IDX_BOARD idxBoard[IDX_MAX_BOARD];
	IDX_STOCK idxStock[IDX_MAX_STOCK];
	ISHM_SECTOR_DIRECTORY idxSector[IDX_MAX_SECTOR];
	ISHM_INDEX_DIRECTORY indexDirectory[IDX_MAX_INDICES];
	ISHM_ISSUER_DIRECTORY issuerDirectory[IDX_MAX_ISSUER];
	ISHM_PARTICIPANT_DIRECTORY participantDirectory[IDX_MAX_BROKER];
	ISHM_FUTURES_DIRECTORY futuresDirectory[IDX_MAX_FUTURES_DIRECTORY];
	ISHM_OPTIONS_DIRECTORY optionsDirectory[IDX_MAX_OPTIONS_DIRECTORY];
	ISHM_ORDERBOOK_DIRECTORY orderBookDirectory[IDX_MAX_ORDERBOOK_DIRECTORY];
	IDX_ORDER idxOrder[IDX_MAX_ORDERS];
	APP_HASH win64AppHash[APP_MAX_HASH];
	//IDX_TRADE_DETAIL idxTradeDetail[IDX_MAX_TRADE_DETAIL];
} ITCH_SHM;
#define SZ_ITCH_SHM sizeof(ITCH_SHM)

#endif
