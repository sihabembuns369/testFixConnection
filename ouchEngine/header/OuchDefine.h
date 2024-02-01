#ifndef __OUCHDEFINE_H__
#define __OUCHDEFINE_H__

#include "constants.h"

const OL_UINT16 MAX_OUCH_PACKAGE_LENGTH = 1024;
//---------------------------------------------------------------
//	OUTCH Inbound Unsequence and Outbound Sequenced Message Type
//---------------------------------------------------------------
const OL_UINT8 OUCH_TYPE_ENTER_ORDER_MSG	= 'O';
const OL_UINT8 OUCH_TYPE_CANCEL_ORDER_MSG	= 'X';
const OL_UINT8 OUCH_TYPE_SYSTEM_EVENT_MSG	= 'S';
const OL_UINT8 OUCH_TYPE_ACCEPTED_ORDER_MSG	= 'A';
const OL_UINT8 OUCH_TYPE_REPLACED_ORDER_MSG	= 'U';
const OL_UINT8 OUCH_TYPE_CANCELED_ORDER_MSG	= 'C';
const OL_UINT8 OUCH_TYPE_EXECUTED_ORDER_MSG	= 'E';
const OL_UINT8 OUCH_TYPE_BROKEN_TRADE_MSG	= 'B';
const OL_UINT8 OUCH_TYPE_REJECTED_ORDER_MSG	= 'J';
const OL_UINT8 OUCH_TYPE_TRADING_LIMIT_MSG	= 'T';
const OL_UINT8 OUCH_TYPE_RESTATEMENT_MSG	= 'R';
//---------------------------------------------------------------

const OL_UINT8 VERB_BUY					= 'B';
const OL_UINT8 VERB_SELL 				= 'S';
const OL_UINT8 VERB_SHORT_SELL 			= 'T';
const OL_UINT8 VERB_MARGIN_BUY 			= 'M';
const OL_UINT8 VERB_PRICE_STABILIZATION	= 'P';

const OL_UINT8 STATE_LIVE = 'L';
const OL_UINT8 STATE_DEAD = 'D';

const OL_UINT32 TIF_IMMEDIATE	= 0;
const OL_UINT32 TIF_SESSION 	= 99997;
const OL_UINT32 TIF_DAY			= 99998;

const OL_UINT8 BROKEN_TRADE_CONSENT		= 'C';
const OL_UINT8 BROKEN_TRADE_SUPERVISORY	= 'S';

const OL_UINT8 TL_TYPE_POOL			= 'P';
const OL_UINT8 TL_TYPE_EQUITY 		= 'E';
const OL_UINT8 TL_TYPE_DERIVATIVE	= 'D';
const OL_UINT64 TL_NOT_SET_VALUE		= 0x7fffffffffffffff;

const OL_UINT8 DOMICILE_INDONESIA		= 'I';
const OL_UINT8 DOMICILE_ASING 			= 'A';
const OL_UINT8 DOMICILE_SENDIRI 		= 'S';
const OL_UINT8 DOMICILE_FOREIGN_HOUSE	= 'F';

const OL_UINT32 MAX_PRICE 		= 0x7fffffff;
const OL_UINT32 MAX_LIMIT_PRICE	= 0x7fffffff - 1;
const OL_UINT64 MAX_QUANTITY	= 0x7fffffffffffffff;
const OL_UINT64 MAX_LIMIT_QTY	= 0x7fffffffffffffff - 1;

//	--	Rejected Order Reason
const OL_UINT8 RO_NOT_TRADEABLE 		= 'H';	//	The orderbook, instrument, board or market is not tradeable.
const OL_UINT8 RO_INVALID_QTY			= 'Z';	//	Invalid quantity or quantity exceeds maximum limit.
const OL_UINT8 RO_INVALID_ORDER_BOOK	= 'S';	//	Invalid orderbook identifier.
const OL_UINT8 RO_NOT_TRADING_TIME		= 'R';	//	The order is not allowed at this time.
const OL_UINT8 RO_INVALID_PRICE 		= 'X';	//	Invalid price.
const OL_UINT8 RO_INVALID_MIN_QTY		= 'N';	//	Invalid Minimum Quantity.
const OL_UINT8 RO_INVALID_ORDER_TYPE	= 'Y';	//	Invalid Order Type. For example, the Time In Force value is not supported in current trading session.
const OL_UINT8 RO_FLOW_CONTROL			= 'F';	//	Flow control in place for user. A throttle is active for this OUCH port.
const OL_UINT8 RO_INVALID_ORDER_SRC		= 'B';	//	Order source is not valid.
const OL_UINT8 RO_EXCEEDED_LIMIT		= 'j';	//	The order has exceeded the firm trading limits.
const OL_UINT8 RO_UNKNOWN				= 'W';	//	Unknown. Contact support for further information.

//	--	Canceled Order Reason
const OL_UINT8 CO_USER_REQUESTED	= 'U';	//	User requested the order to be cancelled. Sent in response to a Cancel Order message or a Replace Order Message.
const OL_UINT8 CO_IMMEDIATE			= 'I';	//	Immediate (order was originally sent with Time In Force of 0).
const OL_UINT8 CO_EXPIRED			= 'T';	//	Timeout. Session/day order got expired.
const OL_UINT8 CO_SUPERVISORY		= 'S';	//	Supervisory. For example, emergency withdraw or the user got suspended.
const OL_UINT8 CO_USER_LOGOFF		= 'L';	//	User logged off.
const OL_UINT8 CO_INVALID_QTY		= 'Z';	//	Invalid quantity or quantity exceeds maximum limit.
const OL_UINT8 CO_ORDER_NOT_ALLOWED	= 'R';	//	The order is not allowed at this time.
const OL_UINT8 CO_INVALID_PRICE		= 'X';	//	Invalid Price
const OL_UINT8 CO_INVALID_ORDER_TYPE= 'Y';	//	Invalid Order Type. For example, the Time In Force value is not supported in current trading session.
const OL_UINT8 CO_UNKNOWN			= 'W';	//	Unknown. Contact support for further information.

typedef struct __OUCH_ENTER_ORDER_MSG__
{
	__OUCH_ENTER_ORDER_MSG__() : type(OUCH_TYPE_ENTER_ORDER_MSG) {};
	OL_UINT8 type;		//	OUCH_TYPE_ENTER_ORDER_MSG = 'O';
	OL_UINT8 orderVerb;	//	'B'uy; 'S'ell; Shor'T' Sell; 'P'rice Stabilization; 'M'argin Buy;
	OL_UINT8 domicile;
	OL_UINT32 orderToken;
	OL_UINT32 orderbookId;
	OL_UINT32 timeInForce;
	OL_UINT32 clientId;
	OL_UINT32 price;
	OL_UINT64 quantity;
	char szBrokerRef[24];
	char szInvestorId[8];
	char szOrderSource[8];
} OUCH_ENTER_ORDER_MSG;

typedef struct __OUCH_REPLACE_ORDER_MSG__
{
	__OUCH_REPLACE_ORDER_MSG__() : type(OUCH_TYPE_REPLACED_ORDER_MSG) {};
	OL_UINT8 type;	//	OUCH_TYPE_REPLACED_ORDER_MSG = 'U';
	OL_UINT32 curOrderToken;
	OL_UINT32 newOrderToken;
	OL_UINT32 price;
	OL_UINT64 quantity;
} OUCH_REPLACE_ORDER_MSG;

typedef struct __OUCH_CANCEL_ORDER_MSG__
{
	__OUCH_CANCEL_ORDER_MSG__() : type(OUCH_TYPE_CANCEL_ORDER_MSG) {};
	OL_UINT8 type;	//	OUCH_TYPE_CANCEL_ORDER_MSG	= 'X';
	OL_UINT32 orderToken;
} OUCH_CANCEL_ORDER_MSG;

typedef struct __OUCH_TRADING_LIMIT_REQ__
{
	__OUCH_TRADING_LIMIT_REQ__() : type(OUCH_TYPE_TRADING_LIMIT_MSG) {};
	OL_UINT8 type;	//	OUCH_TYPE_TRADING_LIMIT_MSG = 'T';
	OL_UINT8 tradingLimitType;
} OUCH_TRADING_LIMIT_REQ;

//------------------------------------------------------------
//	Outbound Sequenced Message Type
//------------------------------------------------------------
/*
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
*/
typedef struct __OUCH_SYSTEM_EVENT_MSG__
{
	OL_UINT8 type;		//	OUCH_TYPE_SYSTEM_EVENT_MSG	= 'S';
	OL_UINT8 eventCode;
	OL_UINT64 timeStamp;	//	Nanoseconds past midnight
} OUCH_SYSTEM_EVENT_MSG;

typedef struct __OUCH_ACCEPTED_ORDER_MSG__
{
	OL_UINT8 type;		//	OUCH_TYPE_ORDER_ACCEPTED_MSG = 'A';
	OL_UINT8 orderVerb;
	OL_UINT8 domicile;
	OL_UINT8 orderState;
	OL_UINT32 orderToken;
	OL_UINT32 orderbookId;
	OL_UINT32 timeInForce;
	OL_UINT32 clientId;
	OL_UINT32 price;
	OL_UINT64 quantity;
	OL_INT64 minQuantity;
	OL_UINT64 timeStamp;
	OL_UINT64 orderNumber;
	OL_UINT64 extOrderNumber;
	char szUserId[24];
	char szBrokerRef[32];
	char szInvestorId[8];
	char szOrderSource[8];
} OUCH_ACCEPTED_ORDER_MSG;

typedef struct __OUCH_REPLACED_ORDER_MSG__
{
	OL_UINT8 type;		//	OUCH_TYPE_REPLACED_ORDER_MSG = 'U';
	OL_UINT8 orderVerb;
	OL_UINT8 orderState;
	OL_UINT64 timeStamp;
	OL_UINT64 quantity;
	OL_UINT32 orderbookId;
	OL_UINT32 price;
	OL_UINT32 timeInForce;
	OL_UINT64 orderNumber;
	OL_UINT64 extOrderNumber;
	OL_UINT32 prevOrderToken;
	OL_UINT32 replacementOrderToken;
	char szUserId[24];
} OUCH_REPLACED_ORDER_MSG;

typedef struct __OUCH_CANCELED_ORDER_MSG__
{
	OL_UINT8 type;		//	OUCH_TYPE_CANCELED_ORDER_MSG = 'C';
	OL_UINT64 timeStamp;
	OL_UINT32 orderToken;
	OL_UINT64 quantity;
	OL_UINT8 reason;
	char szUserId[24];
} OUCH_CANCELED_ORDER_MSG;

typedef struct __OUCH_EXECUTED_ORDER_MSG__
{
	OL_UINT8 type;			//	OUCH_TYPE_EXECUTED_ORDER_MSG = 'E';
	OL_UINT64 timeStamp;
	OL_UINT32 orderToken;
	OL_UINT64 executedQty;
	OL_UINT32 executedPrice;
	OL_UINT8 liquidityFlag;
	OL_UINT64 matchNumber;
	OL_UINT32 counterPartyId;
	char szUserId[24];
	char szOrderId[22];
} OUCH_EXECUTED_ORDER_MSG;

typedef struct __OUCH_BROKEN_TRADE_MSG__
{
	OL_UINT8 type;			//	OUCH_TYPE_BROKEN_TRADE_MSG		= 'B';
	OL_UINT8 reason;
	OL_UINT64 timeStamp;
	OL_UINT32 orderToken;
	OL_UINT64 matchNumber;
	char szUserId[24];
	char szOrderId[22];
} OUCH_BROKEN_TRADE_MSG;

typedef struct __OUCH_REJECTED_ORDER_MSG__
{
	OL_UINT8 type;			//	OUCH_TYPE_REJECTED_ORDER_MSG = 'J';
	OL_UINT64 timeStamp;
	OL_UINT32 orderToken;
	OL_UINT8 reason;
	char szUserId[24];
	char szOrderId[22];
} OUCH_REJECTED_ORDER_MSG;

typedef struct __OUCH_TRADING_LIMIT_MSG__
{
	OL_UINT8 type;
	OL_UINT64 timeStamp;
	OL_UINT8 tradingLimitType;
	OL_UINT64 used;
	OL_UINT64 traded;
	OL_UINT64 limit;
} OUCH_TRADING_LIMIT_MSG;

typedef struct __OUCH_RESTATEMENT_MSG__
{
	OL_UINT8 type;
	OL_UINT64 timeStamp;
	OL_UINT32 orderToken;
	OL_UINT32 price;
	OL_UINT8 reason;
	char szUserId[24];
	char szOrderId[22];
} OUCH_RESTATEMENT_MSG;

#endif
