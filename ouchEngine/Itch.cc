#include "header/Itch.h"
#include <stdio.h>
#include "header/RawPackage.h"

CItch::CItch() : CSoupBin()
{
	//	printf("CItch::CItch()\n");
}

CItch::CItch(CRawPackage *praw) : CSoupBin(praw)
{
	//	printf("CItch::CItch(CRawPackage *praw)\n");
}

CItch::CItch(OL_UINT16 pkgSize, OL_UINT8 pkgId) : CSoupBin(pkgSize, pkgId)
{
	//	printf("CItch::CItch(OL_UINT16 pkgSize, OL_UINT8 pkgId)\n");
}

CItch::CItch(char *pdat, OL_UINT16 pkgSize, bool copyData) : CSoupBin(pdat, pkgSize, NULL, copyData)
{
	//	printf("CItch::CItch(char *pdat, OL_UINT16 pkgSize, bool copyData)\n");
}

uint8_t CItch::GetItchMessageType() const
{
	return pszData[3];
}

void CItch::GetTimeStamp(ITCH_TIME_STAMP &o)
{
	o.type = GetInt8();
	o.seconds = GetInt32();
}

void CItch::GetSystemEventMsg(ITCH_SYSTEM_EVENT_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	GetString(8, o.szGroup, SB_TRIM_RIGHT);
	GetString(12, o.szInstrument, SB_TRIM_RIGHT);
	o.eventCode = GetInt8();
	o.orderbookId = GetInt32();
}

void CItch::GetPriceTickSizeTable(ITCH_PRICE_TICK_SIZE_TABLE &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.tickSizeTableId = GetInt32();
	o.tickSize = GetInt32();
	o.priceStart = GetInt32();
}

void CItch::GetQtyTickSizeTable(ITCH_QTY_TICK_SIZE_TABLE &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.tickSizeTableId = GetInt32();
	o.tickSize = GetInt64();
	o.qtyStart = GetInt64();
}

void CItch::GetIssuerDirectory(ITCH_ISSUER_DIRECTORY &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.issuerId = GetInt32();
	GetString(60, o.szIssuerName, SB_TRIM_RIGHT);
	GetString(21, o.szIssuerCode, SB_TRIM_RIGHT);
}

void CItch::GetSectorDirectory(ITCH_SECTOR_DIRECTORY &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.sectorId = GetInt32();
	GetString(30, o.szSectorName, SB_TRIM_RIGHT);
	GetString(30, o.szIndustryName, SB_TRIM_RIGHT);
}

void CItch::GetOrderBookDirectory(ITCH_ORDERBOOK_DIRECTORY &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderbookId = GetInt32();
	GetString(12, o.szISIN, SB_TRIM_RIGHT);
	GetString(21, o.szSecurityCode, SB_TRIM_RIGHT);
	if (lWritePos > 184)
		GetString(100, o.szSecurityName, SB_TRIM_RIGHT);
	else
		o.szSecurityName[0] = 0x00;
	GetString(3, o.szCurrency, SB_TRIM_RIGHT);
	GetString(8, o.szGroup, SB_TRIM_RIGHT);
	o.issuerId = GetInt32();
	o.ipoPrice = GetInt32();
	o.tradeableShares = GetInt64();
	o.listedShares = GetInt64();
	o.sharesPerLot = GetInt32();
	o.minQty = GetInt64();
	o.qtyTickSizeTableId = GetInt32();
	o.qtyDecimals = GetInt32();
	o.priceTickSizeTableId = GetInt32();
	o.priceDecimals = GetInt32();
	o.delistingMatureDate = GetInt32();
	o.delistingTime = GetInt32();
	GetString(12, o.szInstrument, SB_TRIM_RIGHT);
	GetString(8, o.szRemark0, SB_TRIM_RIGHT);
	GetString(40, o.szRemark1, SB_TRIM_RIGHT);
	o.foreignLimit = GetInt32();
	o.sectorId = GetInt32();
}

void CItch::GetFuturesDirectory(ITCH_FUTURES_DIRECTORY &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderbookId = GetInt32();
	GetString(12, o.szISIN, SB_TRIM_RIGHT);
	GetString(21, o.szSecurityCode, SB_TRIM_RIGHT);
	GetString(3, o.szCurrency, SB_TRIM_RIGHT);
	GetString(8, o.szGroup, SB_TRIM_RIGHT);
	o.qtyTickSizeTableId = GetInt32();
	o.priceTickSizeTableId = GetInt32();
	o.priceDecimals = GetInt32();
	o.expiryDate = GetInt64();
	o.underlyingId = GetInt32();
	GetString(8, o.szRemark0, SB_TRIM_RIGHT);
	GetString(40, o.szRemark1, SB_TRIM_RIGHT);
	o.contractSize = GetInt64();
}

void CItch::GetOptionsDirectory(ITCH_OPTIONS_DIRECTORY &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderbookId = GetInt32();
	GetString(12, o.szISIN, SB_TRIM_RIGHT);
	GetString(21, o.szSecurityCode, SB_TRIM_RIGHT);
	GetString(3, o.szCurrency, SB_TRIM_RIGHT);
	GetString(8, o.szGroup, SB_TRIM_RIGHT);
	o.qtyTickSizeTableId = GetInt32();
	o.priceTickSizeTableId = GetInt32();
	o.priceDecimals = GetInt32();
	o.verb = GetInt8();
	o.strikePrice = GetInt32();
	o.expiryDate = GetInt64();
	o.underlyingId = GetInt32();
	GetString(8, o.szRemark0, SB_TRIM_RIGHT);
	GetString(40, o.szRemark1, SB_TRIM_RIGHT);
	o.contractSize = GetInt64();
}

void CItch::GetIndexDirectory(ITCH_INDEX_DIRECTORY &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderbookId = GetInt32();
	GetString(21, o.szSecurityCode, SB_TRIM_RIGHT);
	o.priceDecimals = GetInt32();
}

void CItch::GetIndexMemberDirectory(ITCH_INDEX_MEMBER_DIRECTORY &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderbookId = GetInt32();
	o.memberOrderbookId = GetInt32();
	o.weight = GetInt64();
}

void CItch::GetParticipantDirectory(ITCH_PARTICIPANT_DIRECTORY &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.participantId = GetInt32();
	GetString(60, o.szParticipantName, SB_TRIM_RIGHT);
	GetString(2, o.szParticipantCode, SB_TRIM_RIGHT);
}

void CItch::GetOrderbookTradingActionMsg(ITCH_OB_TRADING_ACTION_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderbookId = GetInt32();
	o.tradingState = GetInt8();
	o.reason = GetInt8();
}

void CItch::GetAddOrderMsg(ITCH_ADD_ORDER_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderNumber = GetInt64();
	o.orderVerb = GetInt8();
	o.quantity = GetInt64();
	o.orderbookId = GetInt32();
	o.price = GetInt32();
	o.domicile = GetInt8();
}

void CItch::GetAddOrderWithParticipantMsg(ITCH_ADD_ORDER_WITH_PARTICIPANT_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderNumber = GetInt64();
	o.orderVerb = GetInt8();
	o.quantity = GetInt64();
	o.orderbookId = GetInt32();
	o.price = GetInt32();
	o.domicile = GetInt8();
	o.participantId = GetInt32();
}

void CItch::GetOrderExecutedMsg(ITCH_ORDER_EXECUTED_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderNumber = GetInt64();
	o.executedQty = GetInt64();
	o.matchNumber = GetInt64();
	o.tradeIndicator = GetInt8();
	o.buyParticipantId = GetInt32();
	o.sellParticipantId = GetInt32();
	o.buyDomicile = GetInt8();
	o.sellDomicile = GetInt8();
}

void CItch::GetOrderExecutedWithPriceMsg(ITCH_ORDER_EXECUTED_WITH_PRICE_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderNumber = GetInt64();
	o.executedQty = GetInt64();
	o.matchNumber = GetInt64();
	o.tradeIndicator = GetInt8();
	o.printable = GetInt8();
	o.executionPrice = GetInt32();
	o.buyParticipantId = GetInt32();
	o.sellParticipantId = GetInt32();
	o.buyDomicile = GetInt8();
	o.sellDomicile = GetInt8();
}

void CItch::GetTradeMsg(ITCH_TRADE_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.executedQty = GetInt64();
	o.orderbookId = GetInt32();
	o.printable = GetInt8();
	o.executionPrice = GetInt32();
	o.matchNumber = GetInt64();
	o.tradeIndicator = GetInt8();
	o.buyParticipantId = GetInt32();
	o.sellParticipantId = GetInt32();
	o.buyDomicile = GetInt8();
	o.sellDomicile = GetInt8();
}

void CItch::GetBrokenTradeMsg(ITCH_BROKEN_TRADE_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.matchNumber = GetInt64();
	o.reason = GetInt8();
}

void CItch::GetOrderDeleteMsg(ITCH_ORDER_DELETE_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderNumber = GetInt64();
}

void CItch::GetOrderClearMsg(ITCH_ORDER_CLEAR_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderbookId = GetInt32();
}

void CItch::GetOrderReplaceMsg(ITCH_ORDER_REPLACE_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.oriOrderNumber = GetInt64();
	o.newOrderNumber = GetInt64();
	o.quantity = GetInt64();
	o.price = GetInt32();
}

void CItch::GetOrderRelinkMsg(ITCH_ORDER_RELINK_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderNumber = GetInt64();
	o.exchgOrderNumber = GetInt64();
}

void CItch::GetIndicativePriceQtyMsg(ITCH_INDICATIVE_PRICE_QTY_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.theoreticalOpeningQty = GetInt64();
	o.orderbookId = GetInt32();
	o.bestBidPrice = GetInt32();
	o.bestOfferPrice = GetInt32();
	o.theoreticalOpeningPrice = GetInt32();
	o.crossType = GetInt8();
	o.bestBidQtySize = GetInt64();
	o.bestOfferQtySize = GetInt64();
}

void CItch::GetGlimpseSnapshotMsg(ITCH_GLIMPSE_SNAPSHOT_MSG &o)
{
	o.type = GetInt8();
	o.seqNumber = GetInt64();
}

void CItch::GetBestBidOfferMsg(ITCH_BEST_BID_OFFER_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderbookId = GetInt32();
	o.bestBidPrice = GetInt32();
	o.bestBidSize = GetInt64();
	o.bestOfferPrice = GetInt32();
	o.bestOfferSize = GetInt64();
}

void CItch::GetNewsMsg(ITCH_NEWS_MSG &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.orderbookId = GetInt32();
	o.newsId = GetInt32();
	o.participantId = GetInt32();
	GetString(o.szTitle);
	GetString(o.szReference);
	GetString(o.szNews);
}

void CItch::GetParticipantStatus(ITCH_PARTICIPANT_STATUS &o)
{
	o.type = GetInt8();
	o.timeStamp = GetInt32();
	o.participantId = GetInt32();
	o.participantStatus = GetInt8();
}
