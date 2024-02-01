#pragma once
#include <map>
#include <string>
#include <pthread.h>
#include "OuchDefine.h"

class CTokenMap
{
public:
	CTokenMap() {
        pthread_rwlock_init(&rwlock, NULL);
    };
	~CTokenMap() {
        Release();
        pthread_rwlock_destroy(&rwlock);
    };

	void Release();
	bool isRegisterTokenId(uint32_t tokenId);
	bool RegisterToken(uint32_t tokenId, const char *szOrderId);
	bool GetOrderIdByToken(uint32_t tokenId, char *szOrderId, size_t slen);

private:
    pthread_rwlock_t rwlock;
	typedef std::map<uint32_t, std::string> _TOKENMAP_;
	_TOKENMAP_ tokenMap;
};

class CGPackage;
class CIPackage;
class CTEOrderPackage;

class CClientConverter 
{
public:
    CClientConverter() {};
    ~CClientConverter() {};

    //  Request from Client
    static CIPackage* ILogoutRequest(CGPackage *pkg);
    static CIPackage* IComposeLogoutRequest(const char *clientIP, const char *loginId, const char *ref, uint8_t srcId);
    static CIPackage* ILoginRequest(CGPackage *pkg);
    static CIPackage* IValidatePIN(CGPackage *pkg);
    static CIPackage* IRequestChangePwdPin(CGPackage *pkg);
    static CIPackage* IRequestTradingLimit(CGPackage *pkg);
    static CIPackage* IRequestStockBalance(CGPackage *pkg);
    static CIPackage* IRequestAccountInfo(CGPackage *pkg);
    static CIPackage* IRequestRealizedGainLoss(CGPackage *pkg);
    static CIPackage* IRequestTradeConfirmation(CGPackage *pkg);
    static CIPackage* IRequestTaxReport(CGPackage *pkg);
    static CIPackage* IRequestPortfolio(CGPackage *pkg);
    static CIPackage* IRequestPortfolioReturn(CGPackage *pkg);
    static CIPackage* IRequestFundWithdrawInfo(CGPackage *pkg);
    static CIPackage* IRequestFundWithdrawList(CGPackage *pkg);
    static CIPackage* IRequestFundWithdraw(CGPackage *pkg);
    static CIPackage* IRequestCancelFundWithdraw(CGPackage *pkg);
    static CIPackage* IRequestFinancialHistory(CGPackage *pkg);
    static CIPackage* IRequestCashLedger(CGPackage *pkg);
    static CIPackage* IRequestExerciseRightWarrantInfo(CGPackage *pkg);
    static CIPackage* IRequestExerciseRightWarrantList(CGPackage *pkg);
    static CIPackage* IRequestExerciseRightWarrant(CGPackage *pkg);
    static CIPackage* IRequestCancelExerciseRightWarrant(CGPackage *pkg);
    static CIPackage* IRequestStockCollateralInfo(CGPackage *pkg);
    static CIPackage* IRequestTransactionAndHoliday(CGPackage *pkg);
    static CIPackage* IRequestCorporateActionInformation(CGPackage *pkg);
    static CIPackage* IRequestTransactionsReport(CGPackage *pkg);
    static CIPackage* IRequestMonthlyBalance(CGPackage *pkg);
    static CIPackage* IRequestSingleStockBalance(CGPackage *pkg);
    static CIPackage* IRequestValidateLoginPassword(CGPackage *pkg);
    static CIPackage* IRequestSpeedOrderBook(CGPackage *pkg);

    //  Request Order from Client
    static CIPackage* IRequestOrder(CGPackage *pkg);
    static CIPackage* IRequestAmend(CGPackage *pkg);
    static CIPackage* IRequestWithdraw(CGPackage *pkg);
    static CIPackage* IRequestOrderList(CGPackage *pkg);
    static CIPackage* IRequestTradeList(CGPackage *pkg);
    static CIPackage* IRequestRejectedOrderMessage(CGPackage *pkg);
    static CIPackage* IRequestHistoricalOrderList(CGPackage *pkg);
    static CIPackage* IRequestHistoricalTradeList(CGPackage *pkg);
    static CIPackage* IRequestGTCOrder(CGPackage *pkg);
    static CIPackage* IRequestCancelGTCOrder(CGPackage *pkg);
    static CIPackage* IRequestGTCOrderList(CGPackage *pkg);
    static CIPackage* IRequestBreakOrderList(CGPackage *pkg);
    static CIPackage* IRequestTrailingOrderList(CGPackage *pkg);
    static CIPackage* IRequestBreakOrder(CGPackage *pkg);
    static CIPackage* IRequestTrailingOrder(CGPackage *pkg);
    static CIPackage* IRequestCancelBreakOrder(CGPackage *pkg);
    static CIPackage* IRequestCancelTrailingOrder(CGPackage *pkg);
    static CIPackage* IRequestSpeedOrderAmend(CGPackage *pkg);
    static CIPackage* IRequestSpeedOrderWithdraw(CGPackage *pkg);

    //  Reply from IServer
    static CGPackage* ILoginReply(CIPackage *p, uint32_t LOT_SIZE);
    static CGPackage* IValidatePINReply(CIPackage *p);
    static CGPackage* IReplyChangePwdPin(CIPackage *p);
    static CGPackage* IReplyTradingLimit(CIPackage *p);
    static CGPackage* IReplyStockBalance(CIPackage *p);
    static CGPackage* IReplyAccountInfo(CIPackage *p);
    static CGPackage* IReplyRealizedGainLoss(CIPackage *p);
    static CGPackage* IReplyTradeConfirmation(CIPackage *p);
    static CGPackage* IReplyTaxReport(CIPackage *p);
    static CGPackage* IReplyPortfolio(CIPackage *p);
    static CGPackage* IReplyPortfolioReturn(CIPackage *p);

    static CGPackage* IReplyFundWithdrawInfo(CIPackage *p);
    static CGPackage* IReplyFundWithdrawList(CIPackage *p);
    static CGPackage* IReplyFundWithdraw(CIPackage *p);
    static CGPackage* IReplyCancelFundWithdraw(CIPackage *p);
    static CGPackage* IReplyFinancialHistory(CIPackage *p);
    static CGPackage* IReplyCashLedger(CIPackage *p);
    static CGPackage* IReplyExerciseRightWarrantInfo(CIPackage *p);
    static CGPackage* IReplyExerciseRightWarrantList(CIPackage *p);
    static CGPackage* IReplyExerciseRightWarrant(CIPackage *p);
    static CGPackage* IReplyCancelExerciseRightWarrant(CIPackage *p);

    static CGPackage* IReplyStockCollateralInfo(CIPackage *p);
    static CGPackage* IReplyTransactionAndHoliday(CIPackage *p);
    static CGPackage* IReplyCorporateActionInformation(CIPackage *p);
    static CGPackage* IReplyTransactionsReport(CIPackage *p);
    static CGPackage* IReplyMonthlyBalance(CIPackage *p);
    static CGPackage* IReplySingleStockBalance(CIPackage *p);
    static CGPackage* IReplyValidateLoginPassword(CIPackage *p);
    static CGPackage* IReplySpeedOrderBook(CIPackage *p);

    // Order Reply from IServer
    static CGPackage* IReplyOrder(CIPackage *p);
    static CGPackage* IReplyOrderList(CIPackage *p);
    static CGPackage* IReplyTradeList(CIPackage *p);
    static CGPackage* IReplyRejectedOrderMessage(CIPackage *p);
    static CGPackage* IReplyGTCOrderList(CIPackage *p);
    static CGPackage* IReplyBreakOrderList(CIPackage *p);
    static CGPackage* IReplyTrailingOrderList(CIPackage *p);

    //  Ouch Package Composer
    static CTEOrderPackage* ComposeOuchSystemEventMsg(OUCH_SYSTEM_EVENT_MSG &o);
    static CTEOrderPackage* ComposeOuchTradingLimitMsg(OUCH_TRADING_LIMIT_MSG &o);
    static CTEOrderPackage* ComposeOuchAcceptedOrderMsg(OUCH_ACCEPTED_ORDER_MSG &o);
    static CTEOrderPackage* ComposeOuchReplacedOrderMsg(OUCH_REPLACED_ORDER_MSG &o);
    static CTEOrderPackage* ComposeOuchCanceledOrderMsg(OUCH_CANCELED_ORDER_MSG &o);
    static CTEOrderPackage* ComposeOuchExecutedOrderMsg(OUCH_EXECUTED_ORDER_MSG &o);
    static CTEOrderPackage* ComposeOuchRejectedOrderMsg(OUCH_REJECTED_ORDER_MSG &o);
    static CTEOrderPackage* ComposeOuchBrokenTradeMsg(OUCH_BROKEN_TRADE_MSG &o);
    static CTEOrderPackage* ComposeOuchRestatementMsg(OUCH_RESTATEMENT_MSG &o);
};
