#ifndef __ITCH_H__
#define __ITCH_H__

#include <cstdint>
#include <cstddef>
#include "constants.h"
#include "SoupBin.h"
#include "ItchDefine.h"

class CRawPackage;

class CItch : public CSoupBin
{
public:
	CItch();
	CItch(CRawPackage *praw);
	CItch(OL_UINT16 pkgSize, OL_UINT8 pkgId);
	CItch(char *pdat, OL_UINT16 pkgSize, bool copyData = false);
	virtual ~CItch() {};

	uint8_t GetItchMessageType() const;
	
	void GetTimeStamp(ITCH_TIME_STAMP &o);
	void GetSystemEventMsg(ITCH_SYSTEM_EVENT_MSG &o);
	void GetPriceTickSizeTable(ITCH_PRICE_TICK_SIZE_TABLE &o);
	void GetQtyTickSizeTable(ITCH_QTY_TICK_SIZE_TABLE &o);
	void GetIssuerDirectory(ITCH_ISSUER_DIRECTORY &o);
	void GetSectorDirectory(ITCH_SECTOR_DIRECTORY &o);
	void GetOrderBookDirectory(ITCH_ORDERBOOK_DIRECTORY &o);
	void GetFuturesDirectory(ITCH_FUTURES_DIRECTORY &o);
	void GetOptionsDirectory(ITCH_OPTIONS_DIRECTORY &o);
	void GetIndexDirectory(ITCH_INDEX_DIRECTORY &o);
	void GetIndexMemberDirectory(ITCH_INDEX_MEMBER_DIRECTORY &o);
	void GetParticipantDirectory(ITCH_PARTICIPANT_DIRECTORY &o);
	void GetOrderbookTradingActionMsg(ITCH_OB_TRADING_ACTION_MSG &o);
	void GetAddOrderMsg(ITCH_ADD_ORDER_MSG &o);
	void GetAddOrderWithParticipantMsg(ITCH_ADD_ORDER_WITH_PARTICIPANT_MSG &o);
	void GetOrderExecutedMsg(ITCH_ORDER_EXECUTED_MSG &o);
	void GetOrderExecutedWithPriceMsg(ITCH_ORDER_EXECUTED_WITH_PRICE_MSG &o);
	void GetTradeMsg(ITCH_TRADE_MSG &o);
	void GetBrokenTradeMsg(ITCH_BROKEN_TRADE_MSG &o);
	void GetOrderDeleteMsg(ITCH_ORDER_DELETE_MSG &o);
	void GetOrderClearMsg(ITCH_ORDER_CLEAR_MSG &o);
	void GetOrderReplaceMsg(ITCH_ORDER_REPLACE_MSG &o);
	void GetOrderRelinkMsg(ITCH_ORDER_RELINK_MSG &o);
	void GetIndicativePriceQtyMsg(ITCH_INDICATIVE_PRICE_QTY_MSG &o);
	void GetGlimpseSnapshotMsg(ITCH_GLIMPSE_SNAPSHOT_MSG &o);
	void GetBestBidOfferMsg(ITCH_BEST_BID_OFFER_MSG &o);
	void GetNewsMsg(ITCH_NEWS_MSG &o);
	void GetParticipantStatus(ITCH_PARTICIPANT_STATUS &o);
};

#endif
