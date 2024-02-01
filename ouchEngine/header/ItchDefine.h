#ifndef __ITCHDEFINE_H__
#define __ITCHDEFINE_H__

#include "constants.h"

const OL_UINT16 MAX_ITCH_PACKAGE_LENGTH = 5632;	//	5K + 512 bytes
//------------------------------------------------------------
//	ITCH Total View Message Type
//------------------------------------------------------------
const OL_UINT8 ITCH_TYPE_TIMESTAMP 					= 'T';	//	Number of seconds since midnight of the system start
const OL_UINT8 ITCH_TYPE_SYSTEM_EVENT_MSG			= 'S';
const OL_UINT8 ITCH_TYPE_ORDERBOOK_PRICE_TICK_TABLE = 'L';
const OL_UINT8 ITCH_TYPE_ORDERBOOK_QTY_TICK_TABLE	= 'M';
const OL_UINT8 ITCH_TYPE_SECTOR_DIRECTORY			= 'F';
const OL_UINT8 ITCH_TYPE_ORDERBOOK_DIRECTORY		= 'R';
const OL_UINT8 ITCH_TYPE_FUTURES_DIRECTORY			= 'J';
const OL_UINT8 ITCH_TYPE_OPTIONS_DIRECTORY			= 'W';
const OL_UINT8 ITCH_TYPE_PARTICIPANT_DIRECTORY		= 'K';
const OL_UINT8 ITCH_TYPE_TRADING_ACTION_MSG			= 'H';
const OL_UINT8 ITCH_TYPE_ADD_ORDER_MSG				= 'A';
const OL_UINT8 ITCH_TYPE_ADD_ORDER_WITH_PARTICIPANT	= 'a';
const OL_UINT8 ITCH_TYPE_ORDER_EXECUTED_MSG			= 'E';
const OL_UINT8 ITCH_TYPE_ORDER_EXECUTED_WITH_PRICE	= 'C';
const OL_UINT8 ITCH_TYPE_TRADE_MSG					= 'P';
const OL_UINT8 ITCH_TYPE_BROKEN_TRADE_MSG			= 'B';
const OL_UINT8 ITCH_TYPE_ORDER_DELETE_MSG			= 'D';
const OL_UINT8 ITCH_TYPE_ORDER_CLEAR_MSG 			= 'd';
const OL_UINT8 ITCH_TYPE_ORDER_REPLACE_MSG			= 'U';
const OL_UINT8 ITCH_TYPE_INDICATIVE_PRICE_QTY_MSG	= 'I';
const OL_UINT8 ITCH_TYPE_GLIMPSE_SNAPSHOT_MSG		= 'G';
const OL_UINT8 ITCH_TYPE_INDEX_DIRECTORY			= 'Z';
const OL_UINT8 ITCH_TYPE_INDEX_MEMBER_DIRECTORY		= 'O';
const OL_UINT8 ITCH_TYPE_ISSUER_DIRECTORY			= 'V';
const OL_UINT8 ITCH_TYPE_ORDER_RELINK_MSG			= 'r';
const OL_UINT8 ITCH_TYPE_BEST_BID_OFFER_MSG			= 'Q';
const OL_UINT8 ITCH_TYPE_NEWS_MSG					= 'N';
const OL_UINT8 ITCH_TYPE_PARTICIPANT_STATUS			= 'Y';

typedef struct __ITCH_TIME_STAMP__
{
	OL_UINT8  type;		//	ITCH_TIMESTAMP = 'T';
	OL_UINT32 seconds;	//	Number of seconds since midnight of the first day of the system cycle.
} ITCH_TIME_STAMP;


//	System Event Code
const OL_UINT8 SEC_START_OF_MSG				= 'O';
const OL_UINT8 SEC_START_OF_SYSTEM_HOURS	= 'S';
const OL_UINT8 SEC_START_OF_MARKET_HOURS	= 'Q';
const OL_UINT8 SEC_END_OF_MARKET_HOURS		= 'M';
const OL_UINT8 SEC_OPENING_AUCTION_START	= 'P';
const OL_UINT8 SEC_CLOSING_AUCTION_START	= 'K';
const OL_UINT8 SEC_SCHEDULED_AUCTION_START	= 'V';
const OL_UINT8 SEC_SCHEDULED_AUCTION_CLOSES	= 'U';
const OL_UINT8 SEC_START_OF_POST_TRADING	= 'T';
const OL_UINT8 SEC_SCHEDULED_BREAK_STARTS	= 'A';
const OL_UINT8 SEC_SCHEDULED_BREAK_ENDS		= 'B';
const OL_UINT8 SEC_END_OF_SYSTEM_HOURS		= 'E';
const OL_UINT8 SEC_END_OF_MESSAGES			= 'C';
const OL_UINT8 SEC_TRADING_SUSPENSION		= 'X';
const OL_UINT8 SEC_TRADING_ACTIVATION		= 'Y';

#define SYSTEM_EVENT_GROUP_RG "RG"
#define SYSTEM_EVENT_GROUP_TN "TN"
#define SYSTEM_EVENT_GROUP_NG "NG"
#define SYSTEM_EVENT_GROUP_RO "RO"
#define SYSTEM_EVENT_GROUP_RF "RF"

#define SYSTEM_EVENT_INSTRUMENT_ORDI "ORDI"
#define SYSTEM_EVENT_INSTRUMENT_ORDI_PREOPEN "ORDI_PREOPEN"
#define SYSTEM_EVENT_INSTRUMENT_ACCEL "ACCEL"
#define SYSTEM_EVENT_INSTRUMENT_WATCHLIST "WATCHLIST"
#define SYSTEM_EVENT_INSTRUMENT_MUTI "MUTI"
#define SYSTEM_EVENT_INSTRUMENT_RGHI "RGHI"
#define SYSTEM_EVENT_INSTRUMENT_WARI "WARI"
#define SYSTEM_EVENT_INSTRUMENT_S_WARI "S_WARI"
#define SYSTEM_EVENT_INSTRUMENT_WATCH_CALL "WATCH_CALL"
#define SYSTEM_EVENT_INSTRUMENT_WARI_CALL "WARI_CALL"
#define SYSTEM_EVENT_INSTRUMENT_RGHI_CALL "RGHI_CALL"

typedef struct __ITCH_SYSTEM_EVENT_MSG__
{
	OL_UINT8 type;		//	ITCH_SYSTEM_EVENT_MSG = 'S';
	OL_UINT8 eventCode;
	OL_UINT32 timeStamp;	//	Nanoseconds since last Time Stamp Seconds Msg
	OL_UINT32 orderbookId;	//	0 if applies at a System Level or Group Level
	char szGroup[12];
	char szInstrument[16];
} ITCH_SYSTEM_EVENT_MSG;

typedef struct __ITCH_PRICE_TICK_SIZE_TABLE__
{
	OL_UINT8 type;		//	ITCH_ORDERBOOK_PRICE_TICK_TABLE = 'L';
	OL_UINT32 timeStamp;
	OL_UINT32 tickSizeTableId;
	OL_UINT32 tickSize;
	OL_UINT32 priceStart;
} ITCH_PRICE_TICK_SIZE_TABLE;

typedef struct __ITCH_QTY_TICK_SIZE_TABLE__
{
	OL_UINT8 type;		//	ITCH_ORDERBOOK_QTY_TICK_TABLE = 'M';
	OL_UINT32 timeStamp;
	OL_UINT32 tickSizeTableId;
	OL_UINT64 tickSize;
	OL_UINT64 qtyStart;
} ITCH_QTY_TICK_SIZE_TABLE;

typedef struct __ITCH_ISSUER_DIRECTORY__
{
	OL_UINT8 type;		//	ITCH_ISSUER_DIRECTORY = 'V';
	OL_UINT32 timeStamp;
	OL_UINT32 issuerId;
	char szIssuerCode[32];
	char szIssuerName[64];
} ITCH_ISSUER_DIRECTORY;

typedef struct __ITCH_SECTOR_DIRECTORY__
{
	OL_UINT8 type;		//	ITCH_SECTOR_DIRECTORY = 'F';
	OL_UINT32 timeStamp;
	OL_UINT32 sectorId;
	char szSectorName[32];
	char szIndustryName[32];
} ITCH_SECTOR_DIRECTORY;

typedef struct __ITCH_ORDERBOOK_DIRECTORY__
{
//	__ITCH_ORDERBOOK_DIRECTORY__() : type(ITCH_TYPE_ORDERBOOK_DIRECTORY) {};
	OL_UINT8 type;		//	ITCH_ORDERBOOK_DIRECTORY = 'R';
	OL_UINT32 timeStamp;
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
	char szISIN[16];
	char szSecurityCode[32];
	char szSecurityName[108];
	char szCurrency[4];
	char szGroup[12];
	char szInstrument[16];
	char szRemark0[16];
	char szRemark1[64];
} ITCH_ORDERBOOK_DIRECTORY;

#define SZ_ITCH_ORDERBOOK_DIRECTORY sizeof(ITCH_ORDERBOOK_DIRECTORY)

typedef struct __ITCH_FUTURES_DIRECTORY__
{
//	__ITCH_FUTURES_DIRECTORY__() : type(ITCH_TYPE_FUTURES_DIRECTORY) {};
	OL_UINT8 type;		//	ITCH_TYPE_FUTURES_DIRECTORY = 'J';
	OL_UINT32 timeStamp;
	OL_UINT32 orderbookId;
	OL_UINT32 underlyingId;	//	OrderbookId of the underlying security
	OL_UINT32 qtyTickSizeTableId;
	OL_UINT32 priceTickSizeTableId;
	OL_UINT32 priceDecimals;
	OL_UINT64 expiryDate;	//	yyyymmddhhmmss
	OL_UINT64 contractSize;
	char szISIN[16];
	char szSecurityCode[32];
	char szCurrency[4];
	char szGroup[12];
	char szRemark0[16];
	char szRemark1[64];
} ITCH_FUTURES_DIRECTORY;

#define SZ_ITCH_FUTURES_DIRECTORY sizeof(ITCH_FUTURES_DIRECTORY)

typedef struct __ITCH_OPTIONS_DIRECTORY__
{
//	__ITCH_OPTIONS_DIRECTORY__ () : type(ITCH_TYPE_OPTIONS_DIRECTORY) {};
	OL_UINT8 type;		//	ITCH_TYPE_OPTIONS_DIRECTORY = 'W';
	OL_UINT8 verb;	//	'P'ut or 'C'all;
	OL_UINT32 timeStamp;
	OL_UINT32 orderbookId;
	OL_UINT32 underlyingId;	//	OrderbookId of the underlying security
	OL_UINT32 qtyTickSizeTableId;
	OL_UINT32 priceTickSizeTableId;
	OL_UINT32 priceDecimals;
	OL_UINT32 strikePrice;
	OL_UINT64 expiryDate;	//	yyyymmddhhmmss
	OL_UINT64 contractSize;
	char szISIN[16];
	char szSecurityCode[32];
	char szCurrency[4];
	char szGroup[12];
	char szRemark0[16];
	char szRemark1[64];
} ITCH_OPTIONS_DIRECTORY;

#define SZ_ITCH_OPTIONS_DIRECTORY sizeof(ITCH_OPTIONS_DIRECTORY)

typedef struct __ITCH_INDEX_DIRECTORY__
{
//	__ITCH_INDEX_DIRECTORY__ () : type(ITCH_TYPE_INDEX_DIRECTORY) {};
	OL_UINT8 type;		//	ITCH_TYPE_INDEX_DIRECTORY = 'Z';
	OL_UINT32 timeStamp;
	OL_UINT32 orderbookId;
	OL_UINT32 priceDecimals;
	char szSecurityCode[32];
} ITCH_INDEX_DIRECTORY;

#define SZ_ITCH_INDEX_DIRECTORY sizeof(ITCH_INDEX_DIRECTORY)

typedef struct __ITCH_INDEX_MEMBER_DIRECTORY__
{
//	__ITCH_INDEX_MEMBER_DIRECTORY__ () : type(ITCH_TYPE_INDEX_MEMBER_DIRECTORY) {};
	OL_UINT8 type;		//	ITCH_TYPE_INDEX_MEMBER_DIRECTORY = 'O';
	OL_UINT32 timeStamp;
	OL_UINT32 orderbookId;
	OL_UINT32 memberOrderbookId;
	OL_UINT64 weight;
} ITCH_INDEX_MEMBER_DIRECTORY;

typedef struct __ITCH_PARTICIPANT_DIRECTORY__
{
//	__ITCH_PARTICIPANT_DIRECTORY__ () : type (ITCH_TYPE_PARTICIPANT_DIRECTORY) {};
	OL_UINT8 type;		//	ITCH_TYPE_INDEX_MEMBER_DIRECTORY = 'K';
	OL_UINT32 timeStamp;
	OL_UINT32 participantId;
	char szParticipantCode[4];
	char szParticipantName[64];
} ITCH_PARTICIPANT_DIRECTORY;

const OL_UINT8 TRADING_ACTION_STATE_ACTIVE = 'T';
const OL_UINT8 TRADING_ACTION_STATE_SUSPENDED = 'V';
const OL_UINT8 TRADING_ACTION_REASON_NORMAL_TRADING = 'N';
const OL_UINT8 TRADING_ACTION_REASON_SUSPENDED_CB	= 'S';	//	Suspended dur to Circuit Break
typedef struct __ITCH_OB_TRADING_ACTION_MSG__
{
//	__ITCH_OB_TRADING_ACTION_MSG__ () : type (ITCH_TYPE_TRADING_ACTION_MSG) {};
	OL_UINT8 type;		//	ITCH_TYPE_TRADING_ACTION_MSG = 'H';
	OL_UINT32 timeStamp;
	OL_UINT32 orderbookId;
	OL_UINT8 tradingState;
	OL_UINT8 reason;
} ITCH_OB_TRADING_ACTION_MSG;

const OL_UINT8 ORDER_VERB_BUY = 'B';
const OL_UINT8 ORDER_VERB_SELL = 'S';
const OL_UINT8 ORDER_VERB_BLANK = ' ';
const OL_UINT8 ORDER_VERB_SETTLEMENT_PRICE_UPDATE = 'L';
const OL_UINT32 MARKET_ORDER_PRICE = 0x7fffffff;

const OL_UINT8 ITCH_DOMICILE_INDONESIA		= 'I';	//	'I'ndonesian
const OL_UINT8 ITCH_DOMICILE_ASING			= 'A';	//	'A'sing
const OL_UINT8 ITCH_DOMICILE_SELF			= 'S';	//	'S'endiri
const OL_UINT8 ITCH_DOMICILE_FOREIGN_HOUSE	= 'F';	//	'F'oreign house
typedef struct __ITCH_ADD_ORDER_MSG__
{
//	__ITCH_ADD_ORDER_MSG__ () : type (ITCH_TYPE_ADD_ORDER_MSG) {};
	OL_UINT8 type;		//	ITCH_TYPE_ADD_ORDER_MSG	= 'A';
	OL_UINT8 orderVerb;	//	'B'uy; 'S'ell;	Sett'L'ement Price Update; Blank if reference / index price update;
	OL_UINT8 domicile;	//	'I'ndonesia; 'A'sing; 'S'endiri; 'F'oreign House;
	OL_UINT32 timeStamp;
	OL_UINT32 price;
	OL_UINT32 orderbookId;
	OL_UINT64 quantity;
	OL_UINT64 orderNumber;
} ITCH_ADD_ORDER_MSG;

typedef struct __ITCH_ADD_ORDER_WITH_PARTICIPANT_MSG__
{
//	__ITCH_ADD_ORDER_WITH_PARTICIPANT_MSG__ () : type (ITCH_TYPE_ADD_ORDER_WITH_PARTICIPANT) {};
	OL_UINT8 type;		//	ITCH_TYPE_ADD_ORDER_WITH_PARTICIPANT = 'a';
	OL_UINT8 orderVerb;	//	'B'uy; 'S'ell;	Sett'L'ement Price Update; Blank if reference / index price update;
	OL_UINT8 domicile;	//	'I'ndonesia; 'A'sing; 'S'endiri; 'F'oreign House;
	OL_UINT32 timeStamp;
	OL_UINT32 price;
	OL_UINT32 orderbookId;
	OL_UINT32 participantId;
	OL_UINT64 quantity;
	OL_UINT64 orderNumber;
} ITCH_ADD_ORDER_WITH_PARTICIPANT_MSG;

typedef struct __ITCH_ORDER_EXECUTED_MSG__
{
//	__ITCH_ORDER_EXECUTED_MSG__ () : type (ITCH_TYPE_ORDER_EXECUTED_MSG) {};
	OL_UINT8 type;				//	ITCH_TYPE_ORDER_EXECUTED_MSG = 'E';
	OL_UINT8 tradeIndicator;	//	'R'egular Trade; 'U'nintentional self-cross
	OL_UINT32 timeStamp;
	OL_UINT64 orderNumber;
	OL_UINT64 executedQty;
	OL_UINT64 matchNumber;
	OL_UINT32 buyParticipantId;
	OL_UINT32 sellParticipantId;
	OL_UINT8 buyDomicile;
	OL_UINT8 sellDomicile;
} ITCH_ORDER_EXECUTED_MSG;

const OL_UINT8 TRADE_MSG_PRINTABLE = 'Y';
const OL_UINT8 TRADE_MSG_NON_PRINTABLE = 'N';
const OL_UINT8 TRADE_INDICATOR_REGULAR = 'R';		//	'R'egular Trade;
const OL_UINT8 TRADE_INDICATOR_SELF_CROSS = 'U';	//	'U'nintentional self-cross;
const OL_UINT8 TRADE_INDICATOR_NEGDEAL = 'N';		//	'N'egDeal;
const OL_UINT8 TRADE_INDICATOR_REGULAR_TRADE = ' ';	//	Left blank if used to report a regular trade;
typedef struct __ITCH_ORDER_EXECUTED_WITH_PRICE_MSG__
{
//	__ITCH_ORDER_EXECUTED_WITH_PRICE_MSG__ () : type (ITCH_TYPE_ORDER_EXECUTED_WITH_PRICE) {};
	OL_UINT8 type;				//	ITCH_TYPE_ORDER_EXECUTED_WITH_PRICE	= 'C';
	OL_UINT8 tradeIndicator;	//	'R'egular Trade; 'U'nintentional self-cross
	OL_UINT8 printable;			//	'Y' : Printable; 'N' : Non Printable; Indicates if the execution should be reflected in volume calculation
	OL_UINT32 timeStamp;
	OL_UINT32 executionPrice;
	OL_UINT64 orderNumber;
	OL_UINT64 executedQty;
	OL_UINT64 matchNumber;
	OL_UINT32 buyParticipantId;
	OL_UINT32 sellParticipantId;
	OL_UINT8 buyDomicile;
	OL_UINT8 sellDomicile;
} ITCH_ORDER_EXECUTED_WITH_PRICE_MSG;

typedef struct __ITCH_TRADE_MSG__
{
//	__ITCH_TRADE_MSG__ () : type (ITCH_TYPE_TRADE_MSG) {};
	OL_UINT8 type;				//	ITCH_TYPE_TRADE_MSG	= 'P';
	OL_UINT8 printable;			//	'Y' : Printable; 'N' : Non Printable; Indicates if the execution should be reflected in volume calculation
	OL_UINT8 tradeIndicator;	//	'N'egDeal; Left blank if used to report a regular trade;
	OL_UINT32 timeStamp;
	OL_UINT32 orderbookId;
	OL_UINT32 executionPrice;
	OL_UINT64 executedQty;
	OL_UINT64 matchNumber;
	OL_UINT32 buyParticipantId;
	OL_UINT32 sellParticipantId;
	OL_UINT8 buyDomicile;
	OL_UINT8 sellDomicile;
} ITCH_TRADE_MSG;

const OL_UINT8 BROKEN_TRADE_REASON_SUPERVISORY	= 'S';	//	Supervisory - The trade was manually brokern by the Exchange
typedef struct __ITCH_BROKEN_TRADE_MSG__
{
//	__ITCH_BROKEN_TRADE_MSG__ () : type (ITCH_TYPE_BROKEN_TRADE_MSG) {};
	OL_UINT8 type;			//	ITCH_TYPE_BROKEN_TRADE_MSG = 'B';
	OL_UINT8 reason;		//	'S' : Supervisory - The trade was manually broken by the Exchange.
	OL_UINT32 timeStamp;
	OL_UINT64 matchNumber;
} ITCH_BROKEN_TRADE_MSG;

typedef struct __ITCH_ORDER_DELETE_MSG__
{
//	__ITCH_ORDER_DELETE_MSG__ () : type(ITCH_TYPE_ORDER_DELETE_MSG) {};
	OL_UINT8 type;			//	ITCH_TYPE_ORDER_DELETE_MSG = 'D';
	OL_UINT32 timeStamp;
	OL_UINT64 orderNumber;	//	The reference number of the order being canceled.
} ITCH_ORDER_DELETE_MSG;

typedef struct __ITCH_ORDER_CLEAR_MSG__
{
	OL_UINT8 type;		//	ITCH_TYPE_ORDER_CLEAR_MSG = 'd';
	OL_UINT32 timeStamp;
	OL_UINT32 orderbookId;
} ITCH_ORDER_CLEAR_MSG;

typedef struct __ITCH_ORDER_REPLACE_MSG__
{
//	__ITCH_ORDER_REPLACE_MSG__ () : type (ITCH_TYPE_ORDER_REPLACE_MSG) {};
	OL_UINT8 type;			//	ITCH_TYPE_ORDER_REPLACE_MSG	= 'U';
	OL_UINT32 timeStamp;
	OL_UINT64 oriOrderNumber;
	OL_UINT64 newOrderNumber;
	OL_UINT64 quantity;
	OL_UINT32 price;
} ITCH_ORDER_REPLACE_MSG;

typedef struct __ITCH_ORDER_RELINK_MSG__
{
//	__ITCH_ORDER_RELINK_MSG__ () : type (ITCH_TYPE_ORDER_RELINK_MSG) {};
	OL_UINT8 type;			//	ITCH_TYPE_ORDER_RELINK_MSG = 'r';
	OL_UINT32 timeStamp;
	OL_UINT64 orderNumber;
	OL_UINT64 exchgOrderNumber;
} ITCH_ORDER_RELINK_MSG;

const OL_UINT8 ITCH_INDICATIVE_CROSS_TYPE_OPENING_AUCTION	= 'P';	//	Opening Auction
const OL_UINT8 ITCH_INDICATIVE_CROSS_TYPE_INTRADAY_AUCTION	= 'I';	//	Intraday Auction
const OL_UINT8 ITCH_INDICATIVE_CROSS_TYPE_CLOSING_AUCTION	= 'K';	//	Closing Auction
const OL_UINT8 ITCH_INDICATIVE_CROSS_TYPE_CALL_AUCTION		= 'C';	//	Call Auction
typedef struct __ITCH_INDICATIVE_PRICE_QTY_MSG__
{
//	__ITCH_INDICATIVE_PRICE_QTY_MSG__ () : type (ITCH_TYPE_INDICATIVE_PRICE_QTY_MSG) {};
	OL_UINT8 type;			//	ITCH_TYPE_INDICATIVE_PRICE_QTY_MSG = 'I';
	OL_UINT8 crossType;		//	'P' : Opening Auction; 'I' : Intraday Auction; 'K' : Closing Auction;
	OL_UINT32 timeStamp;
	OL_UINT32 orderbookId;
	OL_UINT32 bestBidPrice;
	OL_UINT32 bestOfferPrice;
	OL_UINT32 theoreticalOpeningPrice;
	OL_UINT64 theoreticalOpeningQty;
	OL_UINT64 bestBidQtySize;
	OL_UINT64 bestOfferQtySize;
} ITCH_INDICATIVE_PRICE_QTY_MSG;

typedef struct __ITCH_GLIMPSE_SNAPSHOT_MSG__
{
	OL_UINT8 type;			//	ITCH_TYPE_GLIMPSE_SNAPSHOT_MSG = 'G';
	OL_UINT64 seqNumber;	//	The Itch Total View Sequence number when the GLIMPSE snapshot was taken.
} ITCH_GLIMPSE_SNAPSHOT_MSG;

typedef struct __ITCH_BEST_BID_OFFER_MSG__
{
	OL_UINT8 type;			//	ITCH_TYPE_BEST_BID_OFFER_MSG = 'Q';
	OL_UINT32 timeStamp;
	OL_UINT32 orderbookId;
	OL_UINT32 bestBidPrice;
	OL_UINT64 bestBidSize;
	OL_UINT32 bestOfferPrice;
	OL_UINT64 bestOfferSize;
} ITCH_BEST_BID_OFFER_MSG;

typedef struct __ITCH_NEWS_MSG__
{
	OL_UINT8 type;			//	ITCH_TYPE_NEWS_MSG = 'N';
	OL_UINT32 timeStamp;
	OL_UINT32 orderbookId;
	OL_UINT32 newsId;
	OL_UINT32 participantId;
	char szTitle[84];		//	NULL Terminated
	char szReference[256];	//	NULL Terminated
	char szNews[5004];		//	NULL Terminated
} ITCH_NEWS_MSG;

const OL_UINT8 PARTICIPANT_STATUS_ACTIVE	= 'A';
const OL_UINT8 PARTICIPANT_STATUS_SUSPENDED	= 'S';
typedef struct __ITCH_PARTICIPANT_STATUS__
{
	OL_UINT8 type;			//	ITCH_TYPE_PARTICIPANT_STATUS = 'Y';
	OL_UINT32 timeStamp;
	OL_UINT32 participantId;
	OL_UINT8 participantStatus;		//	'A'ctive or 'S'uspended;
} ITCH_PARTICIPANT_STATUS;

#endif
