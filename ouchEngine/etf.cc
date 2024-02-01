#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "header/journal.h"
#include "header/miscfunc.h"
#include "header/idxdtfeed.h"
#include "header/find_itch.h"
#include "header/PriceFraction.h"
#include "header/etf.h"
#include "header/mutexGuard.h"
#include "header/RawPackage.h"
#include "header/GQueue.h"

CETFForecast::CETFForecast() : unit(0), volume(0), sellValue(0), buyValue(0), curValue(0), curNav(0), buyNav(0), sellNav(0)
{
}

CETFForecast::CETFForecast(OL_UINT32 unitSize, OL_UINT64 vol) : unit(unitSize), volume(vol), sellValue(0), buyValue(0), curValue(0), curNav(0), buyNav(0), sellNav(0)
{
}

CETFForecast &CETFForecast::operator=(CETFForecast &f)
{
	unit = f.unit;
	volume = f.volume;
	curValue = f.curValue;
	buyValue = f.buyValue;
	sellValue = f.sellValue;
	curNav = f.curNav;
	buyNav = f.buyNav;
	sellNav = f.sellNav;
	return *this;
}

void CETFForecast::ClearValues()
{
	curNav = buyNav = sellNav = 0;
	sellValue = buyValue = curValue = 0;
}

void CETFForecast::ClearBuyValues()
{
	buyNav = 0;
	buyValue = 0;
}

void CETFForecast::ClearSellValues()
{
	sellNav = 0;
	sellValue = 0;
}

CETF::CETF(ITCH_SHM *p, const char *dealerid, const char *etf, const char *desc, const char *bufAccount, const char *tradeAccount, OL_UINT32 uSize,
		   OL_INT64 lCash, OL_UINT32 mCU, double spreadp, double up, double disc, uint8_t fl) : maxCU(mCU), unitSize(uSize), unitShares(0), cash(lCash), spread(spreadp), upPrice(up), discPrice(disc), flags(fl), OBDEPTH(5), pidxdata(p)
{
	strcpy(szETF, etf);
	strcpy(szDescription, desc);
	strcpy(szDealerId, dealerid);
	strcpy(szBufAccount, bufAccount);
	strcpy(szTradeAccount, tradeAccount);
	pthread_mutex_init(&utex, NULL);
	pthread_mutex_init(&ftex, NULL);
}

CETF::~CETF(void)
{
	Release();
	pthread_mutex_destroy(&utex);
	pthread_mutex_destroy(&ftex);
}

void CETF::Release()
{
	pthread_mutex_lock(&utex);
	for (OL_UINT32 i = 0; i < underlying.size(); i++)
		delete underlying[i];
	underlying.clear();
	pthread_mutex_unlock(&utex);
}

void CETF::ReleaseForecast()
{
	pthread_mutex_lock(&ftex);
	for (OL_UINT32 i = 0; i < forecast.size(); i++)
		delete forecast[i];
	forecast.clear();
	pthread_mutex_unlock(&ftex);
}

void CETF::ContinueProcessETFInit(CTEOrderPackage *pkg)
{
	Release();
	char szStockCode[32] = {};
	OL_UINT32 n = pkg->GetDWord();
	pthread_mutex_lock(&utex);
	underlying.reserve(n);
	for (OL_UINT32 i = 0; i < n; ++i)
	{
		OL_UINT16 slen = pkg->GetWord();
		pkg->GetString(slen, szStockCode);
		underlying.push_back(new ETFItem(szStockCode, pkg->GetDWord()));
	}
	RecalculatePercentage();
	pthread_mutex_unlock(&utex);
}

void CETF::RecalculatePercentage()
{
	unitShares = 0;
	for (OL_UINT32 i = 0; i < underlying.size(); i++)
		unitShares += underlying[i]->volume;
	for (OL_UINT32 i = 0; i < underlying.size(); i++)
		underlying[i]->percentage = (unitShares) ? (double)underlying[i]->volume / unitShares : 0;
}

void CETF::InitForecast()
{
	int i = 1;
	ReleaseForecast();
	pthread_mutex_lock(&ftex);
	forecast.reserve(25);
	for (i = 1; i <= 5; ++i)
		forecast.push_back(new CETFForecast(i, unitSize * i));
	for (i = 1; i <= 5; ++i)
		forecast.push_back(new CETFForecast(i * 10, unitSize * i * 10));
	for (i = 1; i <= 5; ++i)
		forecast.push_back(new CETFForecast(i * 100, unitSize * i * 100));
	for (i = 1; i <= 5; ++i)
		forecast.push_back(new CETFForecast(i * 1000, unitSize * i * 1000));
	for (i = 1; i <= 5; ++i)
		forecast.push_back(new CETFForecast(i * 10000, unitSize * i * 10000));
	pthread_mutex_unlock(&ftex);
}

void CETF::RecalculateForecast()
{
	pthread_mutex_lock(&ftex);
	for (OL_UINT32 i = 0; i < forecast.size(); i++)
		CalculateForecast(forecast[i]);
	pthread_mutex_unlock(&ftex);
}

void CETF::GetETFForecast(OL_UINT32 idx, CETFForecast *p)
{
	pthread_mutex_lock(&ftex);
	*p = *forecast[idx];
	pthread_mutex_unlock(&ftex);
}

void CETF::RescanUnderlyingStatus()
{
	// OL_INT32 ib = 0, is = 0;
	// IDX_STOCK *ps = pidxdata->idxStock;
	// OL_UINT32 lTotStk = pidxdata->idxState.lTotalStock;
	// umemo.clear();
	// pthread_mutex_lock(&utex);
	// for (OL_UINT32 i = 0; i < underlying.size(); i++)
	//{
	// }
}

bool CETF::CalculateForecast(CETFForecast *p)
{
	// if ((55 == pidxdata->idxState.fStatus) || ('d' == pidxdata->idxState.fStatus))	//	End Sending Records
	//	return false;
	IDX_STOCK *ps = pidxdata->idxStock;
	OL_UINT32 lTotStk = pidxdata->idxState.lTotalStock;
	OL_INT32 ib = 0, is = 0;
	p->ClearValues();
	pthread_mutex_lock(&utex);
	for (OL_UINT32 i = 0; i < underlying.size(); i++)
	{
		if (-1 == (is = find_idx_stock(underlying[i]->szStockCode, ps, lTotStk, SZ_IDX_STOCK)))
			continue;
		IDX_STOCK_DATA *psd = &ps[is].stockData[ib];
		OL_UINT32 price = ((0 == psd->lLastPrice) ? psd->lPrevPrice : psd->lLastPrice) / 100;
		if (0 == price)
			continue;
		OL_UINT64 cVol = p->unit * underlying[i]->volume,
				  bVol = cVol, sVol = cVol;
		p->curValue += price * cVol;
		price = ((0 == psd->lBestOfferPrice) ? psd->lLastPrice : psd->lBestOfferPrice) / 100;
		if (bVol <= psd->lBestOfferVolume)
			p->buyValue += price * bVol;
		else
		{
			OL_UINT64 volume = 0;
			IDX_ORDER_BOOK *pob = &ps[is].orderBook[ib];
			for (OL_UINT16 od = 0; od < pob->lOfferDepth; od++)
			{
				price = pob->offer[od].lPrice / 100;
				volume = pob->offer[od].lVolume;
				if ((price > 0) && (volume > 0) && (od < OBDEPTH))
				{
					if (bVol > volume)
					{
						p->buyValue += price * volume;
						bVol -= volume;
					}
					else
					{
						p->buyValue += price * bVol;
						bVol = 0;
						break;
					}
				}
				else
					break;
			}
			if (bVol > 0)
				p->buyValue += price * bVol;
		}
		price = ((0 == psd->lBestBidPrice) ? psd->lLastPrice : psd->lBestBidPrice) / 100;
		if (sVol <= psd->lBestBidVolume)
			p->sellValue += price * sVol;
		else
		{
			OL_UINT64 volume = 0;
			IDX_ORDER_BOOK *pob = &ps[is].orderBook[ib];
			for (OL_UINT16 od = 0; od < pob->lBidDepth; od++)
			{
				price = pob->bid[od].lPrice / 100;
				volume = pob->bid[od].lVolume;
				if ((price > 0) && (volume > 0) && (od < OBDEPTH))
				{
					if (sVol > volume)
					{
						p->sellValue += price * volume;
						sVol -= volume;
					}
					else
					{
						p->sellValue += price * sVol;
						sVol = 0;
						break;
					}
				}
				else
					break;
			}
			if (sVol > 0)
				p->sellValue += price * sVol;
		}
	}
	p->curNav = ((p->curValue / p->unit) + cash) / (double)unitSize;
	p->buyNav = ((p->buyValue / p->unit) + cash) / (double)unitSize;
	p->sellNav = ((p->sellValue / p->unit) + cash) / (double)unitSize;
	pthread_mutex_unlock(&utex);
	return true;
}

CTEOrderPackage *CETF::ComposeForecastPackage()
{
	CRawPackage raw(PK_ZMQ_ETF_FORECAST_DATA);
	raw.AddString(szETF);
	raw.AddInt32(unitSize);
	raw.AddInt64(cash);
	pthread_mutex_lock(&ftex);
	raw.AddInt32(forecast.size());
	for (OL_UINT32 i = 0; i < forecast.size(); ++i)
	{
		raw.AddInt32(forecast[i]->unit);
		raw.AddInt64(forecast[i]->volume);
		raw.AddInt64(forecast[i]->curValue);
		raw.AddInt64(forecast[i]->buyValue);
		raw.AddInt64(forecast[i]->sellValue);
	}
	pthread_mutex_unlock(&ftex);
	return new CTEOrderPackage(&raw);
}

CETFProcess::CETFProcess(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, CPriceFraction *pfrac, _QUEUE_ *pqueue) : journal(pjournal), received(preceived), executed(pexecuted), pf(pfrac), outq(pqueue)
{
	bHoliday = isHoliday(lCurDate);
	pthread_mutex_init(&mtex, NULL);
	pthread_mutex_init(&stex, NULL);
	if (-1 == shm.Create(SHM_KEY_ITCH, SZ_ITCH_SHM))
	{
		journal->WriteLog("[CETFProcess::CETFProcess] - Failed to create Shared Memory.\n");
		return;
	}
	if (NULL == (pidxdata = shm.AttachMem()))
	{
		journal->WriteLog("[CETFProcess::CETFProcess] - Failed to attach Shared Memory.\n");
		return;
	}
	bRunning = true;
	if (pthread_create(&fid, NULL, (void *(*)(void *))forecast_worker, (void *)this))
		journal->WriteLog("[CETFProcess::CETFProcess] - Can not create Server Forecast Thread ...\n");
	ReqETFInit();
	RequestResendAllETFBatchOrders();
}

CETFProcess::~CETFProcess()
{
	if (bRunning)
	{
		bRunning = false;
		pthread_join(cid, NULL);
		pthread_join(fid, NULL);
	}
	shm.DetachMem();
	Release();
	ReleaseETFBatchOrders();
	pthread_mutex_destroy(&mtex);
	pthread_mutex_destroy(&stex);
}

void CETFProcess::Release()
{
	pthread_mutex_lock(&mtex);
	for (_ETFMAP::const_iterator i = emap.begin(); i != emap.end(); ++i)
		if (i->second)
			delete i->second;
	emap.clear();
	pthread_mutex_unlock(&mtex);
}

void CETFProcess::ReleaseETFBatchOrders()
{
	pthread_mutex_lock(&stex);
	for (__SMAP__::const_iterator ci = smap.begin(); ci != smap.end(); ++ci)
		if (ci->second)
			delete ci->second;
	smap.clear();
	pthread_mutex_unlock(&stex);
}

void CETFProcess::SendHeartBeat()
{
	outq->AddtoQueue(new CTEOrderPackage(0, PK_HTS_HEART_BEAT));
}

void CETFProcess::ReqETFInit()
{
	outq->AddtoQueue(new CTEOrderPackage(0, PK_ZMQ_ETF_INIT));
}

void CETFProcess::RequestResendAllETFBatchOrders()
{
	CTEOrderPackage *p = new CTEOrderPackage(1, PK_ZMQ_REQ_ALL_OPEN_ETF_ORDER);
	p->AddByte(0);
	outq->AddtoQueue(p);
}

void CETFProcess::ProcessETFInit(CTEOrderPackage *pkg)
{
	Release();
	char szETF[32], szDealerId[16], szDescription[128], szBufAccount[16], szTradeAccount[16];
	pthread_mutex_lock(&mtex);
	OL_UINT32 nTot = pkg->GetDWord();
	for (OL_UINT32 i = 0; i < nTot; ++i)
	{
		OL_UINT16 slen = pkg->GetWord();
		pkg->GetString(slen, szETF);
		slen = pkg->GetWord();
		pkg->GetString(slen, szDescription);
		OL_UINT32 unitSize = pkg->GetDWord(),
				  maxCU = pkg->GetDWord();
		OL_INT64 cash = pkg->GetInt64();
		double spread = pkg->GetDouble(),
			   upPrice = pkg->GetDouble(),
			   discPrice = pkg->GetDouble();
		uint8_t flags = pkg->GetByte();
		slen = pkg->GetWord();
		pkg->GetString(slen, szBufAccount);
		slen = pkg->GetWord();
		pkg->GetString(slen, szTradeAccount);
		slen = pkg->GetWord();
		pkg->GetString(slen, szDealerId);
		CETF *petf = SetETF(szDealerId, szETF, szDescription, szBufAccount, szTradeAccount, unitSize, cash, maxCU, spread, upPrice, discPrice, flags);
		petf->journal = journal;
		petf->ContinueProcessETFInit(pkg);
		petf->InitForecast();
	}
	pthread_mutex_unlock(&mtex);
	RecalculateForecast();
}

CETF *CETFProcess::GetETF(const char *etfCode)
{
	pthread_mutex_lock(&mtex);
	_ETFMAP::iterator i = emap.find(etfCode);
	CETF *p = (i == emap.end()) ? NULL : i->second;
	pthread_mutex_unlock(&mtex);
	return p;
}

CETF *CETFProcess::SetETF(const char *dealerId, const char *etf, const char *desc, const char *bufAccount, const char *tradeAccount, OL_UINT32 uSize, OL_INT64 lCash,
						  OL_UINT32 mCU, double spread, double up, double disc, uint8_t fl)
{
	//	journal->WriteLog("[CETFProcess::SetETF] - DealerId:%s; ETFCode:%s; Desc:%s; BufAcct:%s; TradeAcct:%s; UnitSize:%u; Cash:%lu; MaxCU:%u; Spread:%f; Up:%f; Disc:%f; Flags:%u;\n",
	//		dealerId, etf, desc, bufAccount, tradeAccount, uSize, lCash, mCU, spread, up, disc, fl);
	_ETFMAP::iterator i = emap.find(etf);
	if (i == emap.end())
		return emap[etf] = new CETF(pidxdata, dealerId, etf, desc, bufAccount, tradeAccount, uSize, lCash, mCU, spread, up, disc, fl);
	else
	{
		i->second->flags = fl;
		i->second->maxCU = mCU;
		i->second->cash = lCash;
		i->second->spread = spread;
		i->second->upPrice = up;
		i->second->discPrice = disc;
		i->second->unitSize = uSize;
		strcpy(i->second->szDescription, desc);
		strcpy(i->second->szDealerId, dealerId);
		strcpy(i->second->szBufAccount, bufAccount);
		strcpy(i->second->szTradeAccount, tradeAccount);
	}
	return i->second;
}

void CETFProcess::RecalculateForecast()
{
	pthread_mutex_lock(&mtex);
	for (_ETFMAP::const_iterator i = emap.begin(); i != emap.end(); ++i)
		if (i->second)
		{
			i->second->RecalculateForecast();
			outq->AddtoQueue(i->second->ComposeForecastPackage());
		}
	pthread_mutex_unlock(&mtex);
}

void CETFProcess::forecast_worker(void *args)
{
	OL_UINT32 count = 0;
	CETFProcess *p = (CETFProcess *)args;
	while (p->bRunning)
	{
		switch (++count)
		{
		case 1: //	Recalculation of ETF Forecast
			p->RecalculateForecast();
			break;
		case 2: //	Check Criteria and Send Orders
			count = 0;
			p->ProcessCheckOrders();
			break;
		}
		sleep(1); //	Sleep for 1 second
	}
}

void CETFProcess::ProcessCheckOrders()
{
	CMutexGuard m(&stex);
	for (__SMAP__::const_iterator i = smap.begin(); i != smap.end(); ++i)
		CheckOrder(i->second);
}

void CETFProcess::ProcessSubscETFOrder(CTEOrderPackage *pkg)
{
	ETF_BATCH_ORDER *o = new ETF_BATCH_ORDER;
	OL_UINT16 slen = pkg->GetWord();
	pkg->GetString(slen, o->szId);
	slen = pkg->GetWord();
	pkg->GetString(slen, o->szETFCode);
	slen = pkg->GetWord();
	pkg->GetString(slen, o->szCustId);
	o->cmd = pkg->GetByte();
	o->limitNav = pkg->GetDouble();
	o->OUnit = pkg->GetInt64();
	o->ldate = pkg->GetDWord();
	o->orderOption = pkg->GetByte();
	o->TUnit = 0;
	o->maxCU = 0;
	o->status = 1;
	o->lastExecutedNav = 0;
	lCurDate = GetCurrentDate();
	if (lCurDate == o->ldate)
		AddOrder(o);
	else
	{
		char szT[2048];
		GetOrderDescriptions(szT, o);
		received->WriteLog("[ETF BATCH ORDER][INVALID DATE] - %s", szT);
		delete o;
	}
}

void CETFProcess::ProcessUnsubscETFOrder(CTEOrderPackage *pkg)
{
	char szOrderId[32] = {};
	OL_UINT16 slen = pkg->GetWord();
	pkg->GetString(slen, szOrderId);
	WithdrawOrder(szOrderId);
}

void CETFProcess::GetOrderDescriptions(char *szT, ETF_BATCH_ORDER *o)
{
	sprintf(szT, "OrderId:%s; CustId:%s; Cmd:%u; ETFCode:%s; CreationUnit:%lu; ExecutedUnit:%lu; LimitNav:%f; ExecutedNav:%f; OrderDate:%u; OrderOption:%u;\n",
			o->szId, o->szCustId, o->cmd, o->szETFCode, o->OUnit, o->TUnit, o->limitNav, o->lastExecutedNav, o->ldate, o->orderOption);
}

bool CETFProcess::IsDuplicateOrderId(ETF_BATCH_ORDER *o)
{
	CMutexGuard m(&stex);
	std::pair<__SMAP__::iterator, __SMAP__::iterator> iret = smap.equal_range(o->szETFCode);
	for (__SMAP__::iterator j = iret.first; j != iret.second; ++j)
		if (0 == strcmp(j->second->szId, o->szId))
			return true;
	return false;
}

bool CETFProcess::CheckOrder(ETF_BATCH_ORDER *o)
{
	if ((1 != o->status) || (o->TUnit >= o->OUnit) || (0 == o->OUnit))
		return true;
	// if ((55 == pidxdata->idxState.fStatus) || ('d' == pidxdata->idxState.fStatus) || bHoliday)	//	End Sending Records
	//	return false;
	CETF *petf = GetETF(o->szETFCode);
	if ((NULL == petf) || (o->TUnit > 0))
		return false;
	o->maxCU = petf->maxCU * petf->unitSize;
	CETFForecast fcast;
	petf->GetETFForecast(0, &fcast);
	if (0 == o->limitNav) //	direct compose order
		return ComposeETFOrderPackage(o);
	else
	{
		if ((0 == o->cmd) || (2 == o->cmd))
		{
			if (fcast.buyNav <= o->limitNav) //	Execute
				return ComposeETFOrderPackage(o);
		}
		else
		{
			if (fcast.sellNav >= o->limitNav) //	Execute
				return ComposeETFOrderPackage(o);
		}
	}
	return false;
}

bool CETFProcess::IsValidOrder(ETF_BATCH_ORDER *o)
{
	lCurDate = GetCurrentDate();
	if ((1 != o->status) || (o->TUnit >= o->OUnit) || (0 == o->OUnit) || (lCurDate != o->ldate))
		return false;
	// if ((55 == pidxdata->idxState.fStatus) || ('d' == pidxdata->idxState.fStatus) || bHoliday)	//	End Sending Records
	//	return false;
	CETF *petf = GetETF(o->szETFCode);
	if (NULL == petf)
		return false;
	return true;
}

void CETFProcess::AddOrder(ETF_BATCH_ORDER *o)
{
	if (NULL == o)
		return;
	char szT[4096];
	GetOrderDescriptions(szT, o);
	if (IsDuplicateOrderId(o))
	{
		received->WriteLog("[ETF BATCH ORDER][DUPLICATE] - %s", szT);
		delete o;
		o = NULL;
	}
	else
	{
		if (IsValidOrder(o))
		{
			received->WriteLog("[ETF BATCH ORDER] - %s", szT);
			if (false == CheckOrder(o))
			{
				pthread_mutex_lock(&stex);
				smap.insert(std::pair<std::string, ETF_BATCH_ORDER *>(o->szETFCode, o));
				pthread_mutex_unlock(&stex);
			}
		}
		else
		{
			delete o;
			received->WriteLog("[ETF BATCH ORDER][INVALID ORDER] - %s", szT);
		}
	}
}

void CETFProcess::WithdrawOrder(const char *orderId)
{
	char szT[4096];
	pthread_mutex_lock(&stex);
	for (__SMAP__::iterator i = smap.begin(); i != smap.end(); ++i)
		if (0 == strcmp(orderId, i->second->szId))
		{
			GetOrderDescriptions(szT, i->second);
			delete i->second;
			smap.erase(i);
			received->WriteLog("[ETF BATCH ORDER][WITHDRAW] - %s", szT);
			break;
		}
	pthread_mutex_unlock(&stex);
}

void CETFProcess::ContinueExecETFBatchOrder(const char *orderId)
{
	pthread_mutex_lock(&stex);
	for (__SMAP__::iterator i = smap.begin(); i != smap.end(); ++i)
		if (0 == strcmp(orderId, i->second->szId))
		{
			ComposeETFOrderPackage(i->second);
			break;
		}
	pthread_mutex_unlock(&stex);
}

void CETFProcess::WriteExecutedOrderLog(ETF_BATCH_ORDER *o, OL_UINT64 eUnit)
{
	char szT[4096];
	GetOrderDescriptions(szT, o);
	executed->WriteLog("CurExecutedUnit:%lu; %s", eUnit, szT);
}

bool CETFProcess::ComposeETFOrderPackage(ETF_BATCH_ORDER *o)
{
	if ((1 != o->status) || (o->TUnit >= o->OUnit))
		return true;
	OL_INT64 unit = o->OUnit - o->TUnit;
	if ((o->maxCU > 0) && (unit > o->maxCU))
		unit = o->maxCU;
	if (unit <= 0)
		return true;
	o->TUnit += unit;
	CRawPackage raw(PK_ZMQ_SUBSC_ETF_ORDER);
	raw.AddInt8(o->cmd);
	raw.AddInt64(unit);
	raw.AddString(o->szETFCode);
	raw.AddString(o->szCustId);
	raw.AddString(o->szId);
	raw.AddDouble(o->limitNav);
	raw.AddInt8(o->orderOption);
	if (o->TUnit >= o->OUnit)
		o->status = 2; //	Fully Executed
	WriteExecutedOrderLog(o, unit);
	outq->AddtoQueue(new CTEOrderPackage(&raw));
	return (2 == o->status);
}
