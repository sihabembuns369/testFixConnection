#include "header/Ouch.h"
#include <stdio.h>
#include "header/RawPackage.h"

COuch::COuch() : CSoupBin()
{
}

COuch::COuch(OL_UINT16 pkgSize, OL_UINT8 pkgId):CSoupBin(pkgSize, pkgId)
{
}

COuch::COuch(char *pdat, OL_UINT16 pkgSize, const char *uid, bool copyData):CSoupBin(pdat, pkgSize, uid, copyData)
{
}

// COuch::COuch(const OUCH_ENTER_ORDER_MSG &o) : CSoupBin(61, SB_PKG_UNSEQUENCED)
// {
// 	AddInt8(o.type);
// 	AddInt32(o.orderToken);
// 	AddString(o.szBrokerRef, 20);
// 	AddString(o.szInvestorId, 6);
// 	AddInt8(o.orderVerb);
// 	AddString(o.szOrderSource, 4);
// 	AddInt8(o.domicile);
// 	AddInt64(o.quantity);
// 	AddInt32(o.orderbookId);
// 	AddInt32(o.price);
// 	AddInt32(o.timeInForce);
// 	AddInt32(o.clientId);
// }

// COuch::COuch(const OUCH_REPLACE_ORDER_MSG &o) : CSoupBin(21, SB_PKG_UNSEQUENCED)
// {
// 	AddInt8(o.type);
// 	AddInt32(o.curOrderToken);
// 	AddInt32(o.newOrderToken);
// 	AddInt64(o.quantity);
// 	AddInt32(o.price);
// }

// COuch::COuch(const OUCH_CANCEL_ORDER_MSG &o) : CSoupBin(5, SB_PKG_UNSEQUENCED)
// {
// 	AddInt8(o.type);
// 	AddInt32(o.orderToken);
// }

//OL_UINT8 COuch::GetOUCHMessageType() const
//{
//	return pszData[3];
//}

// void COuch::ComposeOuchEnterOrderMsg(const OUCH_ENTER_ORDER_MSG &o)
// {
// 	AllocateBuffer(61);
// 	SetPackageId(SB_PKG_UNSEQUENCED);
// 	AddInt8(o.type);
// 	AddInt32(o.orderToken);
// 	AddString(o.szBrokerRef, 20);
// 	AddString(o.szInvestorId, 6);
// 	AddInt8(o.orderVerb);
// 	AddString(o.szOrderSource, 4);
// 	AddInt8(o.domicile);
// 	AddInt64(o.quantity);
// 	AddInt32(o.orderbookId);
// 	AddInt32(o.price);
// 	AddInt32(o.timeInForce);
// 	AddInt32(o.clientId);
// }

// void COuch::ComposeOuchReplaceOrderMsg(const OUCH_REPLACE_ORDER_MSG &o)
// {
// 	AllocateBuffer(21);
// 	SetPackageId(SB_PKG_UNSEQUENCED);
// 	AddInt8(o.type);
// 	AddInt32(o.curOrderToken);
// 	AddInt32(o.newOrderToken);
// 	AddInt64(o.quantity);
// 	AddInt32(o.price);
// }

// void COuch::ComposeOuchCancelOrderMsg(const OUCH_CANCEL_ORDER_MSG &o)
// {
// 	AllocateBuffer(5);
// 	SetPackageId(SB_PKG_UNSEQUENCED);
// 	AddInt8(o.type);
// 	AddInt32(o.orderToken);
// }

// void COuch::ComposeOuchTradingLimitReq(const OUCH_TRADING_LIMIT_REQ &o)
// {
// 	AllocateBuffer(2);
// 	SetPackageId(SB_PKG_UNSEQUENCED);
// 	AddInt8(o.type);
// 	AddInt8(o.tradingLimitType);
// }

void COuch::GetOuchSystemEventMsg(OUCH_SYSTEM_EVENT_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt64();
	o.eventCode = GetInt8();
}

void COuch::GetOuchAcceptedOrderMsg(OUCH_ACCEPTED_ORDER_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt64();
	o.orderToken = GetInt32();
	GetString(20, o.szBrokerRef);
	GetString(6, o.szInvestorId);
	o.orderVerb = GetInt8();
	GetString(4, o.szOrderSource);
	o.domicile = GetInt8();
	o.quantity = GetInt64();
	o.orderbookId = GetInt32();
	o.price = GetInt32();
	o.timeInForce = GetInt32();
	o.clientId = GetInt32();
	o.orderNumber = GetInt64();
	o.extOrderNumber = GetInt64();
	o.orderState = GetInt8();
	o.minQuantity = GetInt64();
	strcpy(o.szUserId, szIdentity);
}

void COuch::GetOuchReplacedOrderMsg(OUCH_REPLACED_ORDER_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt64();
	o.replacementOrderToken = GetInt32();
	o.orderVerb = GetInt8();
	o.quantity = GetInt64();
	o.orderbookId = GetInt32();
	o.price = GetInt32();
	o.timeInForce = GetInt32();
	o.orderNumber = GetInt64();
	o.extOrderNumber = GetInt64();
	o.orderState = GetInt8();
	o.prevOrderToken = GetInt32();
	strcpy(o.szUserId, szIdentity);
}

void COuch::GetOuchCanceledOrderMsg(OUCH_CANCELED_ORDER_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt64();
	o.orderToken = GetInt32();
	o.quantity = GetInt64();
	o.reason = GetInt8();
	strcpy(o.szUserId, szIdentity);
}

void COuch::GetOuchExecutedOrderMsg(OUCH_EXECUTED_ORDER_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt64();
	o.orderToken = GetInt32();
	o.executedQty = GetInt64();
	o.executedPrice = GetInt32();
	o.liquidityFlag = GetInt8();
	o.matchNumber = GetInt64();
	o.counterPartyId = GetInt32();
	strcpy(o.szUserId, szIdentity);
}

void COuch::GetOuchBrokenTradeMsg(OUCH_BROKEN_TRADE_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt64();
	o.orderToken = GetInt32();
	o.matchNumber = GetInt64();
	o.reason = GetInt8();
	strcpy(o.szUserId, szIdentity);
}

void COuch::GetOuchRejectedOrderMsg(OUCH_REJECTED_ORDER_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt64();
	o.orderToken = GetInt32();
	o.reason = GetInt8();
	strcpy(o.szUserId, szIdentity);
}

void COuch::GetOuchTradingLimitMsg(OUCH_TRADING_LIMIT_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt64();
	o.tradingLimitType = GetInt8();
	o.used = GetInt64();
	o.traded = GetInt64();
	o.limit = GetInt64();
}

void COuch::GetOuchRestatementMsg(OUCH_RESTATEMENT_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt64();
	o.orderToken = GetInt32();
	o.price = GetInt32();
	o.reason = GetInt8();
	strcpy(o.szUserId, szIdentity);
}

COuchEnterOrder::COuchEnterOrder(OL_UINT32 oToken, const char *brokerRef, const char *investorId, OL_UINT8 oVerb, const char *orderSource, OL_UINT8 oDomicile, 
								 OL_INT64 qty, OL_UINT32 orderBookId, OL_UINT32 oPrice, OL_UINT32 oTimeInForce, OL_UINT32 oClientId, OL_INT64 minQty, const char *custId, 
								 const char *stockCode, const char *inputUser) : CSoupBin(72, SB_PKG_UNSEQUENCED)
{
	AddInt8(OUCH_TYPE_ENTER_ORDER_MSG);		//	3
	AddInt32(orderToken = oToken);			//	4
	AddString(brokerRef, 20);				//	8
	AddString(investorId, 6);				//	28
	AddInt8(orderVerb = oVerb);				//	34
	AddString(orderSource, 4);				//	35
	AddInt8(domicile = oDomicile);			//	39
	AddInt64(quantity = qty);				//	40
	AddInt32(orderbookId = orderBookId);	//	48
	AddInt32(price = oPrice);				//	52
	AddInt32(timeInForce = oTimeInForce);	//	56
	AddInt32(clientId = oClientId);			//	60
	AddInt64(minQuantity = minQty);			//	64	//	Hanya support di Replica, Penambahan baru untuk IOC
	strcpy(szCustId, custId);
	strcpy(szOrderId, brokerRef);
	strcpy(szBrokerRef, brokerRef);
	strcpy(szInvestorId, investorId);
	strcpy(szOrderSource, orderSource);
	strcpy(szStockCode, stockCode);
	strcpy(szInputUser, inputUser);
}

OL_UINT32 COuchEnterOrder::SetOrderToken(OL_UINT32 token)
{
	orderToken = token;
	memcpy(pszData + 4, &(token = htonl(token)), SZ_INT32);
	return orderToken;
}

void COuchEnterOrder::SetOrderbookId(OL_UINT32 id)
{
	memcpy(pszData + 48, &(id = htonl(orderbookId = id)), SZ_INT32);
}

void COuchEnterOrder::SetPrice(OL_UINT32 oPrice)
{
	memcpy(pszData + 52, &(oPrice = htonl(price = oPrice)), SZ_INT32);
}

void COuchEnterOrder::SetQuantity(OL_INT64 qty)
{
	memcpy(pszData + 40, &(qty = htonll(quantity = qty)), SZ_INT64);
}

OL_UINT32 COuchReplaceOrder::SetNewOrderToken(OL_UINT32 orderToken)
{
	memcpy(pszData + 8, &(orderToken = htonl(newOrderToken = orderToken)), SZ_INT32);
	return newOrderToken;
}
