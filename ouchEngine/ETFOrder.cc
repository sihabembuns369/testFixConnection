#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "header/journal.h"
#include "header/ETFOrder.h"
#include "header/miscfunc.h"
#include "header/idxdtfeed.h"
#include "header/ItchDefine.h"
#include "header/find_itch.h"
#include "header/RawPackage.h"
#include "header/TEOrderPackage.h"

const uint32_t SHARES_LOT = 100;

CETFOrder::CETFOrderParams::CETFOrderParams():
	TWAIT_AR(5 * 60),	//	Masa tunggun jika terjadi Auto Rejection (5 menit)
	TWAIT_FRAC(20), 	//	Masa tunggu jika terjadi kenaikan harga fraksi 
	NFRAC(1), 			//	Jumlah fraksi harga saat eksekusi
	SPREAD_FRAC(9),		//	Batasan selisih fraksi harga untuk dapat di eksekusi
	MAX_VOLUME(50000 * SHARES_LOT)	//	Max volume dalam 1 transaksi (dalam lembar saham).
{
}

CETFOrder::CETFOrder(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, const char *addr):ptid(NULL), 
	nThread(1), bThreadRunning(false), journal(pjournal), received(preceived), executed(pexecuted)
{
	pf.LoadData();
	(eprocess = new CEProcess(pjournal, preceived, pexecuted, &pf, op, ETF_ORDER_IDENTITY, addr))->RequestResendAllETFOrder();
}

CETFOrder::~CETFOrder()
{
	StopThread();
	delete eprocess;
	eprocess = NULL;
	queue.Release();
}

void CETFOrder::AddQuote(QUOTE *q)
{
	queue.AddtoQueue(q);
}

void CETFOrder::StartThread(OL_UINT16 n)
{
	if (bThreadRunning)
		return;
	bThreadRunning = true;
	nThread = (0 == n)?2:n;
	ptid = new pthread_t[nThread];
	journal->WriteLog("[CETFOrder::StartThread] - Started %u quote_worker Threads.\n", nThread);
	for (OL_UINT16 i = 0; i < nThread; ++i)
	{
		if (pthread_create(ptid + i, NULL, (void* (*)(void*))quote_worker, (void*)this))
			journal->WriteLog("[CETFOrder::StartThread] - Can not create Quote Worker Thread_%u...\n", i);
	}
}

void CETFOrder::StopThread()
{
	if (!bThreadRunning)
		return;
	bThreadRunning = false;
	for (OL_UINT16 i = 0; i < nThread; ++i)
		pthread_join(*(ptid + i), NULL);
	delete []ptid; ptid = NULL;
}

void CETFOrder::quote_worker(void *args)
{
	QUOTE *q = NULL;
	CETFOrder *p = (CETFOrder *)args;
	while (p->bThreadRunning)
	{
		if ((q = p->queue.GetfromQueue()))
		{
			p->eprocess->ProcessData(q);
			delete q; q = NULL;
		}
		else
			usleep(500000);
	}
}

void CETFOrder::CEProcess::ProcessOrderReply(CTEOrderPackage *pkg)
{
	char szRef[ORDER_REF_LEN + 1] = {};
	OL_UINT16 errcode = pkg->GetErrorCode();
	pkg->GetString(ORDER_REF_LEN, szRef);
	if (0 != errcode)
	{
		pthread_mutex_lock(&mtex);
		__OMAP__::const_iterator i = omap.find(szRef);
		if (i != omap.end())
			i->second->status = 0;
		pthread_mutex_unlock(&mtex);
	}
}

void CETFOrder::CEProcess::ProcessSubscETFUnderlyingOrder(CTEOrderPackage *pkg)
{
	ETF_ORDER *o = new ETF_ORDER;
	OL_UINT16 slen = pkg->GetWord(); pkg->GetString(slen, o->szId);
	slen = pkg->GetWord(); pkg->GetString(slen, o->szRef);
	slen = pkg->GetWord(); pkg->GetString(slen, o->szETFCode);
	slen = pkg->GetWord(); pkg->GetString(slen, o->szCustId);
	o->cmd = pkg->GetByte();
	slen = pkg->GetWord(); pkg->GetString(slen, o->szBoard);
	slen = pkg->GetWord(); pkg->GetString(slen, o->szStockCode);
	o->volume = o->remain = pkg->GetInt64();
	o->ldate = pkg->GetDWord();
	o->orderOption = pkg->GetByte();
	if (o->orderOption > 2)
		o->orderOption = 2;
	o->executed = o->execPrice = o->nRetry = 0;
	o->status = 1;
	AddOrder(o);
}

void CETFOrder::CEProcess::ProcessUnsubscETFOrder(CTEOrderPackage *pkg)
{
	char szOrderId[32];
	OL_UINT16 slen = pkg->GetWord(); pkg->GetString(slen, szOrderId);
	WithdrawOrder(szOrderId);
}

long CETFOrder::CEProcess::GetAutoRejectionPercentage(uint32_t price)
{
	if (price <= 200)
		return 35;
	else if (price > 5000)
		return 20;
	return 25;
}

void CETFOrder::CEProcess::order_worker(void *args)
{
	int32_t maxAR = 0, maxPrice = 0, minPrice = 0;
	OL_INT32 eprice = 0;
	ETF_ORDER *o = NULL;
	IDX_TRADE_BOOK *ptb = NULL;
	IDX_ORDER_BOOK *pob = NULL;
	IDX_STOCK_DATA *psd = NULL;
	OL_INT32 is = 0, ib = 0;	//	Assume RG Board
	OL_UINT8 tradingState = 0;
	char szStockCode[32] = {};
	OL_INT64 bVol = 0, oVol = 0, eVol = 0;
	CETFOrder::CEProcess *p = (CETFOrder::CEProcess *)args;
	IDX_CURRENT_STATE *pstate = &(p->pidxdata->idxState);
	IDX_STOCK *ps = p->pidxdata->idxStock;
	p->journal->WriteLog("order_worker %s is running.\n", p->szIdentity);
	while (p->bRunning)
	{
		//if ((0x33 != pstate->fStatus) 		//	Begin First Session
		//	&& (0x35 != pstate->fStatus)	//	Begin Second Session
		//	&& ('f' != pstate->fStatus)		//	Trading Activation
		//	&& ('l' != pstate->fStatus))	//	Market Activation
		//{
		//	sleep(1);
		//	continue;
		//}
		if ( (SEC_START_OF_MARKET_HOURS != pstate->fStatus) && 
			 (SEC_TRADING_ACTIVATION != pstate->fStatus) )
		{
			sleep(1);
			continue;
		}
		memset(szStockCode, 0x00, sizeof(szStockCode));
		for (__SMAP__::iterator i = p->smap.begin(); i != p->smap.end(); ++i)
		{
			if (1 != (o = i->second)->status)
				continue;
			if (o->ldate != p->lCurDate)
			{
				o->status = 0;
				continue;
			}
			if ((0 != strcmp(szStockCode, o->szStockCode)) || (0 == bVol) || (0 == oVol))
			{
				is = find_idx_stock(o->szStockCode, ps, p->pidxdata->idxState.lTotalStock, SZ_IDX_STOCK);
				if (-1 == is)
					continue;
				pob = &ps[is].orderBook[ib];
				psd = &ps[is].stockData[ib];
				ptb = psd->tradeBook;
				bVol = psd->lBestBidVolume;
				oVol = psd->lBestOfferVolume;
				tradingState = psd->tradingState;
				strcpy(szStockCode, o->szStockCode);
				if (ps[is].fStatus & IDX_MASK_PREOPEN)
				{
					uint32_t lprice = ((0 == psd->lOpenPrice)?psd->lPrevPrice:psd->lOpenPrice) / 100;
					maxAR = p->GetAutoRejectionPercentage(lprice);
					maxPrice = p->pf->GetFracPrice((1.0 + (maxAR / 100.0)) * lprice);
					minPrice = p->pf->GetAtomicFracPrice((1.0 - (maxAR / 100.0)) * lprice, true);
				}
				else
				{
					uint32_t lprice = psd->lPrevPrice / 100;
					maxAR = p->GetAutoRejectionPercentage(lprice);
					maxPrice = p->pf->GetFracPrice((1.0 + (maxAR / 100.0)) * lprice);
					minPrice = p->pf->GetAtomicFracPrice((1.0 - (maxAR / 100.0)) * lprice, true);
				}
			}
			if ((0 == o->cmd) || (2 == o->cmd) || (4 == o->cmd))	//	Buy Order
			{
				if ((0 == oVol) || (0 == psd->lBestOfferPrice))
				{
				//	if (0 == (ps[is].fStatus & IDX_MASK_SUSPENDED))	//	If Suspended then release the order directly
					if (TRADING_ACTION_STATE_ACTIVE == tradingState)//	If Suspended then release the order directly
					{
						//if (psd->lChangeRate < maxAR)
						//	continue;
						if ((int32_t)(psd->lLastPrice / 100) < maxPrice)
							continue;
						if (o->nRetry++ < (p->op + o->orderOption)->TWAIT_AR)	//	5 minutes
							continue;
						else
							o->nRetry = 0;
					}
				}
				else if ((0 != o->execPrice) && (o->execPrice < psd->lBestOfferPrice / 100))
				{
					uint32_t tbperc = 0;
					int itb = find_idx_trade_book(psd->lBestOfferPrice, ptb, psd->lTotalTradeBook, SZ_IDX_TRADE_BOOK);
					if (-1 != itb)
						tbperc = (ptb[itb].lVolume * 100) / oVol;
					if ((o->nRetry++ < (p->op + o->orderOption)->TWAIT_FRAC) && (tbperc < 10))	//	10 seconds
						continue;
					else
						o->nRetry = 0;
				}
				int32_t fprice = p->pf->GetFracPrice(psd->lLastPrice / 100, true, (p->op + o->orderOption)->SPREAD_FRAC);
				eprice = (0 == psd->lBestOfferPrice)?(psd->lLastPrice / 100) + 1 : psd->lBestOfferPrice / 100;
				if ((eprice > fprice) || ((0 == oVol) && (psd->lBestOfferPrice > 0)))
					continue;
				if (0 == oVol)
					eVol = o->remain;
				else
				{
					eVol = (o->remain < oVol)?o->remain:oVol;
					if (eVol > (p->op + o->orderOption)->MAX_VOLUME)
						eVol = (p->op + o->orderOption)->MAX_VOLUME;
					oVol -= eVol;
				}
				if (pob->lOfferDepth > 1)
				{
					uint32_t n = (pob->lOfferDepth > ((p->op + o->orderOption)->NFRAC + 1))?(p->op + o->orderOption)->NFRAC:pob->lOfferDepth - 1;
					eprice = p->pf->GetFracPrice(eprice, true, n);
				}
			}
			else
			{
				if ((0 == bVol) || (0 == psd->lBestBidPrice))
				{
					//if (0 == (ps[is].fStatus & IDX_MASK_SUSPENDED))	//	If Suspended then release the order directly
					if (TRADING_ACTION_STATE_ACTIVE == tradingState)//	If Suspended then release the order directly
					{
						//if (abs(psd->lChangeRate) < maxAR)
						//	continue;
						if ((int32_t)(psd->lLastPrice / 100) > minPrice)
							continue;
						if (o->nRetry++ < (p->op + o->orderOption)->TWAIT_AR)
							continue;
						else
							o->nRetry = 0;
					}
				}
				else if ((0 != o->execPrice) && (o->execPrice > psd->lBestBidPrice / 100))
				{
					uint32_t tbperc = 0;
					int itb = find_idx_trade_book(psd->lBestBidPrice, ptb, psd->lTotalTradeBook, SZ_IDX_TRADE_BOOK);
					if (-1 != itb)
						tbperc = (ptb[itb].lVolume * 100) / bVol;
					if ((o->nRetry++ < (p->op + o->orderOption)->TWAIT_FRAC) && (tbperc < 10))
						continue;
					else
						o->nRetry = 0;
				}
				int32_t fprice = p->pf->GetFracPrice(psd->lLastPrice / 100, false, (p->op + o->orderOption)->SPREAD_FRAC);
				eprice = (0 == psd->lBestBidPrice)?(psd->lLastPrice / 100) - 1 : psd->lBestBidPrice / 100;
				if ((eprice < fprice) || ((0 == bVol) && (psd->lBestBidPrice > 0)))
					continue;
				if (0 == bVol)
					eVol = o->remain;
				else
				{
					eVol = (o->remain < bVol)?o->remain:bVol;
					if (eVol > (p->op + o->orderOption)->MAX_VOLUME)
						eVol = (p->op + o->orderOption)->MAX_VOLUME;
					bVol -= eVol;
				}
				if (pob->lBidDepth > 1)
				{
					uint32_t n = (pob->lBidDepth > ((p->op + o->orderOption)->NFRAC + 1))?(p->op + o->orderOption)->NFRAC:pob->lBidDepth - 1;
					eprice = p->pf->GetFracPrice(eprice, false, n);
				}
			}
			if (eVol > 0)
				p->ComposeOrderPackage(o, eprice, eVol);
		}
		sleep(1);	//	1 second
	}
	p->journal->WriteLog("order_worker %s ended.\n", p->szIdentity);
}

void CETFOrder::CEProcess::client_worker(void *args)
{
	zmq_msg_t msg;
	int size = 0, counter = 0;
	CETFOrder::CEProcess *p = (CETFOrder::CEProcess *)args;
	p->journal->WriteLog("client_worker %s is running.\n", p->szIdentity);
	while (p->bRunning)
	{
		if (-1 != size)
			zmq_msg_init(&msg);
		size = zmq_msg_recv(&msg, p->client, 0);
		if (-1 == size)
		{
			uint32_t lDate = GetCurrentDate();
			if (lDate > p->lCurDate)
			{
				p->bHoliday = isHoliday(p->lCurDate);
			}
			if (5 == ++counter)
			{
				counter = 0;
				p->SendHeartBeat();
			}
			continue;
		}
		else
			counter = 0;
		if (size >= (int)TE_HEADER_LENGTH)
		{
			char *psz = new char[size];
			memcpy(psz, zmq_msg_data(&msg), size);
			CTEOrderPackage *pkg = new CTEOrderPackage();
			pkg->SetRawData(psz, size);
			switch (pkg->GetPackageId())
			{
			case PK_TE_NEW_REGULAR_ORDER:
			case PK_TE_NEW_CASH_ORDER:
			case PK_TE_NEW_ADVERTISEMENT_ORDER:
				p->ProcessOrderReply(pkg);
				break;
			case PK_ZMQ_SUBSC_ETF_UNDERLYING:
				p->ProcessSubscETFUnderlyingOrder(pkg);
				break;
			//case PK_ZMQ_UNSUB_ETF_ORDER:
			//	p->ProcessUnsubscETFOrder(pkg);
			//	break;
			case PK_ZMQ_RESET_ETF_ORDER:
				p->CleanupAllETFOrder();
				break;
			}
			if (pkg)
			{
				delete pkg; pkg = NULL;
			}
		}
		zmq_msg_close(&msg);
	}
	if (-1 == size)
		zmq_msg_close(&msg);
	p->journal->WriteLog("client_worker %s ended.\n", p->szIdentity);
}

CETFOrder::CEProcess::CEProcess(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, CPriceFraction *frac, CETFOrderParams *param, const char *identity, const char *szAddress):
	bRunning(true), journal(pjournal), received(preceived), executed(pexecuted), pf(frac), op(param)
{
	context = zmq_ctx_new();
	bHoliday = isHoliday(lCurDate);
	strcpy(szIdentity, identity);
	client = zmq_socket(context, ZMQ_DEALER);
	zmq_setsockopt(client, ZMQ_IDENTITY, szIdentity, strlen(szIdentity));
	int hwm = 0;
	zmq_setsockopt(client, ZMQ_RCVHWM, &hwm, sizeof(hwm));
	zmq_setsockopt(client, ZMQ_SNDHWM, &hwm, sizeof(hwm));
	hwm = 1000;
	zmq_setsockopt(client, ZMQ_LINGER, &hwm, sizeof(hwm));		//	Linger Period of Socket Shutdown
	zmq_setsockopt(client, ZMQ_RCVTIMEO, &hwm, sizeof(hwm));	//	Receive time out 1 second
	pthread_mutex_init(&mtex, NULL);
	pthread_mutex_init(&ztex, NULL);
	if (-1 == shm.Create(SHM_KEY_ITCH, SZ_ITCH_SHM))
	{
		journal->WriteLog("[CETFOrder::CEProcess::CEProcess][Id:%s][Addr:%s] - Failed to create Shared Memory.\n", szIdentity, szAddress);
		return;
	}
	if (NULL == (pidxdata = shm.AttachMem()))
	{
		journal->WriteLog("[CETFOrder::CEProcess::CEProcess][Id:%s][Addr:%s] - Failed to attach Shared Memory.\n", szIdentity, szAddress);
		return;
	}
	if (-1 == zmq_connect(client, szAddress))
	{
		journal->WriteLog("[CETFOrder::CEProcess::CEProcess][Id:%s][Addr:%s] - Failed to connect to frontend %s.\n", szIdentity, szAddress);
		return;
	}
	if (pthread_create(&tid, NULL, (void* (*)(void*))client_worker, (void*)this))
		journal->WriteLog("[CETFOrder::CEProcess::CEProcess] - Can not create Server Client Worker Thread ...\n");
	if (pthread_create(&oid, NULL, (void* (*)(void*))order_worker, (void*)this))
		journal->WriteLog("[CETFOrder::CEProcess::CEProcess] - Can not create Server Order Worker Thread ...\n");
}

CETFOrder::CEProcess::~CEProcess()
{
	bRunning = false;
	pthread_join(tid, NULL);
	pthread_join(oid, NULL);
	Release();
	pthread_mutex_destroy(&mtex);
	pthread_mutex_destroy(&ztex);
	zmq_close(client);
	zmq_ctx_destroy(context);
	shm.DetachMem();
}

void CETFOrder::CEProcess::Release()
{
	pthread_mutex_lock(&mtex);
	for (__SMAP__::const_iterator i = smap.begin(); i != smap.end(); ++i)
		delete i->second;
	smap.clear();
	omap.clear();
	pthread_mutex_unlock(&mtex);
}

void CETFOrder::CEProcess::GetOrderDescriptions(char *szT, ETF_ORDER *o, OL_UINT64 execVolume)
{
	sprintf(szT, "OrderId:%s; RefId:%s; ETFCode:%s; CustId:%s; Cmd:%u; StockCode:%s.%s; Price:%u; OVolume:%ld; Remain:%ld; Executed:%ld; OrderDate:%u; OrderOption:%u;\n",
		o->szId, o->szRef, o->szETFCode, o->szCustId, o->cmd, o->szStockCode, o->szBoard, o->execPrice, execVolume, o->remain, o->executed, o->ldate, o->orderOption);
}

// bool CETFOrder::CEProcess::CheckOrder(ETF_ORDER *o)
// {
	// if ((55 == pidxdata->idxState.fStatus) || ('d' == pidxdata->idxState.fStatus) || bHoliday)	//	End Sending Records
		// return false;
	// IDX_STOCK *ps = pidxdata->idxStock;
	// OL_UINT32 lTotStk = pidxdata->idxState.lTotalStock;
	// OL_INT32 ib = find_idx_board(o->szBoard), is = find_idx_stock(o->szStockCode, ps, lTotStk, SZ_IDX_STOCK);
	// if (-1 == ib || -1 == is)
	// {
		// journal->WriteLog("Can not find Stock : %s; Board : %s;\n", o->szStockCode, o->szBoard);
		// return false;
	// }
	// IDX_STOCK_DATA *psd = &ps[is].stockData[ib];
	// if (1 != o->status)
		// return false;
	// OL_UINT32 TPrice = ((0 == o->TPriceType)?psd->lBestBidPrice:(1 == o->TPriceType)?psd->lBestOfferPrice:(2 == o->TPriceType)?psd->lLastPrice:psd->lAvgPrice) / 100;
	// if (0 == TPrice)
		// return false;
	// bool bProcess = (0 == o->TPriceCriteria)?o->TPrice == TPrice:(1 == o->TPriceCriteria)?TPrice < o->TPrice:TPrice > o->TPrice;
	// if (o->TVolumeType) // > 0
	// {
		// OL_UINT64 TVolume = (1 == o->TVolumeType)?psd->lTradedVolume:(2 == o->TVolumeType)?psd->lBestBidVolume:psd->lBestOfferVolume;
		// bProcess &= (0 == o->TVolumeCriteria)?TVolume <= o->TVolume:TVolume >= o->TVolume;
	// }
	// if (bProcess)
	// {
		// o->status = 2;	//	Sent
		// //	Lakukan pengiriman order ke TE
		// ComposeOrderPackage(o);
	// }
	// return bProcess;
// }

void CETFOrder::CEProcess::AddOrder(ETF_ORDER *o)
{
	if (NULL == o)
		return;
	char szStockCode[64] = {}, szT[2048] = {};
	GetOrderDescriptions(szT, o, 0);
	sprintf(szStockCode, "%s.%s", o->szStockCode, o->szBoard);
	//if (bHoliday || (o->ldate != lCurDate))
	if (o->ldate != lCurDate)
	{
		if (bHoliday)
			received->WriteLog("[ETF Order][HOLIDAY] - %s", szT);
		else
			received->WriteLog("[ETF Order][INVALID ORDER DATE] - %s", szT);
		delete o; o = NULL;
		return;
	}
	if (o->remain <= 0)
	{
		received->WriteLog("[ETF Order][INVALID VOLUME] - %s", szT);
		delete o; o = NULL;
		return;
	}
	pthread_mutex_lock(&mtex);
	__OMAP__::const_iterator i = omap.find(o->szId);
	if (i == omap.end())
	{
		received->WriteLog("[ETF Order] - %s", szT);
		omap.insert(std::pair<std::string, ETF_ORDER*>(o->szId, o));
		smap.insert(std::pair<std::string, ETF_ORDER*>(szStockCode, o));
	}
	else
	{
		received->WriteLog("[ETF Order][DUPLICATE] - %s", szT);
		delete o; o = NULL;
	}
	pthread_mutex_unlock(&mtex);
}

void CETFOrder::CEProcess::WithdrawOrder(const char *orderId)
{
	char szStockCode[64] = {}, szT[2048] = {};
	pthread_mutex_lock(&mtex);
	__OMAP__::iterator i = omap.find(orderId);
	if (i != omap.end())
	{
		ETF_ORDER *o = i->second;
		GetOrderDescriptions(szT, o, 0);
		omap.erase(i);
		sprintf(szStockCode, "%s.%s", o->szStockCode, o->szBoard);
		std::pair<__SMAP__::iterator, __SMAP__::iterator> iret = smap.equal_range(szStockCode);
		for (__SMAP__::iterator j = iret.first; j != iret.second; ++j)
			if (0 == strcmp(j->second->szId, orderId))
			{
				delete o;
				smap.erase(j);
				received->WriteLog("[ETF Order][WITHDRAW] - %s", szT);
				break;
			}
	}
	pthread_mutex_unlock(&mtex);
}

void CETFOrder::CEProcess::WriteExecutedOrderLog(ETF_ORDER *o, OL_UINT64 execVolume)
{
	char szT[2048];
	GetOrderDescriptions(szT, o, execVolume);
	executed->WriteLog("CnnId:%s; %s", szIdentity, szT);
}

void CETFOrder::CEProcess::ComposeOrderPackage(ETF_ORDER *o, OL_UINT32 price, OL_UINT64 volume)
{
	CTEOrderPackage *opkg = new CTEOrderPackage();
	opkg->AllocateBuffer(156 + ORDER_REF_LEN);
	if (0 == strcmp(RG_BOARD, o->szBoard))
		opkg->SetPackageId(PK_TE_NEW_REGULAR_ORDER);
	else if (0 == strcmp(TN_BOARD, o->szBoard))
		opkg->SetPackageId(PK_TE_NEW_CASH_ORDER);
	else if (0 == strcmp(ADV_BOARD, o->szBoard))
		opkg->SetPackageId(PK_TE_NEW_ADVERTISEMENT_ORDER);
	opkg->SetServerFd(0);
	opkg->AddString(o->szCustId, 8);
	opkg->AddString("", 25);	//	PIN
	opkg->AddByte(SRC_ETF_ORDER);	//	ETF Order
	opkg->AddByte(o->cmd);
	opkg->AddByte(0);	//	Day Session
	opkg->AddByte(0);	//	Randomized
	opkg->AddDWord(1);	//	N Split Order
	opkg->AddDWord(o->execPrice = price);	//	Price
	opkg->AddInt64(volume);	//	Volume
	opkg->AddString(o->szStockCode, 30);
	opkg->AddString(o->szCustId, 8);
	opkg->AddString(o->szId, 30);
	opkg->AddString(o->szId, ORDER_REF_LEN);
	opkg->AddString(szIdentity, 25);
	opkg->AddByte(0);	//	bPriceStep
	opkg->AddDWord(0);	//	Price Step
	opkg->AddByte(0);	//	bAutoPriceStep
	opkg->AddDWord(0);	//	Auto Price Step
	o->executed += volume;
	o->remain -= volume;
	if (o->remain <= 0)
		o->status = 2;
	o->nRetry = 0;
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char*)*opkg, opkg->GetPackageLength(), 0);
	WriteExecutedOrderLog(o, volume);
	pthread_mutex_unlock(&ztex);
	delete opkg; opkg = NULL;
}
/*
void CETFOrder::CEProcess::ComposeETFBatchOrder(ETF_BATCH_ORDER *o)
{
	OL_UINT32 unit = 0;
	CRawPackage raw(PK_ZMQ_SUBSC_ETF_ORDER);
	raw.AddInt8(o->cmd);
	raw.AddInt32(unit);
	raw.AddString(o->szETFCode);
	raw.AddString(o->szCustId);
	raw.AddString(o->szId);
	raw.AddDouble(o->limitNav);
	
	CTEOrderPackage pkg(&raw);
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char*)pkg, pkg.GetPackageLength(), 0);
	//WriteExecutedOrderLog(o, volume);
	pthread_mutex_unlock(&ztex);
}
*/
void CETFOrder::CEProcess::SendHeartBeat()
{
	CTEOrderPackage *opkg = new CTEOrderPackage(0, PK_HTS_HEART_BEAT);
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char *)*opkg, opkg->GetPackageLength(), 0);
	pthread_mutex_unlock(&ztex);
	delete opkg; opkg = NULL;
}

void CETFOrder::CEProcess::RequestResendAllETFOrder()
{
	CTEOrderPackage *opkg = new CTEOrderPackage(1, PK_ZMQ_REQ_ALL_OPEN_ETF_ORDER);
	opkg->AddByte(0x01);	//	Request Open ETF Underlying Orders
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char *)*opkg, opkg->GetPackageLength(), 0);
	pthread_mutex_unlock(&ztex);
	delete opkg; opkg = NULL;
}

OL_UINT32 CETFOrder::CEProcess::ProcessData(QUOTE *q)
{
	if (NULL == q || bHoliday)
		return 0;
	OL_UINT32 nOrder = 0;
	// pthread_mutex_lock(&mtex);
	// std::pair<__SMAP__::iterator, __SMAP__::iterator> iret = amap.equal_range(q->szStockCode);
	// for (__SMAP__::iterator i = iret.first; i != iret.second; ++i)
	// {
		// ETF_ORDER *o = i->second;
		// if (NULL == o)
			// continue;
		// if (1 != o->status)
			// continue;
		// OL_UINT32 TPrice = (0 == o->TPriceType)?q->lBestBidPrice:(1 == o->TPriceType)?q->lBestOfferPrice:(2 == o->TPriceType)?q->lLastPrice:q->lAvgPrice;
		// if (0 == TPrice)
			// continue;
		// bool bProcess = (0 == o->TPriceCriteria)?o->TPrice == TPrice:(1 == o->TPriceCriteria)?TPrice < o->TPrice:TPrice > o->TPrice;
		// if (o->TVolumeType) // > 0
		// {
			// OL_UINT64 TVolume = (1 == o->TVolumeType)?q->lVolume:(2 == o->TVolumeType)?q->lBestBidVolume:q->lBestOfferVolume;
			// bProcess &= (0 == o->TVolumeCriteria)?TVolume <= o->TVolume:TVolume >= o->TVolume;
		// }
		// if (bProcess)
		// {
			// ++nOrder;
			// o->status = 2;	//	Sent
			// //	Lakukan pengiriman order ke TE
			// ComposeOrderPackage(o);
		// }
	// }
	// pthread_mutex_unlock(&mtex);
	return nOrder;
}

void CETFOrder::CEProcess::CleanupAllETFOrder()
{
	journal->WriteLog("[%s] - Receiving Cleanup ETF Order Signal, cleaning up all ETF Orders ...\n", szIdentity);
	pthread_mutex_lock(&mtex);
	for (__OMAP__::iterator i = omap.begin(); i != omap.end(); ++i)
		if (i->second)
			delete i->second;
	omap.clear();
	smap.clear();
	pthread_mutex_unlock(&mtex);
	journal->WriteLog("[%s] - End of cleanup ETF Order Process, requesting all open ETF Order ...\n", szIdentity);
	sleep(30);
	RequestResendAllETFOrder();
}
