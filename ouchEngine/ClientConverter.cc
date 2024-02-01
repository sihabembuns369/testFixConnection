#include "header/ClientConverter.h"
#include "header/GPackage.h"
#include "header/IPackage.h"
#include "header/constants.h"
#include "header/IConstants.h"
#include "header/TEOrderPackage.h"

void CTokenMap::Release()
{
	pthread_rwlock_wrlock(&rwlock);
	tokenMap.clear();
	pthread_rwlock_unlock(&rwlock);
}

bool CTokenMap::isRegisterTokenId(uint32_t tokenId)
{
	pthread_rwlock_rdlock(&rwlock);
	_TOKENMAP_::const_iterator i = tokenMap.find(tokenId);
	bool b = (i != tokenMap.end());
	pthread_rwlock_unlock(&rwlock);
	return b;
}

bool CTokenMap::RegisterToken(uint32_t tokenId, const char *szOrderId)
{
	pthread_rwlock_wrlock(&rwlock);
	_TOKENMAP_::const_iterator i = tokenMap.find(tokenId);
	bool b = (i == tokenMap.end());
	if (b)
		tokenMap.insert(_TOKENMAP_::value_type(tokenId, szOrderId));
	pthread_rwlock_unlock(&rwlock);
	return b;
}

bool CTokenMap::GetOrderIdByToken(uint32_t tokenId, char *szOrderId, size_t slen)
{
	pthread_rwlock_rdlock(&rwlock);
	_TOKENMAP_::const_iterator i = tokenMap.find(tokenId);
	bool b = (i != tokenMap.end());
	if (b)
		strcpy(szOrderId, i->second.c_str());
	else
		memset(szOrderId, 0x00, slen);
	pthread_rwlock_unlock(&rwlock);
	return b;
}

CIPackage* CClientConverter::ILogoutRequest(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(170, IPKG_LOGOUT_REQ);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddInt8(srcId);
	return o;
}

CIPackage* CClientConverter::IComposeLogoutRequest(const char *clientIP, const char *loginId, const char *ref, uint8_t srcId)
{
	CIPackage *o = new CIPackage(170, IPKG_LOGOUT_REQ);
	o->SetSourceId(srcId);
	o->AddString(clientIP);
	o->AddString(loginId);
	o->AddInt8(srcId);
	return o;
}

CIPackage* CClientConverter::ILoginRequest(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPassword[ILOGINPWD_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPassword);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(170, IPKG_LOGIN_REQ);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPassword);
	o->AddString(szRef);
	return o;
}

CIPackage* CClientConverter::IValidatePIN(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(170, IPKG_VALIDATE_PIN);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szRef);
	return o;
}

CIPackage* CClientConverter::IRequestChangePwdPin(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szCurPIN[ILOGINPIN_LEN], szNewPIN[ILOGINPIN_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	uint8_t type = pkg->GetInt8();
	slen = pkg->GetInt16(); pkg->GetString(slen, szCurPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szNewPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(190, IPKG_CHG_PIN_PWD);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddInt8(type);
	o->AddString(szCurPIN);
	o->AddString(szNewPIN);
	o->AddString(szRef);
	return o;
}

CIPackage* CClientConverter::IRequestTradingLimit(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szStockCode[ISTOCKCODE_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szStockCode);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(220, IPKG_REQ_TL);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szStockCode);
	o->AddString(szRef);
	return o;
}

CIPackage* CClientConverter::IRequestStockBalance(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(220, IPKG_REQ_STK_BALANCE);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	return o;
}

CIPackage* CClientConverter::IRequestAccountInfo(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(220, IPKG_REQ_ACC_INFO);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	return o;
}

CIPackage* CClientConverter::IRequestRealizedGainLoss(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
    uint32_t period = pkg->GetInt32();
	CIPackage *o = new CIPackage(224, IPKG_REQ_REALIZED_GAIN_LOSS);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt32(period);
	return o;
}

CIPackage* CClientConverter::IRequestTradeConfirmation(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        reqType = pkg->GetInt8();
    uint32_t tcDate = pkg->GetInt32();
	CIPackage *o = new CIPackage(224, IPKG_REQ_TRADE_CONFIRMATION);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(reqType);
    o->AddInt32(tcDate);
	return o;
}

CIPackage* CClientConverter::IRequestTaxReport(CGPackage* pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        reqType = pkg->GetInt8();
    uint32_t taxYear = pkg->GetInt32();
	CIPackage *o = new CIPackage(224, IPKG_REQ_TAX_REPORT);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(reqType);
    o->AddInt32(taxYear);
	return o;
}

CIPackage* CClientConverter::IRequestPortfolio(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(224, IPKG_REQ_PORTFOLIO);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	return o;
}

CIPackage* CClientConverter::IRequestPortfolioReturn(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        reqType = pkg->GetInt8();
    uint32_t startDate = pkg->GetInt32(),
            endDate = pkg->GetInt32();
	CIPackage *o = new CIPackage(230, IPKG_REQ_PORTFOLIO_RETURN);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(reqType);
    o->AddInt32(startDate);
    o->AddInt32(endDate);
	return o;
}

CIPackage* CClientConverter::IRequestFundWithdrawInfo(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(224, IPKG_REQ_FUND_WITHDRAW_INFO);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	return o;
}

CIPackage* CClientConverter::IRequestFundWithdrawList(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        status = pkg->GetInt8();
    uint32_t fromDate = pkg->GetInt32(),
            toDate = pkg->GetInt32();
	CIPackage *o = new CIPackage(230, IPKG_REQ_FUND_WITHDRAW_LIST);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(status);
    o->AddInt32(fromDate);
    o->AddInt32(toDate);
	return o;
}

CIPackage* CClientConverter::IRequestFundWithdraw(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
    uint32_t transferDate = pkg->GetInt32();
    uint64_t amountTransfer = pkg->GetInt64();
    uint8_t rtgs = pkg->GetInt8();
	CIPackage *o = new CIPackage(250, IPKG_REQ_FUND_WITHDRAW);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt32(transferDate);
    o->AddInt64(amountTransfer);
    o->AddInt8(rtgs);
    o->AddInt8(srcId);
	return o;
}

CIPackage* CClientConverter::IRequestCancelFundWithdraw(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
    uint64_t fundWithdrawId = pkg->GetInt64();
	CIPackage *o = new CIPackage(250, IPKG_REQ_CANCEL_FUND_WITHDRAW);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt64(fundWithdrawId);
    o->AddInt8(srcId);
	return o;
}

CIPackage* CClientConverter::IRequestFinancialHistory(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
    uint32_t period = pkg->GetInt32();
	CIPackage *o = new CIPackage(230, IPKG_REQ_FINANCIAL_HISTORY);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
    o->AddString(szRef);
    o->AddInt32(period);
	return o;
}

CIPackage* CClientConverter::IRequestCashLedger(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
    uint32_t period = pkg->GetInt32();
	CIPackage *o = new CIPackage(230, IPKG_REQ_CASH_LEDGER);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
    o->AddString(szRef);
    o->AddInt32(period);
	return o;
}

CIPackage* CClientConverter::IRequestExerciseRightWarrantInfo(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(224, IPKG_REQ_EXERCISE_INFO);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	return o;
}

CIPackage* CClientConverter::IRequestExerciseRightWarrantList(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(224, IPKG_REQ_EXERCISE_LIST);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	return o;
}

CIPackage* CClientConverter::IRequestExerciseRightWarrant(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
    slen = pkg->GetInt16(); pkg->GetString(slen, szStockCode);
    uint32_t exerciseDate = pkg->GetInt32();
    uint64_t exerciseVolume = pkg->GetInt64();
	CIPackage *o = new CIPackage(250, IPKG_REQ_EXERCISE);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddString(szStockCode);
    o->AddInt32(exerciseDate);
    o->AddInt64(exerciseVolume);
    o->AddInt8(srcId);
	return o;
}

CIPackage* CClientConverter::IRequestCancelExerciseRightWarrant(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
    uint64_t exerciseId = pkg->GetInt64();
	CIPackage *o = new CIPackage(250, IPKG_REQ_CANCEL_EXERCISE);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt64(exerciseId);
    o->AddInt8(srcId);
	return o;
}

CIPackage* CClientConverter::IRequestStockCollateralInfo(CGPackage *pkg)
{
    char szRef[IREF_LEN];
    uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
    uint16_t srcId = pkg->GetInt16();
    CIPackage *o = new CIPackage(IREF_LEN + 1, IPKG_REQ_STOCK_INFO);
    o->SetSourceId(srcId);
    o->AddString(szRef);
    return o;
}

CIPackage* CClientConverter::IRequestTransactionAndHoliday(CGPackage *pkg)
{
    char szRef[IREF_LEN];
    uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
    uint8_t srcId = pkg->GetInt8(),
        reqType = pkg->GetInt8();
    CIPackage *o = new CIPackage(IREF_LEN + 2, IPKG_REQ_TRADING_HOLIDAY);
    o->SetSourceId(srcId);
    o->AddString(szRef);
    o->AddInt8(reqType);
    return o;
}

CIPackage* CClientConverter::IRequestCorporateActionInformation(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
    slen = pkg->GetInt16(); pkg->GetString(slen, szStockCode);
	CIPackage *o = new CIPackage(250, IPKG_REQ_CA_INFO);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddString(szStockCode);
	return o;
}

CIPackage* CClientConverter::IRequestTransactionsReport(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        reqType = pkg->GetInt8();
    uint32_t period = pkg->GetInt32();
	CIPackage *o = new CIPackage(250, IPKG_REQ_TRANSACTIONS_REPORT);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(reqType);
    o->AddInt32(period);
	return o;
}

CIPackage* CClientConverter::IRequestMonthlyBalance(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        reqType = pkg->GetInt8();
    uint32_t period = pkg->GetInt32();
	CIPackage *o = new CIPackage(250, IPKG_REQ_MONTHLY_POSITION);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(reqType);
    o->AddInt32(period);
	return o;
}

CIPackage* CClientConverter::IRequestSingleStockBalance(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        iBoard = pkg->GetInt8();
    slen = pkg->GetInt16(); pkg->GetString(slen, szStockCode);
	CIPackage *o = new CIPackage(250, IPKG_REQ_SINGLE_STOCK_BALANCE);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(iBoard);
    o->AddString(szStockCode);
	return o;
}

CIPackage* CClientConverter::IRequestValidateLoginPassword(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPassword[ILOGINPWD_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPassword);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(224, IPKG_VALIDATE_LOGIN_PASSWORD);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPassword);
	o->AddString(szRef);
	return o;
}

CIPackage* CClientConverter::IRequestSpeedOrderBook(CGPackage *pkg)
{
    char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        iBoard = pkg->GetInt8();
    slen = pkg->GetInt16(); pkg->GetString(slen, szStockCode);
    uint8_t subsc = pkg->GetInt8();
	CIPackage *o = new CIPackage(250, IPKG_REQ_SPEED_ORDER_BOOK);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(iBoard);
    o->AddString(szStockCode);
    o->AddInt8(subsc);
	return o;
}

//	Request Order Packages from Client

CIPackage* CClientConverter::IRequestOrder(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        	iBoard = pkg->GetInt8(),
			command = pkg->GetInt8();
	slen = pkg->GetInt16(); pkg->GetString(slen, szStockCode);
	uint32_t price = pkg->GetInt32();
	uint64_t volume = pkg->GetInt64();
	uint8_t randomizedSplit = pkg->GetInt8();
	uint32_t nSplit = pkg->GetInt32();
	uint8_t activatePriceStep = pkg->GetInt8();
	uint32_t nPriceStep = pkg->GetInt32();
	uint8_t activateAutomaticOrder = pkg->GetInt8();
	uint32_t nAutomaticOrderPriceStep = pkg->GetInt32();
	uint8_t preventSameOrder = pkg->GetInt8();
	CIPackage *o = new CIPackage(250, IPKG_REQ_ORDER);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(iBoard);
	o->AddInt8(srcId);
	o->AddInt8(command);
    o->AddString(szStockCode);
	o->AddInt32(price);
	o->AddInt64(volume);
	o->AddInt8(randomizedSplit);
	o->AddInt32(nSplit);
	o->AddInt8(activatePriceStep);
	o->AddInt32(nPriceStep);
	o->AddInt8(activateAutomaticOrder);
	o->AddInt32(nAutomaticOrderPriceStep);
	o->AddInt8(preventSameOrder);
	return o;
}

CIPackage* CClientConverter::IRequestAmend(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szStockCode[ISTOCKCODE_LEN];
	char szOrderId[32] = {}, szJATSId[32] = {};
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        	iBoard = pkg->GetInt8(),
			command = pkg->GetInt8();
	slen = pkg->GetInt16(); pkg->GetString(slen, szOrderId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szJATSId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szStockCode);
	uint32_t newPrice = pkg->GetInt32();
	uint64_t newVolume = pkg->GetInt64();
	CIPackage *o = new CIPackage(270, IPKG_REQ_AMEND);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(iBoard);
	o->AddInt8(srcId);
	o->AddInt8(command);
	o->AddString(szOrderId);
	o->AddString(szJATSId);
    o->AddString(szStockCode);
	o->AddInt32(newPrice);
	o->AddInt64(newVolume);
	return o;
}

CIPackage* CClientConverter::IRequestWithdraw(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szStockCode[ISTOCKCODE_LEN];
	char szOrderId[32] = {}, szJATSId[32] = {};
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        	iBoard = pkg->GetInt8(),
			command = pkg->GetInt8();
	slen = pkg->GetInt16(); pkg->GetString(slen, szOrderId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szJATSId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szStockCode);
	uint32_t price = pkg->GetInt32();
	CIPackage *o = new CIPackage(270, IPKG_REQ_WITHDRAW);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(iBoard);
	o->AddInt8(srcId);
	o->AddInt8(command);
	o->AddString(szOrderId);
	o->AddString(szJATSId);
    o->AddString(szStockCode);
	o->AddInt32(price);
	return o;
}

CIPackage* CClientConverter::IRequestOrderList(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(180, IPKG_REQ_ORDER_LIST);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	return o;
}

CIPackage* CClientConverter::IRequestTradeList(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	CIPackage *o = new CIPackage(180, IPKG_REQ_TRADE_LIST);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	return o;
}

CIPackage* CClientConverter::IRequestRejectedOrderMessage(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szOrderId[32] = {};
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	slen = pkg->GetInt16(); pkg->GetString(slen, szOrderId);
	CIPackage *o = new CIPackage(210, IPKG_REQ_REJECTED_ORDER_MESSAGE);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	o->AddString(szOrderId);
	return o;
}

CIPackage* CClientConverter::IRequestHistoricalOrderList(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	uint32_t fdate = pkg->GetInt32(),
		tdate = pkg->GetInt32();
	CIPackage *o = new CIPackage(190, IPKG_REQ_HISTORICAL_ORDER_LIST);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	o->AddInt32(fdate);
	o->AddInt32(tdate);
	return o;
}

CIPackage* CClientConverter::IRequestHistoricalTradeList(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	uint32_t fdate = pkg->GetInt32(),
		tdate = pkg->GetInt32();
	CIPackage *o = new CIPackage(190, IPKG_REQ_HISTORICAL_TRADE_LIST);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	o->AddInt32(fdate);
	o->AddInt32(tdate);
	return o;
}

CIPackage* CClientConverter::IRequestGTCOrder(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        	iBoard = pkg->GetInt8(),
			command = pkg->GetInt8();
	slen = pkg->GetInt16(); pkg->GetString(slen, szStockCode);
	uint32_t price = pkg->GetInt32();
	uint64_t volume = pkg->GetInt64();
	uint8_t activateAutomaticOrder = pkg->GetInt8();
	uint32_t nAutomaticOrderPriceStep = pkg->GetInt32(),
		effectiveDate = pkg->GetInt32(),
		dueDate = pkg->GetInt32();
	CIPackage *o = new CIPackage(250, IPKG_REQ_NEW_GTC_ORDER);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(iBoard);
	o->AddInt8(srcId);
	o->AddInt8(command);
    o->AddString(szStockCode);
	o->AddInt32(price);
	o->AddInt64(volume);
	o->AddInt8(activateAutomaticOrder);
	o->AddInt32(nAutomaticOrderPriceStep);
	o->AddInt32(effectiveDate);
	o->AddInt32(dueDate);
	return o;
}

CIPackage* CClientConverter::IRequestCancelGTCOrder(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szGTCId[ISTOCKCODE_LEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	slen = pkg->GetInt16(); pkg->GetString(slen, szGTCId);
	CIPackage *o = new CIPackage(210, IPKG_REQ_CANCEL_GTC_ORDER);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	o->AddInt8(srcId);
	o->AddString(szGTCId);
	return o;
}

CIPackage* CClientConverter::IRequestGTCOrderList(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
		status = pkg->GetInt8();
	CIPackage *o = new CIPackage(180, IPKG_REQ_GTC_ORDER_LIST);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	o->AddInt8(srcId);
	o->AddInt8(status);
	return o;
}

CIPackage* CClientConverter::IRequestBreakOrderList(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
		status = pkg->GetInt8();
	CIPackage *o = new CIPackage(180, IPKG_REQ_BREAK_ORDER_LIST);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	o->AddInt8(srcId);
	o->AddInt8(status);
	return o;
}

CIPackage* CClientConverter::IRequestTrailingOrderList(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
		status = pkg->GetInt8();
	CIPackage *o = new CIPackage(180, IPKG_REQ_TRAILING_ORDER_LIST);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	o->AddInt8(srcId);
	o->AddInt8(status);
	return o;
}

CIPackage* CClientConverter::IRequestBreakOrder(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        	iBoard = pkg->GetInt8(),
			command = pkg->GetInt8();
	uint32_t price = pkg->GetInt32();
	uint64_t volume = pkg->GetInt64();
	// Conditional Price Type
	// 0 : Best Bid Price
	// 1 : Best Offer Price
	// 2 : Last Price
	// 3 : Avg Price
	// Conditional Price Comparison
	// 0 : = (Equal To)
	// 1 : < (Less Than)
	// 2 : > (More Than)
	uint8_t condPriceType = pkg->GetInt8(),
		condPriceComparation = pkg->GetInt8();
	uint32_t condPrice = pkg->GetInt32();
	uint8_t condVolType = pkg->GetInt8(),
		condVolComparation = pkg->GetInt8();
	uint64_t condVolume = pkg->GetInt64();
	uint32_t nAutomaticOrderPriceStep = pkg->GetInt32(),
		effectiveDate = pkg->GetInt32(),
		dueDate = pkg->GetInt32();
	slen = pkg->GetInt16(); pkg->GetString(slen, szStockCode);
	CIPackage *o = new CIPackage(255, IPKG_REQ_NEW_BREAK_ORDER);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(iBoard);
	o->AddInt8(srcId);
	o->AddInt8(command);
	o->AddInt32(price);
	o->AddInt64(volume);
	o->AddInt8(condPriceType);
	o->AddInt8(condPriceComparation);
	o->AddInt32(condPrice);
	o->AddInt8(condVolType);
	o->AddInt8(condVolComparation);
	o->AddInt64(condVolume);
	o->AddInt32(nAutomaticOrderPriceStep);
	o->AddInt32(effectiveDate);
	o->AddInt32(dueDate);
	o->AddString(szStockCode);
	return o;
}

CIPackage* CClientConverter::IRequestTrailingOrder(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
        	iBoard = pkg->GetInt8(),
			command = pkg->GetInt8(),
			execPriceStep = pkg->GetInt8();
	uint64_t volume = pkg->GetInt64();
	uint32_t dropPrice = pkg->GetInt32(),
		trailingStep = pkg->GetInt32();	//	2 Decimals
	// Conditional Price Type
	// 0 : Best Bid Price
	// 1 : Best Offer Price
	// 2 : Last Price
	// 3 : Avg Price
	// 4 : High Price;
	// 5 : Lowest Price; ( For Trailing Buy Always 5 )
	uint8_t condPriceType = pkg->GetInt8();
	uint32_t nAutomaticOrderPriceStep = pkg->GetInt32(),
		effectiveDate = pkg->GetInt32(),
		dueDate = pkg->GetInt32();
	slen = pkg->GetInt16(); pkg->GetString(slen, szStockCode);
	CIPackage *o = new CIPackage(250, IPKG_REQ_NEW_TRAILING_ORDER);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
    o->AddInt8(iBoard);
	o->AddInt8(srcId);
	o->AddInt8(command);
	o->AddInt8(execPriceStep);
	o->AddInt64(volume);
	o->AddInt32(dropPrice);
	o->AddInt32(trailingStep);
	o->AddInt8(condPriceType);
	o->AddInt32(nAutomaticOrderPriceStep);
	o->AddInt32(effectiveDate);
	o->AddInt32(dueDate);
	o->AddString(szStockCode);
	return o;
}

CIPackage* CClientConverter::IRequestCancelBreakOrder(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szOrderId[ISTOCKCODE_LEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	slen = pkg->GetInt16(); pkg->GetString(slen, szOrderId);
	CIPackage *o = new CIPackage(200, IPKG_REQ_CANCEL_BREAK_ORDER);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	o->AddInt8(srcId);
	o->AddString(szOrderId);
	return o;
}

CIPackage* CClientConverter::IRequestCancelTrailingOrder(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szOrderId[ISTOCKCODE_LEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8();
	slen = pkg->GetInt16(); pkg->GetString(slen, szOrderId);
	CIPackage *o = new CIPackage(200, IPKG_REQ_CANCEL_TRAILING_ORDER);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	o->AddInt8(srcId);
	o->AddString(szOrderId);
	return o;
}

CIPackage* CClientConverter::IRequestSpeedOrderAmend(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
		iBoard = pkg->GetInt8();
	slen = pkg->GetInt16(); pkg->GetString(slen, szStockCode);
	CIPackage *o = new CIPackage(pkg->GetPackageLength(), IPKG_REQ_SPEED_ORDER_AMEND);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	o->AddInt8(iBoard);
	o->AddString(szStockCode);
	uint32_t i = 0, nArr = pkg->GetInt32();
	o->AddInt32(nArr);	//	Bid Array
	for (i = 0; i < nArr; i++)
	{
		o->AddInt32(pkg->GetInt32());	//	Bid Old Price
		o->AddInt32(pkg->GetInt32());	//	Bid New Price
	}
	o->AddInt32(nArr = pkg->GetInt32());	//	Offer Array
	for (i = 0; i < nArr; i++)
	{
		o->AddInt32(pkg->GetInt32());	//	Offer Old Price
		o->AddInt32(pkg->GetInt32());	//	Offer New Price
	}
	return o;
}

CIPackage* CClientConverter::IRequestSpeedOrderWithdraw(CGPackage *pkg)
{
	char szLoginId[ILOGINID_LEN], szPIN[ILOGINPIN_LEN], szAccId[IACCID_LEN], szRef[IREF_LEN], szClientIP[INET6_ADDRSTRLEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t slen = pkg->GetInt16(); pkg->GetString(slen, szClientIP);
	slen = pkg->GetInt16(); pkg->GetString(slen, szLoginId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szPIN);
	slen = pkg->GetInt16(); pkg->GetString(slen, szAccId);
	slen = pkg->GetInt16(); pkg->GetString(slen, szRef);
	uint8_t srcId = pkg->GetInt8(),
		iBoard = pkg->GetInt8();
	slen = pkg->GetInt16(); pkg->GetString(slen, szStockCode);
	uint8_t execFlags = pkg->GetInt8();
	CIPackage *o = new CIPackage(pkg->GetPackageLength(), IPKG_REQ_SPEED_ORDER_WITHDRAW);
	o->SetSourceId(srcId);
	o->AddString(szClientIP);
	o->AddString(szLoginId);
	o->AddString(szPIN);
	o->AddString(szAccId);
	o->AddString(szRef);
	o->AddInt8(iBoard);
	o->AddString(szStockCode);
	o->AddInt8(execFlags);
	uint32_t i = 0, nArr = pkg->GetInt32();
	o->AddInt32(nArr);	//	Bid Array
	for (i = 0; i < nArr; i++)
		o->AddInt32(pkg->GetInt32());	//	Bid Price
		
	o->AddInt32(nArr = pkg->GetInt32());	//	Offer Array
	for (i = 0; i < nArr; i++)
		o->AddInt32(pkg->GetInt32());	//	Offer Price
	return o;
}

//  Reply from IServer
CGPackage* CClientConverter::ILoginReply(CIPackage *p, uint32_t LOT_SIZE)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szAccName[128];
	uint16_t errCode = p->GetErrorCode(),
		olFeeBuy = 0, olFeeSell = 0,
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	uint8_t loginType = p->GetInt8();
	uint32_t permissions = p->GetInt32(),
		nAcc = p->GetInt32();
	CGPackage *o = new CGPackage(18 + ILOGINID_LEN + 
		IREF_LEN + nAcc * (IACCID_LEN + 138), IPKG_LOGIN_REQ);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddInt32(LOT_SIZE);
	if (errCode)
		return o;
	//	--------------------------------------
	o->AddInt8(loginType);
	o->AddInt32(permissions);
	o->AddInt32(nAcc);
	for (uint32_t i = 0; i < nAcc; i++)
	{
		slen = p->GetInt16(); p->GetString(slen, szAccId);
		slen = p->GetInt16(); p->GetString(slen, szAccName);
		olFeeBuy = p->GetInt16();
		olFeeSell = p->GetInt16();
		o->AddString(szAccId);
		o->AddString(szAccName);
		o->AddInt16(olFeeBuy);
		o->AddInt16(olFeeSell);
	}
	return o;
}

CGPackage* CClientConverter::IValidatePINReply(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	CGPackage *o = new CGPackage(4 + ILOGINID_LEN + IREF_LEN, IPKG_VALIDATE_PIN);
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szLoginId);
	o->AddString(szRef);
	return o;
}

CGPackage* CClientConverter::IReplyChangePwdPin(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szLoginId);
	uint8_t type = p->GetInt8();
	slen = p->GetInt16(); p->GetString(slen, szRef);
	CGPackage *o = new CGPackage(5 + ILOGINID_LEN + IREF_LEN, IPKG_CHG_PIN_PWD);
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szLoginId);
	o->AddInt8(type);
	o->AddString(szRef);
	return o;
}

CGPackage* CClientConverter::IReplyTradingLimit(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szAccName[IACCNAME_LEN] = {};
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_TL);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	if (errCode)
		return o;
	slen = p->GetInt16(); p->GetString(slen, szAccName);
	o->AddString(szAccName);
	o->AddInt64(p->GetInt64());	//	Current Cash
	o->AddInt64(p->GetInt64());	//	RDN Cash
	o->AddInt64(p->GetInt64());	//	Cash on T+3
	o->AddInt32(p->GetInt32());	//	Current Ratio
	o->AddInt64(p->GetInt64());	//	Trading Limit
	o->AddInt64(p->GetInt64());	//	Remain Trading Limit
	return o;
}

CGPackage* CClientConverter::IReplyStockBalance(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	uint32_t nArr = p->GetInt32();
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_STK_BALANCE);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt32(nArr);
	for (uint32_t i = 0; i < nArr; i++)
	{
		slen = p->GetInt16(); p->GetString(slen, szStockCode);
		o->AddInt16(slen);
		o->AddString(szStockCode, slen);
		o->AddInt32(p->GetInt32());	//	Avg Price
		o->AddInt32(p->GetInt32());	//	Last Price
		o->AddInt64(p->GetInt64());	//	Volume in shares
		o->AddInt64(p->GetInt64());	//	Balance in shares
		o->AddInt64(p->GetInt64());	//	Potential Gain / Loss
		o->AddInt32(p->GetInt32());	//	Potential Gain / Loss in Percentage
	}
	return o;
}

CGPackage* CClientConverter::IReplyAccountInfo(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	char szKSEIAccNo[ISTR32_LEN], szSID[ISTR32_LEN], szAccStatus[ISTR32_LEN], szIDType[ISTR32_LEN];
	char szID[ISTR32_LEN], szCustType[ISTR32_LEN], szNationality[ISTR32_LEN], szMotherName[IACCNAME_LEN];
	char szJob[ISTR64_LEN], szCompanyName[ISTR64_LEN], szManagingBranch[ISTR64_LEN], szBankCode[ISTR64_LEN];
	char szBankAccNo[ISTR64_LEN], szBankAccName[IACCNAME_LEN];
	char szRDNBankCode[ISTR64_LEN], szRDNBankAccNo[ISTR64_LEN], szRDNBankAccName[IACCNAME_LEN];
	char szEmail[256], szPhone[ISTR32_LEN], szHP[ISTR32_LEN], szOfficePhone[ISTR32_LEN];
	char szHomeAddr[256], szOfficeAddr[256];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength() + CGPackage::GP_HEADER_LENGTH, IPKG_REQ_ACC_INFO);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	if (errCode)
		return o;
	o->AddInt32(p->GetInt32());	//	OL Fee Buy
	o->AddInt32(p->GetInt32());	//	OL Fee Sell
	slen = p->GetInt16(); p->GetString(slen, szKSEIAccNo);
	slen = p->GetInt16(); p->GetString(slen, szSID);
	slen = p->GetInt16(); p->GetString(slen, szAccStatus);
	slen = p->GetInt16(); p->GetString(slen, szIDType);
	slen = p->GetInt16(); p->GetString(slen, szID);
	o->AddString(szKSEIAccNo);
	o->AddString(szSID);
	o->AddString(szAccStatus);
	o->AddString(szIDType);
	o->AddString(szID);
	o->AddInt32(p->GetInt32());	//	ID Expiry
	slen = p->GetInt16(); p->GetString(slen, szCustType);
	slen = p->GetInt16(); p->GetString(slen, szNationality);
	slen = p->GetInt16(); p->GetString(slen, szMotherName);
	slen = p->GetInt16(); p->GetString(slen, szJob);
	slen = p->GetInt16(); p->GetString(slen, szCompanyName);
	o->AddString(szCustType);
	o->AddString(szNationality);
	o->AddString(szMotherName);
	o->AddString(szJob);
	o->AddString(szCompanyName);
	o->AddInt32(p->GetInt32());	//	Opening Account Date
	slen = p->GetInt16(); p->GetString(slen, szManagingBranch);
	slen = p->GetInt16(); p->GetString(slen, szBankCode);
	slen = p->GetInt16(); p->GetString(slen, szBankAccNo);
	slen = p->GetInt16(); p->GetString(slen, szBankAccName);
	slen = p->GetInt16(); p->GetString(slen, szRDNBankCode);
	slen = p->GetInt16(); p->GetString(slen, szRDNBankAccNo);
	slen = p->GetInt16(); p->GetString(slen, szRDNBankAccName);
	o->AddString(szManagingBranch);
	o->AddString(szBankCode);
	o->AddString(szBankAccNo);
	o->AddString(szBankAccName);
	o->AddString(szRDNBankCode);
	o->AddString(szRDNBankAccNo);
	o->AddString(szRDNBankAccName);
	o->AddInt32(p->GetInt32());	//	RDN Bank Account Open Date
	slen = p->GetInt16(); p->GetString(slen, szEmail);
	slen = p->GetInt16(); p->GetString(slen, szPhone);
	slen = p->GetInt16(); p->GetString(slen, szHP);
	slen = p->GetInt16(); p->GetString(slen, szOfficePhone);
	slen = p->GetInt16(); p->GetString(slen, szHomeAddr);
	slen = p->GetInt16(); p->GetString(slen, szOfficeAddr);
	o->AddString(szEmail);
	o->AddString(szPhone);
	o->AddString(szHP);
	o->AddString(szOfficePhone);
	o->AddString(szHomeAddr);
	o->AddString(szOfficeAddr);
	return o;
}

CGPackage* CClientConverter::IReplyRealizedGainLoss(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	uint32_t period = p->GetInt32(),
		nArr = p->GetInt32();
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_REALIZED_GAIN_LOSS);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt32(period);
	o->AddInt32(nArr);
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt32(p->GetInt32());	//	Transaction Date
		o->AddInt8(p->GetInt8());	//	Command
		o->AddInt16(slen = p->GetInt16());	//	slen
		p->GetString(slen, szStockCode);
		o->AddString(szStockCode, slen);	//	StockCode
		o->AddInt64(p->GetInt64());	//	Avg Price
		o->AddInt32(p->GetInt32());	//	Price
		o->AddInt64(p->GetInt64());	//	Volume in shares
		o->AddInt64(p->GetInt64());	//	Gain / Loss
		o->AddInt64(p->GetInt64());	//	Total Gain / Loss
	}
	return o;
}

CGPackage* CClientConverter::IReplyTradeConfirmation(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	uint8_t reqType = p->GetInt8();
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_TRADE_CONFIRMATION);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt8(reqType);
	if (errCode)
		return o;
	if (0 == reqType)	//	TC Date List
	{
		uint32_t nArr = p->GetInt32();
		o->AddInt32(nArr);	//	Array TC Date List
		for (uint32_t i = 0; i < nArr; i++)
			o->AddInt32(p->GetInt32());	//	TC Date List
		return o;
	}
	//	TC Data ( reqType=1 )
	uint32_t tcDate = p->GetInt32(),
		nArr = p->GetInt32(), i = 0;
	o->AddInt32(tcDate);
	o->AddInt32(nArr);	//	Array Buy
	for (i = 0; i < nArr; i++)
	{
		o->AddInt16(slen = p->GetInt16());	//	slen
		p->GetString(slen, szStockCode);
		o->AddString(szStockCode, slen);	//	StockCode
		o->AddInt32(p->GetInt32());	//	Price
		o->AddInt64(p->GetInt64());	//	Qty
		o->AddInt64(p->GetInt64());	//	Gross Amount
		o->AddInt64(p->GetInt64());	//	Broker Fee
		o->AddInt64(p->GetInt64());	//	Levy
		o->AddInt64(p->GetInt64());	//	WHT
		o->AddInt64(p->GetInt64());	//	VAT
		o->AddInt64(p->GetInt64());	//	Net Amount
	}
	o->AddInt64(p->GetInt64());	//	Total Gross Amount
	o->AddInt64(p->GetInt64());	//	Total Broker Fee
	o->AddInt64(p->GetInt64());	//	Total Levy
	o->AddInt64(p->GetInt64());	//	Total WHT
	o->AddInt64(p->GetInt64());	//	Total VAT
	o->AddInt64(p->GetInt64());	//	Total Net Amount
	//	Sell
	o->AddInt32(nArr = p->GetInt32());	//	Array Sell
	for (i = 0; i < nArr; i++)
	{
		o->AddInt16(slen = p->GetInt16());	//	slen
		p->GetString(slen, szStockCode);
		o->AddString(szStockCode, slen);	//	StockCode
		o->AddInt32(p->GetInt32());	//	Price
		o->AddInt64(p->GetInt64());	//	Qty
		o->AddInt64(p->GetInt64());	//	Gross Amount
		o->AddInt64(p->GetInt64());	//	Broker Fee
		o->AddInt64(p->GetInt64());	//	Levy
		o->AddInt64(p->GetInt64());	//	WHT
		o->AddInt64(p->GetInt64());	//	VAT
		o->AddInt64(p->GetInt64());	//	Sales Tax
		o->AddInt64(p->GetInt64());	//	Net Amount
	}
	o->AddInt64(p->GetInt64());	//	Total Gross Amount
	o->AddInt64(p->GetInt64());	//	Total Broker Fee
	o->AddInt64(p->GetInt64());	//	Total Levy
	o->AddInt64(p->GetInt64());	//	Total WHT
	o->AddInt64(p->GetInt64());	//	Total VAT
	o->AddInt64(p->GetInt64());	//	Total Sales Tax
	o->AddInt64(p->GetInt64());	//	Total Net Amount
	o->AddInt64(p->GetInt64());	//	Net Purchase ( Sell )
	o->AddInt64(p->GetInt64());	//	Grand Total Broker Fee
	o->AddInt64(p->GetInt64());	//	Grand Total Levy
	o->AddInt64(p->GetInt64());	//	Grand Total WHT
	o->AddInt64(p->GetInt64());	//	Grand Total VAT
	o->AddInt64(p->GetInt64());	//	Grand Total Sales Tax
	o->AddInt64(p->GetInt64());	//	Grand Total Net Amount
	o->AddInt64(p->GetInt64());	//	Other Fee ( Biaya Materai )
	return o;
}

CGPackage* CClientConverter::IReplyTaxReport(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szStockCode[ISTOCKCODE_LEN], szBoard[IBOARD_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	uint8_t reqType = p->GetInt8();
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_TAX_REPORT);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt8(reqType);
	if (errCode)
		return o;
	if (0 == reqType)	//	Tax Report Date List
	{
		uint32_t nArr = p->GetInt32();
		o->AddInt32(nArr);
		for (uint32_t i = 0; i < nArr; i++)
			o->AddInt32(p->GetInt32());	//	Tax Report Year List ( yyyy )
		return o;
	}
	uint32_t taxReportYear = p->GetInt32(),
		nArr = p->GetInt32(), i = 0;
	o->AddInt32(taxReportYear);
	o->AddInt32(nArr);
	for (i = 0; i < nArr; i++)
	{
		o->AddInt32(p->GetInt32());	//	Date yyyymmdd
		o->AddInt16(slen = p->GetInt16());	//	slen Board
		p->GetString(slen, szBoard);
		o->AddString(szBoard, slen);
		o->AddInt16(slen = p->GetInt16());	//	slen StockCode
		p->GetString(slen, szStockCode);
		o->AddString(szStockCode, slen);
		o->AddInt32(p->GetInt32());			//	Price
		o->AddInt64(p->GetInt64());			//	Volume ( Qty )
		o->AddInt64(p->GetInt64());			//	Gross Amount
		o->AddInt64(p->GetInt64());			//	Broker Fee
		o->AddInt64(p->GetInt64());			//	Levy
		o->AddInt64(p->GetInt64());			//	WHT
		o->AddInt64(p->GetInt64());			//	VAT
		o->AddInt64(p->GetInt64());			//	Sales Tax
		o->AddInt64(p->GetInt64());			//	Nett Amount
	}
	o->AddInt64(p->GetInt64());			//	Total Gross Amount
	o->AddInt64(p->GetInt64());			//	Total Broker Fee
	o->AddInt64(p->GetInt64());			//	Total Levy
	o->AddInt64(p->GetInt64());			//	Total WHT
	o->AddInt64(p->GetInt64());			//	Total VAT
	o->AddInt64(p->GetInt64());			//	Total Sales Tax
	o->AddInt64(p->GetInt64());			//	Total Nett Amount
	return o;
}

CGPackage* CClientConverter::IReplyPortfolio(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_PORTFOLIO);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	if (errCode)
		return o;
	o->AddInt64(p->GetInt64());	//	Current Cash
	o->AddInt64(p->GetInt64());	//	Cash on T+2
	o->AddInt64(p->GetInt64());	//	Remain Trading Limit ( TL - UsedTL )
	o->AddInt32(p->GetInt32());	//	Current Ratio
	o->AddInt32(p->GetInt32());	//	Market Ratio
	o->AddInt32(p->GetInt32());	//	Potential Ratio
	uint32_t nArr = p->GetInt32();	//	Array
	o->AddInt32(nArr);
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt16(slen = p->GetInt16());	//	slen StockCode
		p->GetString(slen, szStockCode);
		o->AddString(szStockCode, slen);	//	StockCode
		o->AddInt32(p->GetInt32());	//	Avg Price
		o->AddInt32(p->GetInt32());	//	Last Price
		o->AddInt64(p->GetInt64());	//	Volume
		o->AddInt64(p->GetInt64());	//	Balance

		o->AddInt64(p->GetInt64());	//	Open Sell Volume
		o->AddInt64(p->GetInt64());	//	Open Buy Volume
		o->AddInt64(p->GetInt64());	//	Market Value
		o->AddInt32(p->GetInt32());	//	Haircut
		o->AddInt64(p->GetInt64());	//	Potential Gain Loss
		o->AddInt32(p->GetInt32());	//	Potential Gain Loss Percentage
		o->AddInt8(p->GetInt8());	//	Marginable
		o->AddInt8(p->GetInt8());	//	Has Corporate Actions
	}
	o->AddInt64(p->GetInt64());	//	Total Market Value
	o->AddInt64(p->GetInt64());	//	Total Potential Gain / Loss
	o->AddInt32(p->GetInt32());	//	Total Potential Gain / Loss Percentage
	//	AR / AP Info
	o->AddInt32(p->GetInt32());	//	T0 Date
	o->AddInt64(p->GetInt64());	//	T0 AR
	o->AddInt64(p->GetInt64());	//	T0 AP
	o->AddInt64(p->GetInt64());	//	T0 Fund Transfer Request
	o->AddInt64(p->GetInt64());	//	T0 Net Cash

	o->AddInt32(p->GetInt32());	//	T1 Date
	o->AddInt64(p->GetInt64());	//	T1 AR
	o->AddInt64(p->GetInt64());	//	T1 AP
	o->AddInt64(p->GetInt64());	//	T1 Fund Transfer Request
	o->AddInt64(p->GetInt64());	//	T1 Net Cash

	o->AddInt32(p->GetInt32());	//	T2 Date
	o->AddInt64(p->GetInt64());	//	T2 AR
	o->AddInt64(p->GetInt64());	//	T2 AP
	o->AddInt64(p->GetInt64());	//	T2 Fund Transfer Request
	o->AddInt64(p->GetInt64());	//	T2 Net Cash

	o->AddInt32(p->GetInt32());	//	T3 Date
	o->AddInt64(p->GetInt64());	//	T3 AR
	o->AddInt64(p->GetInt64());	//	T3 AP
	o->AddInt64(p->GetInt64());	//	T3 Fund Transfer Request
	o->AddInt64(p->GetInt64());	//	T3 Net Cash
	o->AddInt64(p->GetInt64());	//	Interest
	return o;
}

CGPackage* CClientConverter::IReplyPortfolioReturn(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_PORTFOLIO_RETURN);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt8(p->GetInt8());	//	Request Type
	o->AddInt32(p->GetInt32());	//	Start Date
	o->AddInt32(p->GetInt32());	//	End Date
	uint32_t nArr = p->GetInt32();
	o->AddInt32(nArr);			//	Array
	if (errCode)
		return o;
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt32(p->GetInt32());	//	Date ( Daily ) / Monthly ( yyyymm )
		o->AddInt64(p->GetInt64());	//	Asset Value
		o->AddInt64(p->GetInt64());	//	Deposit
		o->AddInt64(p->GetInt64());	//	Withdraw
		o->AddInt64(p->GetInt64());	//	Unrealize Gain / Loss
		o->AddInt32(p->GetInt32());	//	Yield ( % )
	}
	o->AddInt64(p->GetInt64());	//	Total Asset Value
	o->AddInt64(p->GetInt64());	//	Total Deposit
	o->AddInt64(p->GetInt64());	//	Total Withdraw
	o->AddInt64(p->GetInt64());	//	Total Unrealize Gain / Loss
	o->AddInt32(p->GetInt32());	//	Total Yield ( % )
	return o;
}

CGPackage* CClientConverter::IReplyFundWithdrawInfo(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szBankAccNo[ISTR32_LEN], szBankAccName[IACCNAME_LEN], szBank[ISTR32_LEN], szBankBranch[ISTR32_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_FUND_WITHDRAW_INFO);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	if (errCode)
		return o;
	o->AddInt32(p->GetInt32());	//	T0 Date
	o->AddInt64(p->GetInt64());	//	T0 Cash
	o->AddInt32(p->GetInt32());	//	T1 Date
	o->AddInt64(p->GetInt64());	//	T1 Cash
	o->AddInt32(p->GetInt32());	//	T2 Date
	o->AddInt64(p->GetInt64());	//	T2 Cash
	o->AddInt32(p->GetInt32());	//	T3 Date
	o->AddInt64(p->GetInt64());	//	T3 Cash
	o->AddInt32(p->GetInt32());	//	Min Amount
	o->AddInt32(p->GetInt32());	//	Kliring Fee
	o->AddInt32(p->GetInt32());	//	RTGS Fee
	o->AddInt32(p->GetInt32());	//	Pindah Buku Fee
	slen = p->GetInt16(); p->GetString(slen, szBankAccNo);
	slen = p->GetInt16(); p->GetString(slen, szBankAccName);
	slen = p->GetInt16(); p->GetString(slen, szBank);
	slen = p->GetInt16(); p->GetString(slen, szBankBranch);
	o->AddString(szBankAccNo);
	o->AddString(szBankAccName);
	o->AddString(szBank);
	o->AddString(szBankBranch);
	return o;
}

CGPackage* CClientConverter::IReplyFundWithdrawList(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_FUND_WITHDRAW_LIST);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt8(p->GetInt8());	//	Status
	o->AddInt32(p->GetInt32());	//	From Date
	o->AddInt32(p->GetInt32());	//	To Date
	uint32_t nArr = p->GetInt32();	//	Array
	o->AddInt32(nArr);				//	Array
	char szBankId[ISTR32_LEN], szBankAccount[ISTR32_LEN], szInputUser[ISTR64_LEN], szMessage[ISTR128_LEN];
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt64(p->GetInt64());	//	Transfer Id
		o->AddInt32(p->GetInt32());	//	Subscribe Date
		o->AddInt32(p->GetInt32());	//	Subscribe Time
		o->AddInt32(p->GetInt32());	//	Transfer Date
		o->AddInt32(p->GetInt32());	//	Executed Date
		o->AddInt64(p->GetInt64());	//	Amount Transfer
		o->AddInt32(p->GetInt32());	//	Fee
		o->AddInt8(p->GetInt8());	//	RTGS
		slen = p->GetInt16(); p->GetString(slen, szBankId);
		slen = p->GetInt16(); p->GetString(slen, szBankAccount);
		o->AddString(szBankId);
		o->AddString(szBankAccount);
		o->AddInt8(p->GetInt8());	//	Status
		slen = p->GetInt16(); p->GetString(slen, szInputUser);
		slen = p->GetInt16(); p->GetString(slen, szMessage);
		o->AddString(szInputUser);
		o->AddString(szMessage);
	}
	return o;
}

CGPackage* CClientConverter::IReplyFundWithdraw(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength() + SZ_INT64, IPKG_REQ_FUND_WITHDRAW);
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt64(p->GetInt64());	//	Transfer Id
	return o;
}

CGPackage* CClientConverter::IReplyCancelFundWithdraw(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength() + SZ_INT64, IPKG_REQ_CANCEL_FUND_WITHDRAW);
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt64(p->GetInt64());	//	Transfer Id
	return o;
}

CGPackage* CClientConverter::IReplyFinancialHistory(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szDesc[ISTR64_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_FINANCIAL_HISTORY);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt32(p->GetInt32());	//	Period
	if (errCode)
		return o;
	o->AddInt64(p->GetInt64());	//	RDN Cash Balance
	uint32_t nArr = p->GetInt32();	//	Array
	o->AddInt32(nArr);	//	Array
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt32(p->GetInt32());	//	Transaction Date
		o->AddInt16(slen = p->GetInt16());	//	slen Description
		p->GetString(slen, szDesc);
		o->AddString(szDesc, slen);
		o->AddInt64(p->GetInt64());	//	Debit
		o->AddInt64(p->GetInt64());	//	Credit
		o->AddInt64(p->GetInt64());	//	Balance
	}
	return o;
}

CGPackage* CClientConverter::IReplyCashLedger(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szDesc[ISTR64_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_CASH_LEDGER);
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt32(p->GetInt32());	//	Period
	uint32_t nArr = p->GetInt32();	//	Array
	o->AddInt32(nArr);	//	Array
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt32(p->GetInt32());	//	Transaction Date
		o->AddInt16(slen = p->GetInt16());	//	slen Description
		p->GetString(slen, szDesc);
		o->AddString(szDesc, slen);
		o->AddInt64(p->GetInt64());	//	Debit
		o->AddInt64(p->GetInt64());	//	Credit
		o->AddInt64(p->GetInt64());	//	Balance
	}
	return o;
}

CGPackage* CClientConverter::IReplyExerciseRightWarrantInfo(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_EXERCISE_INFO);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	if (errCode)
		return o;
	o->AddInt32(p->GetInt32());	//	T0 Date
	o->AddInt64(p->GetInt64());	//	T0 Cash
	o->AddInt32(p->GetInt32());	//	T1 Date
	o->AddInt64(p->GetInt64());	//	T1 Cash
	o->AddInt32(p->GetInt32());	//	T2 Date
	o->AddInt64(p->GetInt64());	//	T2 Cash
	o->AddInt32(p->GetInt32());	//	T3 Date
	o->AddInt64(p->GetInt64());	//	T3 Cash
	uint32_t nArr = p->GetInt32();	//	Array
	o->AddInt32(nArr);
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt16(slen = p->GetInt16());
		p->GetString(slen, szStockCode);
		o->AddString(szStockCode, slen);
		o->AddInt32(p->GetInt32());	//	Exercise Price
		o->AddInt64(p->GetInt64());	//	Volume Balance
		o->AddInt32(p->GetInt32());	//	Old Shares
		o->AddInt32(p->GetInt32());	//	New Shares
		o->AddInt32(p->GetInt32());	//	Exercise Date Begin
		o->AddInt32(p->GetInt32());	//	Exercise Date End
	}
	return o;
}

CGPackage* CClientConverter::IReplyExerciseRightWarrantList(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szStockCode[ISTOCKCODE_LEN], szMessage[ISTR128_LEN], szInputUser[ISTR64_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_EXERCISE_LIST);
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	uint32_t nArr = p->GetInt32();	//	Array
	o->AddInt32(nArr);				//	Array
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt64(p->GetInt64());	//	Exercise Id
		o->AddInt32(p->GetInt32());	//	Subscribe Date
		o->AddInt32(p->GetInt32());	//	Subscribe Time
		o->AddInt32(p->GetInt32());	//	Exercise Date
		o->AddInt32(p->GetInt32());	//	Executed Date
		o->AddInt16(slen = p->GetInt16());	//	slen StockCode
		p->GetString(slen, szStockCode);
		o->AddString(szStockCode, slen);
		o->AddInt32(p->GetInt32());	//	Exercise Price
		o->AddInt64(p->GetInt64());	//	Exercise Qty
		o->AddInt64(p->GetInt64());	//	Exercise Amount
		o->AddInt32(p->GetInt32());	//	Exercise Fee
		o->AddInt8(p->GetInt8());	//	Source Id
		o->AddInt8(p->GetInt8());	//	Status
		o->AddInt16(slen = p->GetInt16());	//	slen InputUser
		p->GetString(slen, szInputUser);
		o->AddString(szInputUser, slen);
		o->AddInt16(slen = p->GetInt16());	//	slen Message
		p->GetString(slen, szMessage);
		o->AddString(szMessage, slen);
	}
	return o;
}

CGPackage* CClientConverter::IReplyExerciseRightWarrant(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength() + SZ_INT64, IPKG_REQ_EXERCISE);
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt64(p->GetInt64());	//	Exercise Id
	return o;
}

CGPackage* CClientConverter::IReplyCancelExerciseRightWarrant(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength() + SZ_INT64, IPKG_REQ_CANCEL_EXERCISE);
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt64(p->GetInt64());	//	Exercise Id
	return o;
}

CGPackage* CClientConverter::IReplyStockCollateralInfo(CIPackage *p)
{
	char szStockCode[ISTOCKCODE_LEN], szRef[IREF_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szRef);
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_STOCK_INFO);
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szRef);
	uint32_t nArr = p->GetInt32();
	o->AddInt32(nArr);				//	Array
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt16(slen = p->GetInt16());
		p->GetString(slen, szStockCode);
		o->AddString(szStockCode, slen);
		o->AddInt16(p->GetInt16());	//	Collateral
		o->AddInt32(p->GetInt32());	//	Status
	}
	return o;
}

CGPackage* CClientConverter::IReplyTransactionAndHoliday(CIPackage *p)
{
	char szRef[IREF_LEN], szDesc[ISTR64_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szRef);
	uint8_t reqType = p->GetInt8();
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_TRADING_HOLIDAY);
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szRef);
	o->AddInt8(reqType);	//	Request Type
	uint32_t nArr = p->GetInt32();
	o->AddInt32(nArr);
	if (0 == reqType)	//	Request Trading Date ( T0 - T7 )
	{
		for (uint32_t i = 0; i < nArr; i++)
			o->AddInt32(p->GetInt32());	//	Trading Date ( T0 - T7 )
		return o;
	}
	//	RequestType = 1 ( Request Holiday )
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt32(p->GetInt32());	//	Holiday ( yyyymmdd )
		o->AddInt16(slen = p->GetInt16());	//	slen Description
		p->GetString(slen, szDesc);
		o->AddString(szDesc, slen);
	}
	return o;
}

CGPackage* CClientConverter::IReplyCorporateActionInformation(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	slen = p->GetInt16(); p->GetString(slen, szStockCode);
	uint32_t nArr = p->GetInt32();
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_CA_INFO);
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddString(szStockCode);
	o->AddInt32(nArr);
	char szCACode[ISTR32_LEN], szCAType[8], szCATypeDesc[ISTR128_LEN], szCADesc[ISTR128_LEN];
	for (uint32_t i = 0; i < nArr; i++)
	{
		slen = p->GetInt16(); p->GetString(slen, szCACode);
		slen = p->GetInt16(); p->GetString(slen, szCAType);
		slen = p->GetInt16(); p->GetString(slen, szCATypeDesc);
		o->AddString(szCACode);
		o->AddString(szCAType);
		o->AddString(szCATypeDesc);
		o->AddInt32(p->GetInt32());	//	Old Shares
		o->AddInt32(p->GetInt32());	//	New Shares
		o->AddInt32(p->GetInt32());	//	Price
		slen = p->GetInt16(); p->GetString(slen, szCADesc);
		o->AddString(szCADesc);
		o->AddInt32(p->GetInt32());	//	Start Date
		o->AddInt32(p->GetInt32());	//	End Date
		o->AddInt32(p->GetInt32());	//	Cum Date
		o->AddInt32(p->GetInt32());	//	Ex Date
		o->AddInt32(p->GetInt32());	//	Rec Date
		o->AddInt32(p->GetInt32());	//	Pay Date
		o->AddInt32(p->GetInt32());	//	Meet Date
	}
	return o;
}

CGPackage* CClientConverter::IReplyTransactionsReport(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	uint8_t reqType = p->GetInt8();
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_TRANSACTIONS_REPORT);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt8(reqType);
	if (errCode)
		return o;
	if (0 == reqType)	//	Transactions Report Year List
	{
		uint32_t nArr = p->GetInt32();
		o->AddInt32(nArr);
		for (uint32_t i = 0; i < nArr; i++)
			o->AddInt32(p->GetInt32());	//	Transaction Report Year List
		return o;
	}
	uint32_t reportPeriod = p->GetInt32(),
					 nArr = p->GetInt32();
	o->AddInt32(reportPeriod);
	o->AddInt32(nArr);
	char szBoard[IBOARD_LEN], szStockCode[ISTOCKCODE_LEN];
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt32(p->GetInt32());	//	Date
		o->AddInt8(p->GetInt8());	//	Command
		o->AddInt16(slen = p->GetInt16());	//	slen Board
		p->GetString(slen, szBoard);
		o->AddString(szBoard, slen);		//	Board
		o->AddInt16(slen = p->GetInt16());	//	slen StockCode
		p->GetString(slen, szStockCode);
		o->AddString(szStockCode, slen);	//	StockCode
		o->AddInt32(p->GetInt32());	//	Price
		o->AddInt64(p->GetInt64());	//	Qty
		o->AddInt64(p->GetInt64());	//	Gross Amount
		o->AddInt64(p->GetInt64());	//	Broker Fee
		o->AddInt64(p->GetInt64());	//	Levy
		o->AddInt64(p->GetInt64());	//	WHT
		o->AddInt64(p->GetInt64());	//	VAT
		o->AddInt64(p->GetInt64());	//	Sales Tax
		o->AddInt64(p->GetInt64());	//	Subtotal Fees
		o->AddInt64(p->GetInt64());	//	Nett Amount
	}
	o->AddInt64(p->GetInt64());	//	Total Gross Amount
	o->AddInt64(p->GetInt64());	//	Total Broker Fee
	o->AddInt64(p->GetInt64());	//	Total Levy
	o->AddInt64(p->GetInt64());	//	Total WHT
	o->AddInt64(p->GetInt64());	//	Total VAT
	o->AddInt64(p->GetInt64());	//	Total Sales Tax
	o->AddInt64(p->GetInt64());	//	Total Fees
	o->AddInt64(p->GetInt64());	//	Total Nett Amount
	return o;
}

CGPackage* CClientConverter::IReplyMonthlyBalance(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	uint8_t reqType = p->GetInt8();
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_MONTHLY_POSITION);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt8(reqType);
	if (errCode)
		return o;
	if (0 == reqType)	//	Monthly Balance Year List
	{
		uint32_t nArr = p->GetInt32();	//	Array
		o->AddInt32(nArr);	//	Array
		for (uint32_t i = 0; i < nArr; i++)
			o->AddInt32(p->GetInt32());
		return o;
	}
	uint32_t reportYear = p->GetInt32(), 
		nArr = p->GetInt32();	//	Array
	o->AddInt32(reportYear);
	o->AddInt32(nArr);
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt16(slen = p->GetInt16());	//	StockCode Len
		p->GetString(slen, szStockCode);
		o->AddString(szStockCode, slen);
		o->AddInt32(p->GetInt32());	//	Avg Price
		o->AddInt32(p->GetInt32());	//	Close Price
		o->AddInt64(p->GetInt64());	//	Balance Qty
		o->AddInt64(p->GetInt64());	//	Market Value
		o->AddInt64(p->GetInt64());	//	Potential Gain / Loss
		o->AddInt32(p->GetInt32());	//	Potential Gain / Loss %
	}
	o->AddInt64(p->GetInt64());	//	Total Market Value
	o->AddInt64(p->GetInt64());	//	Total Potential Gain / Loss
	o->AddInt32(p->GetInt32());	//	Total Potential Gain / Loss %
	o->AddInt64(p->GetInt64());	//	Total Cash
	o->AddInt64(p->GetInt64());	//	Total Assets
	return o;
}

CGPackage* CClientConverter::IReplySingleStockBalance(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_SINGLE_STOCK_BALANCE);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt8(p->GetInt8());
	slen = p->GetInt16(); p->GetString(slen, szStockCode);
	o->AddString(szStockCode);
	if (errCode)
		return o;
	o->AddInt64(p->GetInt64());	//	Used in Shares
	o->AddInt64(p->GetInt64());	//	Balance in Shares
	return o;
}

CGPackage* CClientConverter::IReplyValidateLoginPassword(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_VALIDATE_LOGIN_PASSWORD);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	if (errCode)
		return o;
	o->AddInt8(p->GetInt8());	//	Login Type
	o->AddInt32(p->GetInt32());	//	Login Permissions
	return o;
}

CGPackage* CClientConverter::IReplySpeedOrderBook(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szStockCode[ISTOCKCODE_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength(), IPKG_REQ_SPEED_ORDER_BOOK);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt8(p->GetInt8());	//	iBoard
	slen = p->GetInt16(); p->GetString(slen, szStockCode);
	o->AddString(szStockCode);
	if (errCode)
		return o;
	o->AddInt64(p->GetInt64());	//	Cash T+2
	o->AddInt64(p->GetInt64());	//	Available TL
	o->AddInt64(p->GetInt64());	//	Stock Balance
	uint32_t nArr = p->GetInt32();	//	Array
	o->AddInt32(nArr);
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt32(p->GetInt32());	//	Price
		o->AddInt64(p->GetInt64());	//	Bid Open Balance
		o->AddInt64(p->GetInt64());	//	Bid Traded
		o->AddInt64(p->GetInt64());	//	Offer Open Balance
		o->AddInt64(p->GetInt64());	//	Offer Traded
	}
	return o;
}

// Order Reply from IServer

CGPackage* CClientConverter::IReplyOrder(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	CGPackage *o = new CGPackage(p->GetPackageLength(), p->GetPackageId());
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	return o;
}

CGPackage* CClientConverter::IReplyOrderList(CIPackage *p)
{
	uint32_t orderStatus = 0;
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	uint16_t packageId = p->GetPackageId(), errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	uint32_t nArr = p->GetInt32();
	CGPackage *o = new CGPackage(p->GetPackageLength(), packageId);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt32(nArr);
	char szOrderId[ISTR32_LEN] = {}, szAmendId[ISTR32_LEN] = {}, szJATSId[ISTR32_LEN] = {}, szExchg[ISTR32_LEN] = {}, szBoard[IBOARD_LEN] = {}, szStockCode[ISTOCKCODE_LEN] = {}, szInputUser[ISTR64_LEN] = {}, szCounterPartyId[ISTR32_LEN] = {}, szComplianceId[ISTR32_LEN] = {}, szClientId[ISTR32_LEN] = {}, szRejectedNote[ISTR256_LEN] = {};
	for (uint32_t i = 0; i < nArr; i++)
	{
		o->AddInt8(orderStatus = p->GetInt8());	//	Order Status
		slen = p->GetInt16(); p->GetString(slen, szOrderId);
		slen = p->GetInt16(); p->GetString(slen, szAmendId);
		slen = p->GetInt16(); p->GetString(slen, szJATSId);
		o->AddString(szOrderId);
		o->AddString(szAmendId);
		o->AddString(szJATSId);
		o->AddInt32(p->GetInt32());	//	Order Date
		o->AddInt32(p->GetInt32());	//	Order Time
		o->AddInt32(p->GetInt32());	//	Sent Time
		slen = p->GetInt16(); p->GetString(slen, szExchg);
		slen = p->GetInt16(); p->GetString(slen, szBoard);
		o->AddString(szExchg);
		o->AddString(szBoard);
		o->AddInt8(p->GetInt8());	//	Expiry
		o->AddInt8(p->GetInt8());	//	Command
		slen = p->GetInt16(); p->GetString(slen, szStockCode);
		o->AddString(szStockCode);
		o->AddInt32(p->GetInt32());	//	Order Price
		o->AddInt64(p->GetInt64());	//	OVolume
		o->AddInt64(p->GetInt64());	//	RVolume
		o->AddInt64(p->GetInt64());	//	TVolume
		slen = p->GetInt16(); p->GetString(slen, szInputUser);
		slen = p->GetInt16(); p->GetString(slen, szCounterPartyId);
		o->AddString(szInputUser);
		o->AddString(szCounterPartyId);
		o->AddInt8(p->GetInt8());	//	SourceId
		slen = p->GetInt16(); p->GetString(slen, szComplianceId);
		slen = p->GetInt16(); p->GetString(slen, szClientId);
		o->AddString(szComplianceId);
		o->AddString(szClientId);
		if (ORDER_REJECTED == orderStatus)
			if (IPKG_REQ_HISTORICAL_ORDER_LIST == packageId)
			{
				slen = p->GetInt16(); p->GetString(slen, szRejectedNote);
				o->AddString(szRejectedNote);
			}
	}
	return o;
}

CGPackage* CClientConverter::IReplyTradeList(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	uint16_t packageId = p->GetPackageId(), errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	uint32_t nArr = p->GetInt32();
	CGPackage *o = new CGPackage(p->GetPackageLength(), packageId);
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt32(nArr);
	char szOrderId[ISTR32_LEN] = {}, szTradeId[ISTR32_LEN] = {}, szJATSId[ISTR32_LEN] = {}, szExchg[ISTR32_LEN] = {}, szBoard[IBOARD_LEN] = {}, szStockCode[ISTOCKCODE_LEN] = {}, szCounterPartyId[ISTR32_LEN] = {};
	for (uint32_t i = 0; i < nArr; i++)
	{
		slen = p->GetInt16(); p->GetString(slen, szOrderId);
		slen = p->GetInt16(); p->GetString(slen, szJATSId);
		slen = p->GetInt16(); p->GetString(slen, szTradeId);
		o->AddString(szOrderId);
		o->AddString(szJATSId);
		o->AddString(szTradeId);
		o->AddInt32(p->GetInt32());	//	Trade Date
		o->AddInt32(p->GetInt32());	//	Trade Time
		slen = p->GetInt16(); p->GetString(slen, szExchg);
		slen = p->GetInt16(); p->GetString(slen, szBoard);
		o->AddString(szExchg);
		o->AddString(szBoard);
		o->AddInt8(p->GetInt8());	//	Expiry
		o->AddInt8(p->GetInt8());	//	Command
		slen = p->GetInt16(); p->GetString(slen, szStockCode);
		o->AddString(szStockCode);
		o->AddInt32(p->GetInt32());	//	Trade Price
		o->AddInt64(p->GetInt64());	//	Trade Volume
		slen = p->GetInt16(); p->GetString(slen, szCounterPartyId);
		o->AddString(szCounterPartyId);
		o->AddInt8(p->GetInt8());	//	SourceId
	}
	return o;
}

CGPackage* CClientConverter::IReplyRejectedOrderMessage(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN], szOrderId[ISTR32_LEN] = {}, szRejectedMsg[ISTR256_LEN] = {};
	uint16_t errCode = p->GetErrorCode(),
	slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	slen = p->GetInt16(); p->GetString(slen, szOrderId);
	CGPackage *o = new CGPackage(p->GetPackageLength(), p->GetPackageId());
	o->SetReference(szRef);
	o->SetErrorCode(errCode);
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddString(szOrderId);
	if (errCode)
		return o;
	slen = p->GetInt16(); p->GetString(slen, szRejectedMsg);
	o->AddString(szRejectedMsg);
	return o;
}

CGPackage* CClientConverter::IReplyGTCOrderList(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	uint32_t nArr = p->GetInt32();
	CGPackage *o = new CGPackage(p->GetPackageLength(), p->GetPackageId());
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt32(nArr);
	char szOrderId[ISTR32_LEN] = {}, szExchg[ISTR32_LEN] = {}, szBoard[IBOARD_LEN] = {}, szStockCode[ISTOCKCODE_LEN] = {}, szInputUser[ISTR64_LEN] = {}, szCancelUser[ISTR64_LEN] = {}, szComplianceId[ISTR32_LEN] = {};
	for (uint32_t i = 0; i < nArr; i++)
	{
		slen = p->GetInt16(); p->GetString(slen, szOrderId);
		o->AddString(szOrderId);		//	GTC Order Id
		o->AddInt32(p->GetInt32());	//	Subscribe Date
		o->AddInt32(p->GetInt32());	//	Subscribe Time
		o->AddInt32(p->GetInt32());	//	Effective Date
		o->AddInt32(p->GetInt32());	//	Due Date
		o->AddInt8(p->GetInt8());	//	SessionId
		o->AddInt8(p->GetInt8());	//	GTC Flags
		slen = p->GetInt16(); p->GetString(slen, szExchg);
		slen = p->GetInt16(); p->GetString(slen, szBoard);
		o->AddString(szExchg);
		o->AddString(szBoard);
		o->AddInt8(p->GetInt8());	//	Expiry
		o->AddInt8(p->GetInt8());	//	Command
		slen = p->GetInt16(); p->GetString(slen, szStockCode);
		o->AddString(szStockCode);
		o->AddInt32(p->GetInt32());	//	Order Price
		o->AddInt64(p->GetInt64());	//	OVolume
		o->AddInt64(p->GetInt64());	//	RVolume
		o->AddInt64(p->GetInt64());	//	TVolume
		o->AddInt32(p->GetInt32());	//	Automatic Order Price Step
		o->AddInt8(p->GetInt8());	//	Subscribe Status
		o->AddInt8(p->GetInt8());	//	Source Id
		o->AddInt8(p->GetInt8());	//	Cancel Source Id
		slen = p->GetInt16(); p->GetString(slen, szInputUser);
		slen = p->GetInt16(); p->GetString(slen, szCancelUser);
		slen = p->GetInt16(); p->GetString(slen, szComplianceId);
		o->AddString(szInputUser);
		o->AddString(szCancelUser);
		o->AddString(szComplianceId);
	}
	return o;
}

CGPackage* CClientConverter::IReplyBreakOrderList(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	uint32_t nArr = p->GetInt32();
	CGPackage *o = new CGPackage(p->GetPackageLength(), p->GetPackageId());
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt32(nArr);
	char szOrderId[ISTR32_LEN] = {}, szBoard[IBOARD_LEN] = {}, szStockCode[ISTOCKCODE_LEN] = {}, szInputUser[ISTR64_LEN] = {}, szCancelUser[ISTR64_LEN] = {}, szDescription[ISTR256_LEN] = {}, szResultNote[ISTR256_LEN] = {}, szClientIP[ISTR128_LEN] = {};
	for (uint32_t i = 0; i < nArr; i++)
	{
		slen = p->GetInt16(); p->GetString(slen, szOrderId);	//	Break Order Id
		o->AddString(szOrderId);								//	Break Order Id
		o->AddInt8(p->GetInt8());	//	Command
		slen = p->GetInt16(); p->GetString(slen, szBoard);
		slen = p->GetInt16(); p->GetString(slen, szStockCode);
		o->AddString(szBoard);
		o->AddString(szStockCode);
		o->AddInt32(p->GetInt32());	//	Order Price
		o->AddInt64(p->GetInt64());	//	Volume
		o->AddInt32(p->GetInt32());	//	Automatic Order Price Step
		o->AddInt8(p->GetInt8());	//	Price Type
		o->AddInt8(p->GetInt8());	//	Price Criteria
		o->AddInt32(p->GetInt32());	//	Target Price
		o->AddInt8(p->GetInt8());	//	Volume Type
		o->AddInt8(p->GetInt8());	//	Volume Criteria
		o->AddInt64(p->GetInt64());	//	Target Volume
		o->AddInt8(p->GetInt8());	//	Order Status
		o->AddInt32(p->GetInt32());	//	Order Date
		o->AddInt32(p->GetInt32());	//	Order Time
		o->AddInt32(p->GetInt32());	//	Sent Date
		o->AddInt32(p->GetInt32());	//	Sent Time
		o->AddInt32(p->GetInt32());	//	Effective Date
		o->AddInt32(p->GetInt32());	//	Due Date
		o->AddInt8(p->GetInt8());	//	Source Id
		o->AddInt8(p->GetInt8());	//	Withdraw Source Id
		slen = p->GetInt16(); p->GetString(slen, szInputUser);
		slen = p->GetInt16(); p->GetString(slen, szCancelUser);
		slen = p->GetInt16(); p->GetString(slen, szDescription);
		slen = p->GetInt16(); p->GetString(slen, szClientIP);
		slen = p->GetInt16(); p->GetString(slen, szResultNote);
		o->AddString(szInputUser);
		o->AddString(szCancelUser);
		o->AddString(szDescription);
		o->AddString(szClientIP);
		o->AddString(szResultNote);
	}
	return o;
}

CGPackage* CClientConverter::IReplyTrailingOrderList(CIPackage *p)
{
	char szLoginId[ILOGINID_LEN], szRef[IREF_LEN], szAccId[IACCID_LEN];
	uint16_t slen = p->GetInt16(); p->GetString(slen, szLoginId);
	slen = p->GetInt16(); p->GetString(slen, szRef);
	slen = p->GetInt16(); p->GetString(slen, szAccId);
	uint32_t nArr = p->GetInt32();
	CGPackage *o = new CGPackage(p->GetPackageLength(), p->GetPackageId());
	o->SetReference(szRef);
	o->SetErrorCode(p->GetErrorCode());
	o->AddString(szLoginId);
	o->AddString(szRef);
	o->AddString(szAccId);
	o->AddInt32(nArr);
	char szOrderId[ISTR32_LEN] = {}, szBoard[IBOARD_LEN] = {}, szStockCode[ISTOCKCODE_LEN] = {}, szInputUser[ISTR64_LEN] = {}, szCancelUser[ISTR64_LEN] = {}, szDescription[ISTR256_LEN] = {}, szResultNote[ISTR256_LEN] = {}, szClientIP[ISTR128_LEN] = {};
	for (uint32_t i = 0; i < nArr; i++)
	{
		slen = p->GetInt16(); p->GetString(slen, szOrderId);	//	Trailing Order Id
		o->AddString(szOrderId);								//	Trailing Order Id
		o->AddInt8(p->GetInt8());	//	Command
		slen = p->GetInt16(); p->GetString(slen, szBoard);
		slen = p->GetInt16(); p->GetString(slen, szStockCode);
		o->AddString(szBoard);
		o->AddString(szStockCode);
		o->AddInt8(p->GetInt8());	//	Exec Price
		o->AddInt64(p->GetInt64());	//	Volume
		o->AddInt32(p->GetInt32());	//	Automatic Order Price Step
		o->AddInt32(p->GetInt32());	//	Drop Price
		o->AddInt8(p->GetInt8());	//	Trailing Price Type
		o->AddInt32(p->GetInt32());	//	Trailing Step
		o->AddInt32(p->GetInt32());	//	Stop Price
		o->AddInt32(p->GetInt32());	//	Trailing Price
		o->AddInt32(p->GetInt32());	//	Executed Price
		o->AddInt8(p->GetInt8());	//	Order Status
		o->AddInt32(p->GetInt32());	//	Order Date
		o->AddInt32(p->GetInt32());	//	Order Time
		o->AddInt32(p->GetInt32());	//	Sent Date
		o->AddInt32(p->GetInt32());	//	Sent Time
		o->AddInt32(p->GetInt32());	//	Effective Date
		o->AddInt32(p->GetInt32());	//	Due Date
		o->AddInt8(p->GetInt8());	//	Source Id
		o->AddInt8(p->GetInt8());	//	Withdraw Source Id
		slen = p->GetInt16(); p->GetString(slen, szInputUser);
		slen = p->GetInt16(); p->GetString(slen, szCancelUser);
		slen = p->GetInt16(); p->GetString(slen, szDescription);
		slen = p->GetInt16(); p->GetString(slen, szClientIP);
		slen = p->GetInt16(); p->GetString(slen, szResultNote);
		o->AddString(szInputUser);
		o->AddString(szCancelUser);
		o->AddString(szDescription);
		o->AddString(szClientIP);
		o->AddString(szResultNote);
	}
	return o;
}

//  Ouch Package Composer

CTEOrderPackage* CClientConverter::ComposeOuchSystemEventMsg(OUCH_SYSTEM_EVENT_MSG &o)
{
	CTEOrderPackage *pkg = new CTEOrderPackage(10, PK_RT_OUTCH_OUTBOUND_SEQ_MSG);
	if (NULL == pkg)
		return NULL;
	pkg->AddByte(o.type);
	pkg->AddInt64(o.timeStamp);
	pkg->AddByte(o.eventCode);
	return pkg;
}

CTEOrderPackage* CClientConverter::ComposeOuchTradingLimitMsg(OUCH_TRADING_LIMIT_MSG &o)
{
	CTEOrderPackage *pkg = new CTEOrderPackage(34, PK_RT_OUTCH_OUTBOUND_SEQ_MSG);
	if (NULL == pkg)
		return NULL;
	pkg->AddByte(o.type);
	pkg->AddInt64(o.timeStamp);
	pkg->AddByte(o.tradingLimitType);
	pkg->AddInt64(o.used);
	pkg->AddInt64(o.traded);
	pkg->AddInt64(o.limit);
	return pkg;
}

CTEOrderPackage* CClientConverter::ComposeOuchAcceptedOrderMsg(OUCH_ACCEPTED_ORDER_MSG &o)
{
	CTEOrderPackage *pkg = new CTEOrderPackage(110, PK_RT_OUTCH_OUTBOUND_SEQ_MSG);
	if (NULL == pkg)
		return NULL;
	pkg->AddByte(o.type);
	pkg->AddInt64(o.timeStamp);
	pkg->AddDWord(o.orderToken);
	pkg->AddString(o.szBrokerRef, 20);
	pkg->AddString(o.szInvestorId, 6);
	pkg->AddByte(o.orderVerb);
	pkg->AddString(o.szOrderSource, 4);
	pkg->AddByte(o.domicile);
	pkg->AddInt64(o.quantity);
	pkg->AddDWord(o.orderbookId);
	pkg->AddDWord(o.price);
	pkg->AddDWord(o.timeInForce);
	pkg->AddDWord(o.clientId);
	pkg->AddInt64(o.orderNumber);
	pkg->AddInt64(o.extOrderNumber);
	pkg->AddByte(o.orderState);
	pkg->AddInt64(o.minQuantity);
	pkg->AddString(o.szUserId, 15);
	return pkg;
}

CTEOrderPackage* CClientConverter::ComposeOuchReplacedOrderMsg(OUCH_REPLACED_ORDER_MSG &o)
{
	CTEOrderPackage *pkg = new CTEOrderPackage(70, PK_RT_OUTCH_OUTBOUND_SEQ_MSG);
	if (NULL == pkg)
		return NULL;
	pkg->AddByte(o.type);
	pkg->AddInt64(o.timeStamp);
	pkg->AddDWord(o.replacementOrderToken);
	pkg->AddByte(o.orderVerb);
	pkg->AddInt64(o.quantity);
	pkg->AddDWord(o.orderbookId);
	pkg->AddDWord(o.price);
	pkg->AddDWord(o.timeInForce);
	pkg->AddInt64(o.orderNumber);
	pkg->AddInt64(o.extOrderNumber);
	pkg->AddByte(o.orderState);
	pkg->AddDWord(o.prevOrderToken);
	pkg->AddString(o.szUserId, 15);
	return pkg;
}

CTEOrderPackage* CClientConverter::ComposeOuchCanceledOrderMsg(OUCH_CANCELED_ORDER_MSG &o)
{
	CTEOrderPackage *pkg = new CTEOrderPackage(37, PK_RT_OUTCH_OUTBOUND_SEQ_MSG);
	if (NULL == pkg)
		return NULL;
	pkg->AddByte(o.type);
	pkg->AddInt64(o.timeStamp);
	pkg->AddDWord(o.orderToken);
	pkg->AddInt64(o.quantity);
	pkg->AddByte(o.reason);
	pkg->AddString(o.szUserId, 15);
	return pkg;
}

CTEOrderPackage* CClientConverter::ComposeOuchExecutedOrderMsg(OUCH_EXECUTED_ORDER_MSG &o)
{
	CTEOrderPackage *pkg = new CTEOrderPackage(64, PK_RT_OUTCH_OUTBOUND_SEQ_MSG);
	if (NULL == pkg)
		return NULL;
	pkg->AddByte(o.type);
	pkg->AddInt64(o.timeStamp);
	pkg->AddDWord(o.orderToken);
	pkg->AddInt64(o.executedQty);
	pkg->AddDWord(o.executedPrice);
	pkg->AddByte(o.liquidityFlag);
	pkg->AddInt64(o.matchNumber);
	pkg->AddDWord(o.counterPartyId);
	pkg->AddString(o.szUserId, 15);
	return pkg;
}

CTEOrderPackage* CClientConverter::ComposeOuchRejectedOrderMsg(OUCH_REJECTED_ORDER_MSG &o)
{
	CTEOrderPackage *pkg = new CTEOrderPackage(sizeof(OUCH_REJECTED_ORDER_MSG) + 16, PK_RT_OUTCH_OUTBOUND_SEQ_MSG);
	if (NULL == pkg)
		return NULL;
	pkg->AddByte(o.type);
	pkg->AddInt64(o.timeStamp);
	pkg->AddDWord(o.orderToken);
	pkg->AddByte(o.reason);
	pkg->AddString(o.szUserId, 15);
	pkg->AddString(o.szOrderId, 20);
	return pkg;
}

CTEOrderPackage* CClientConverter::ComposeOuchBrokenTradeMsg(OUCH_BROKEN_TRADE_MSG &o)
{
	CTEOrderPackage *pkg = new CTEOrderPackage(sizeof(OUCH_BROKEN_TRADE_MSG) + 16, PK_RT_OUTCH_OUTBOUND_SEQ_MSG);
	if (NULL == pkg)
		return NULL;
	pkg->AddByte(o.type);
	pkg->AddInt64(o.timeStamp);
	pkg->AddDWord(o.orderToken);
	pkg->AddInt64(o.matchNumber);
	pkg->AddByte(o.reason);
	pkg->AddString(o.szUserId, 15);
	pkg->AddString(o.szOrderId, 20);
	return pkg;
}

CTEOrderPackage* CClientConverter::ComposeOuchRestatementMsg(OUCH_RESTATEMENT_MSG &o)
{
	CTEOrderPackage *pkg = new CTEOrderPackage(sizeof(OUCH_RESTATEMENT_MSG) + 16, PK_RT_OUTCH_OUTBOUND_SEQ_MSG);
	if (NULL == pkg)
		return NULL;
	pkg->AddByte(o.type);
	pkg->AddInt64(o.timeStamp);
	pkg->AddDWord(o.orderToken);
	pkg->AddDWord(o.price);
	pkg->AddByte(o.reason);
	pkg->AddString(o.szUserId, 15);
	pkg->AddString(o.szOrderId, 20);
	return pkg;
}
