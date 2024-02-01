#ifndef __OUTCH_H__
#define __OUTCH_H__

#include <cstddef>
#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "SoupBin.h"
#include "OuchDefine.h"

class COuch : public CSoupBin
{
public:
	COuch();
	COuch(OL_UINT16 pkgSize, OL_UINT8 pkgId);
	COuch(char *pdat, OL_UINT16 pkgSize, const char *uid = NULL, bool copyData = false);
	// COuch(const OUCH_ENTER_ORDER_MSG &o);
	// COuch(const OUCH_REPLACE_ORDER_MSG &o);
	// COuch(const OUCH_CANCEL_ORDER_MSG &o);

	//const char *GetUserId() const { return szUserId; };

	// void ComposeOuchEnterOrderMsg(const OUCH_ENTER_ORDER_MSG &o);
	// void ComposeOuchReplaceOrderMsg(const OUCH_REPLACE_ORDER_MSG &o);
	// void ComposeOuchCancelOrderMsg(const OUCH_CANCEL_ORDER_MSG &o);
	// void ComposeOuchTradingLimitReq(const OUCH_TRADING_LIMIT_REQ &o);
	
	void GetOuchSystemEventMsg(OUCH_SYSTEM_EVENT_MSG &o);
	void GetOuchAcceptedOrderMsg(OUCH_ACCEPTED_ORDER_MSG &o);
	void GetOuchReplacedOrderMsg(OUCH_REPLACED_ORDER_MSG &o);
	void GetOuchCanceledOrderMsg(OUCH_CANCELED_ORDER_MSG &o);
	void GetOuchExecutedOrderMsg(OUCH_EXECUTED_ORDER_MSG &o);
	void GetOuchRejectedOrderMsg(OUCH_REJECTED_ORDER_MSG &o);
	void GetOuchTradingLimitMsg(OUCH_TRADING_LIMIT_MSG &o);
	void GetOuchBrokenTradeMsg(OUCH_BROKEN_TRADE_MSG &o);
	void GetOuchRestatementMsg(OUCH_RESTATEMENT_MSG &o);
};

class COuchTradingLimitRequest : public CSoupBin
{
public:
	COuchTradingLimitRequest(OL_UINT8 TLType) : CSoupBin(2, SB_PKG_UNSEQUENCED) {
		AddInt8(OUCH_TYPE_TRADING_LIMIT_MSG);
		AddInt8(tradingLimitType = TLType);
	};
	virtual ~COuchTradingLimitRequest() {};
	inline OL_UINT8 GetTradingLimitType() const { return tradingLimitType; };

private:
	OL_UINT8 tradingLimitType;
};

class COuchEnterOrder : public CSoupBin
{
public:
	COuchEnterOrder(OL_UINT32 oToken, const char *brokerRef, const char *investorId, OL_UINT8 oVerb, const char *orderSource, OL_UINT8 oDomicile, OL_INT64 qty, 
		OL_UINT32 orderBookId, OL_UINT32 oPrice, OL_UINT32 oTimeInForce, OL_UINT32 oClientId, OL_INT64 minQty, const char *custId, const char *stockCode, const char *inputUser);
	virtual ~COuchEnterOrder() {};

	inline OL_UINT32 GetOrderBookId() const { return orderbookId; };
	inline const char *GetOrderId() const { return szOrderId; };
	inline const char *GetStockCode() const { return szStockCode; };

	OL_UINT32 SetOrderToken(OL_UINT32 token);
	void SetOrderbookId(OL_UINT32 id);
	void SetPrice(OL_UINT32 oPrice);
	void SetQuantity(OL_INT64 qty);

	void GetDescriptions(char *s) {
		sprintf(s, "OrderId:%s; CustId:%s; Verb:%c; StockCode:%s; OrderBookId:%u; OrderToken:%u; Price:%d; Qty:%ld; MinQty:%ld; TIF:%u; Domicile:%c; ClientId:%u; InvestorId:%s; BrokerRef:%s; OrderSource:%s; InputUser:%s;",
			szOrderId, szCustId, orderVerb, szStockCode, orderbookId, orderToken, price, quantity, minQuantity, timeInForce, domicile, clientId, szInvestorId, szBrokerRef, szOrderSource, szInputUser);
	};

private:
	OL_UINT8	orderVerb,	//	'B'uy; 'S'ell; Shor'T' Sell; 'P'rice Stabilization; 'M'argin Buy;
				domicile;
	OL_UINT32	orderToken,
				orderbookId,
				timeInForce,
				clientId;
	OL_INT32 price;
	OL_INT64 quantity;
	/*
		If TIF = 0, specifies the minimum acceptable quantity to execute.
		For FOK order, set Minimum Quantity = Quantity
		For FAK order, set Minimum Quantity >= 0
	*/
	OL_INT64 minQuantity;		//	If TIF = 0, specifies the minimum acceptable quantity to execute. (Non-0 values are only supported for IOC.)
	char szCustId[16];
	char szOrderId[32];
	char szStockCode[64];	//	StockCode.Board
	char szBrokerRef[32];
	char szInvestorId[16];
	char szOrderSource[16];
	char szInputUser[64];
};


class COuchReplaceOrder : public CSoupBin
{
public:
	COuchReplaceOrder(OL_UINT32 currentToken, OL_UINT32 newToken, OL_UINT32 oPrice, OL_INT64 qty, const char *amendId, const char *orderId, const char *clientId) : 
		CSoupBin(21, SB_PKG_UNSEQUENCED)
	{
		AddInt8(OUCH_TYPE_REPLACED_ORDER_MSG);		//	3
		AddInt32(curOrderToken = currentToken);		//	4
		AddInt32(newOrderToken = newToken);			//	8
		AddInt64(quantity = qty);					//	12
		AddInt32(price = oPrice);					//	20
		strcpy(szAmendId, amendId);
		strcpy(szOrderId, orderId);
		strcpy(szClientId, clientId);
	};
	virtual ~COuchReplaceOrder() {};

	OL_UINT32 SetNewOrderToken(OL_UINT32 orderToken);
	const char *GetAmendId() const { return szAmendId; };
	const char *GetOrderId() const { return szOrderId; };
	const char *GetClientId() const { return szClientId; };

	void GetDescriptions(char *s) {
		sprintf(s, "AmendId:%s; OrderId:%s; CurOrderToken:%u; NewOrderToken:%u; Price:%d; Qty:%ld; ClientId:%s;",
			szAmendId, szOrderId, curOrderToken, newOrderToken, price, quantity, szClientId);
	};

private:
	OL_UINT32 curOrderToken;
	OL_UINT32 newOrderToken;
	OL_INT32 price;
	OL_INT64 quantity;
	char szAmendId[32];
	char szOrderId[32];
	char szClientId[32];
};

class COuchCancelOrder : public CSoupBin
{
public:
	COuchCancelOrder(OL_UINT32 oToken, const char *withdrawId, const char *orderId, const char *clientId) : 
		CSoupBin(5, SB_PKG_UNSEQUENCED)
	{
		AddInt8(OUCH_TYPE_CANCEL_ORDER_MSG);
		AddInt32(orderToken = oToken);
		strcpy(szOrderId, orderId);
		strcpy(szClientId, clientId);
		strcpy(szWithdrawId, withdrawId);
	};
	virtual ~COuchCancelOrder() {};

	OL_UINT32 GetOrderToken() const { return orderToken; };
	const char *GetOrderId() const { return szOrderId; };
	const char *GetClientId() const { return szClientId; };
	const char *GetWithdrawId() const { return szWithdrawId; };

	void GetDescriptions(char *s) {
		sprintf(s, "WithdrawId:%s; OrderId:%s; OrderToken:%u; ClientId:%s;", szWithdrawId, szOrderId, orderToken, szClientId);
	};

private:
	OL_UINT32 orderToken;
	char szOrderId[32];
	char szWithdrawId[32];
	char szClientId[32];
};

#endif
