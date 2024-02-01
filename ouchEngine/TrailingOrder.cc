#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "header/journal.h"
#include "header/miscfunc.h"
#include "header/find_itch.h"
#include "header/ItchDefine.h"
#include "header/TrailingOrder.h"
#include "header/PriceFraction.h"
#include "header/TEOrderPackage.h"

CTrailingOrder::CTrailingOrder(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, CPriceFraction *pfrac, const char *addr) : pf(pfrac), journal(pjournal), received(preceived), executed(pexecuted) //	Address to connect
{
	nThread = 8;
	bThreadRunning = false;
	arrProcess.reserve(26);
	for (OL_UINT32 i = 0; i < 26; ++i)
	{
		char szIdentity[32];
		sprintf(szIdentity, "TrailingOrder_%c", (char)'A' + i);
		arrProcess[i] = new CTrailingProcess(pjournal, preceived, pexecuted, pfrac, szIdentity, addr);
	}
	arrProcess[0]->RequestResendAllTrailingOrder();
}

CTrailingOrder::~CTrailingOrder()
{
	StopThread();
	for (OL_UINT32 i = 0; i < 26; ++i)
		delete arrProcess[i];
	arrProcess.clear();
	queue.Release();
}

void CTrailingOrder::AddQuote(QUOTE *q)
{
	queue.AddtoQueue(q);
}

void CTrailingOrder::StartThread(OL_UINT16 n)
{
	if (bThreadRunning)
		return;
	bThreadRunning = true;
	nThread = (0 == n) ? 8 : n;
	ptid = new pthread_t[nThread];
	journal->WriteLog("[CTrailingOrder::StartThread] - Started %u quote_worker Threads.\n", nThread);
	for (OL_UINT16 i = 0; i < nThread; ++i)
	{
		if (pthread_create(ptid + i, NULL, (void *(*)(void *))quote_worker, (void *)this))
			journal->WriteLog("[CTrailingOrder::StartThread] - Can not create Quote Worker Thread_%u...\n", i);
	}
}

void CTrailingOrder::StopThread()
{
	if (!bThreadRunning)
		return;
	bThreadRunning = false;
	for (OL_UINT16 i = 0; i < nThread; ++i)
		pthread_join(*(ptid + i), NULL);
	delete[] ptid;
	ptid = NULL;
}

void CTrailingOrder::quote_worker(void *args)
{
	CTrailingOrder *p = (CTrailingOrder *)args;
	while (p->bThreadRunning)
	{
		QUOTE *q = p->queue.GetfromQueue();
		if (q)
		{
			if (q->szStockCode[0] >= 'A' && q->szStockCode[0] <= 'Z')
				p->arrProcess[q->szStockCode[0] - 'A']->ProcessData(q);
			delete q;
			q = NULL;
		}
		else
			usleep(500000);
	}
}

void CTrailingOrder::CTrailingProcess::ProcessOrderReply(CTEOrderPackage *pkg)
{
	char szRef[ORDER_REF_LEN + 1];
	OL_UINT16 errcode = pkg->GetErrorCode();
	pkg->GetString(ORDER_REF_LEN, szRef);
	if (0 != errcode)
	{
		pthread_mutex_lock(&mtex);
		__BOMAP__::const_iterator i = bmap.find(szRef);
		if (i != bmap.end())
			i->second->status = 0;
		pthread_mutex_unlock(&mtex);
	}
}

void CTrailingOrder::CTrailingProcess::ProcessSubscTrailingOrder(CTEOrderPackage *pkg)
{
	TRAILING_ORDER *o = new TRAILING_ORDER;
	OL_UINT16 slen = pkg->GetWord();
	pkg->GetString(slen, o->szId);
	slen = pkg->GetWord();
	pkg->GetString(slen, o->szCustId);
	o->cmd = pkg->GetByte();
	slen = pkg->GetWord();
	pkg->GetString(slen, o->szBoard);
	slen = pkg->GetWord();
	pkg->GetString(slen, o->szStockCode);
	o->status = 1;
	o->ExecPriceType = pkg->GetByte();
	o->volume = pkg->GetInt64();
	o->autoPriceStep = pkg->GetDWord();
	o->TPriceDrop = pkg->GetDWord();
	o->TPriceType = pkg->GetByte();
	o->TStepPercentage = pkg->GetDWord();
	o->StopPrice = pkg->GetDWord();
	o->TPrice = pkg->GetDWord();
	o->ExecPrice = pkg->GetDWord();
	o->startDate = pkg->GetDWord();
	o->dueDate = pkg->GetDWord();
	lCurDate = GetCurrentDate();
	if ((lCurDate >= o->startDate) && (lCurDate <= o->dueDate))
		AddOrder(o);
	else
		delete o;
}

void CTrailingOrder::CTrailingProcess::ProcessUnsubscTrailingOrder(CTEOrderPackage *pkg)
{
	char szOrderId[32];
	OL_UINT16 slen = pkg->GetWord();
	pkg->GetString(slen, szOrderId);
	WithdrawOrder(szOrderId);
}

void CTrailingOrder::CTrailingProcess::client_worker(void *args)
{
	zmq_msg_t msg;
	int size = 0, counter = 0;
	CTrailingOrder::CTrailingProcess *p = (CTrailingOrder::CTrailingProcess *)args;
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
			case PK_ZMQ_SUBSC_TRAILING_ORDER:
				p->ProcessSubscTrailingOrder(pkg);
				break;
			case PK_ZMQ_UNSUB_TRAILING_ORDER:
				p->ProcessUnsubscTrailingOrder(pkg);
				break;
			case PK_ZMQ_RESET_TRAILING_ORDER:
				p->CleanupAllTrailingOrder();
				break;
			}
			if (pkg)
			{
				delete pkg;
				pkg = NULL;
			}
		}
		zmq_msg_close(&msg);
	}
	if (-1 == size)
		zmq_msg_close(&msg);
	p->journal->WriteLog("client_worker %s ended.\n", p->szIdentity);
}

CTrailingOrder::CTrailingProcess::CTrailingProcess(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, CPriceFraction *pfrac,
												   const char *identity, const char *szAddress) : bRunning(true), journal(pjournal), received(preceived), executed(pexecuted), pf(pfrac)
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
	zmq_setsockopt(client, ZMQ_LINGER, &hwm, sizeof(hwm));	 //	Linger Period of Socket Shutdown
	zmq_setsockopt(client, ZMQ_RCVTIMEO, &hwm, sizeof(hwm)); //	Receive time out 1 second
	pthread_mutex_init(&mtex, NULL);
	pthread_mutex_init(&ztex, NULL);
	if (-1 == shm.Create(SHM_KEY_ITCH, SZ_ITCH_SHM))
	{
		journal->WriteLog("[CTrailingOrder::CTrailingProcess::CTrailingProcess][Id:%s][Addr:%s] - Failed to create Shared Memory.\n", szIdentity, szAddress);
		return;
	}
	if (NULL == (pidxdata = shm.AttachMem()))
	{
		journal->WriteLog("[CTrailingOrder::CTrailingProcess::CTrailingProcess][Id:%s][Addr:%s] - Failed to attach Shared Memory.\n", szIdentity, szAddress);
		return;
	}
	if (-1 == zmq_connect(client, szAddress))
	{
		journal->WriteLog("[CTrailingOrder::CTrailingProcess::CTrailingProcess][Id:%s][Addr:%s] - Failed to connect to frontend %s.\n", szIdentity, szAddress);
		return;
	}
	if (pthread_create(&tid, NULL, (void *(*)(void *))client_worker, (void *)this))
		journal->WriteLog("[CTrailingOrder::CTrailingProcess::CTrailingProcess] - Can not create Server Worker Thread ...\n");
}

CTrailingOrder::CTrailingProcess::~CTrailingProcess()
{
	bRunning = false;
	pthread_join(tid, NULL);
	Release();
	pthread_mutex_destroy(&mtex);
	pthread_mutex_destroy(&ztex);
	zmq_close(client);
	zmq_ctx_destroy(context);
	shm.DetachMem();
}

void CTrailingOrder::CTrailingProcess::Release()
{
	pthread_mutex_lock(&mtex);
	for (__AOMAP__::const_iterator i = amap.begin(); i != amap.end(); ++i)
		if (i->second)
			delete i->second;
	amap.clear();
	bmap.clear();
	pthread_mutex_unlock(&mtex);
}

void CTrailingOrder::CTrailingProcess::GetOrderDescriptions(char *szT, TRAILING_ORDER *o)
{
	char szCondition[2048];
	if ((0 == o->cmd) || (2 == o->cmd))
	{
		char szBuyAt[32];
		o->StopPrice = o->TPrice * (1.0 + (o->TStepPercentage / 10000.0));
		if (0 == o->ExecPriceType)
			strcpy(szBuyAt, "Best Offer Price");
		else
			sprintf(szBuyAt, "Best Offer + %u", o->ExecPriceType);
		sprintf(szCondition, "Buy at %s if Last Price is lower than %u and Upper than %0.02f%% from the Lowest Price:%u at Stop Price:%u",
				szBuyAt, o->TPriceDrop, (float)(o->TStepPercentage / 100.0), o->TPrice, o->StopPrice);
	}
	else
	{
		char szSellAt[32];
		o->StopPrice = o->TPrice * (1.0 - (o->TStepPercentage / 10000.0));
		if (0 == o->ExecPriceType)
			strcpy(szSellAt, "Best Bid Price");
		else
			sprintf(szSellAt, "Best Bid + %u", o->ExecPriceType);
		sprintf(szCondition, "Sell at %s if drop %0.02f%% from %s:%u at Stop Price:%u",
				szSellAt, (float)(o->TStepPercentage / 100.0),
				(0 == o->TPriceType) ? "Best Bid Price" : (1 == o->TPriceType) ? "Best Offer Price"
													  : (2 == o->TPriceType)   ? "Last Price"
													  : (3 == o->TPriceType)   ? "Avg Price"
													  : (4 == o->TPriceType)   ? "Highest Price"
																			   : "Lowest Price",
				o->TPrice, o->StopPrice);
	}
	sprintf(szT, "OrderId:%s; CustId:%s; Cmd:%u; StockCode:%s.%s; Volume:%ld; AutoPriceStep:%u; StartDate:%u; DueDate:%u; Condition:%s;\n",
			o->szId, o->szCustId, o->cmd, o->szStockCode, o->szBoard, o->volume, o->autoPriceStep, o->startDate, o->dueDate, szCondition);
}

bool CTrailingOrder::CTrailingProcess::CheckOrder(TRAILING_ORDER *o)
{
	if ((1 != o->status) || (lCurDate < o->startDate || lCurDate > o->dueDate))
		return true; //	return true to delete order
	//	FIX4 DataFeed
	// if ((55 == pidxdata->idxState.fStatus) || ('d' == pidxdata->idxState.fStatus) || bHoliday)	//	End Sending Records
	//	return false;

	//	ITCH
	if ((SEC_END_OF_SYSTEM_HOURS == pidxdata->idxState.fStatus) || (SEC_END_OF_MESSAGES == pidxdata->idxState.fStatus) || bHoliday) //	End Sending Records
		return false;
	IDX_STOCK *ps = pidxdata->idxStock;
	OL_UINT32 lTotStk = pidxdata->idxState.lTotalStock;
	OL_INT32 ib = find_idx_board(o->szBoard), is = find_idx_stock(o->szStockCode, ps, lTotStk, SZ_IDX_STOCK);
	if (-1 == ib || -1 == is)
		return false;
	IDX_STOCK_DATA *psd = &ps[is].stockData[ib];
	OL_UINT32 TPrice = ((0 == o->TPriceType) ? psd->lBestBidPrice : (1 == o->TPriceType) ? psd->lBestOfferPrice
																: (2 == o->TPriceType)	 ? psd->lLastPrice
																: (3 == o->TPriceType)	 ? psd->lAvgPrice
																: (4 == o->TPriceType)	 ? psd->lHighPrice
																						 : psd->lLowPrice) /
					   100;
	if (0 == TPrice)
		return false;
	if ((0 == o->cmd) || (2 == o->cmd)) //	Trailing Buy
	{
		if (TPrice < o->TPrice)
		{
			o->TPrice = TPrice;
			o->StopPrice = pf->GetFracPrice((1.0 + (o->TStepPercentage / 10000.0)) * o->TPrice);
			o->ExecPrice = pf->GetFracPrice(o->StopPrice, true, o->ExecPriceType);
			SendUpdateTrailingState(o);
		}
	}
	else //	Trailing Sell
	{
		if (TPrice > o->TPrice)
		{
			o->TPrice = TPrice;
			o->StopPrice = pf->GetFracPrice((1.0 - (o->TStepPercentage / 10000.0)) * o->TPrice);
			o->ExecPrice = pf->GetFracPrice(o->StopPrice, false, o->ExecPriceType);
			SendUpdateTrailingState(o);
		}
	}
	OL_UINT32 lastPrice = psd->lLastPrice / 100;
	bool bProcess = (0 == o->cmd || 2 == o->cmd) ? (lastPrice >= o->StopPrice) && (lastPrice < o->TPriceDrop) : lastPrice <= o->StopPrice;
	if (bProcess)
	{
		o->status = 2;
		if ((0 == o->cmd) || (2 == o->cmd))
			o->ExecPrice = (0 == o->ExecPriceType) ? psd->lBestOfferPrice / 100 : pf->GetFracPrice(psd->lBestOfferPrice / 100, true, o->ExecPriceType);
		else
			o->ExecPrice = (0 == o->ExecPriceType) ? psd->lBestBidPrice / 100 : pf->GetFracPrice(psd->lBestBidPrice / 100, false, o->ExecPriceType);
		if (0 == o->ExecPrice)
			o->ExecPrice = psd->lLastPrice / 100;
		//	Lakukan pengiriman order ke TE
		ComposeOrderPackage(o);
	}
	return bProcess;
}

void CTrailingOrder::CTrailingProcess::AddOrder(TRAILING_ORDER *o)
{
	if (NULL == o)
		return;
	char szStockCode[64], szT[4096];
	GetOrderDescriptions(szT, o);
	sprintf(szStockCode, "%s.%s", o->szStockCode, o->szBoard);
	pthread_mutex_lock(&mtex);
	__BOMAP__::const_iterator i = bmap.find(o->szId);
	if (i == bmap.end())
	{
		received->WriteLog("[Trailing Order] - %s", szT);
		if (false == CheckOrder(o))
		{
			bmap.insert(std::pair<std::string, TRAILING_ORDER *>(o->szId, o));
			amap.insert(std::pair<std::string, TRAILING_ORDER *>(szStockCode, o));
		}
		else
			delete o;
	}
	else
	{
		received->WriteLog("[Trailing Order][DUPLICATE] - %s", szT);
		delete o;
		o = NULL;
	}
	pthread_mutex_unlock(&mtex);
}

void CTrailingOrder::CTrailingProcess::WithdrawOrder(const char *orderId)
{
	char szT[4096];
	pthread_mutex_lock(&mtex);
	__BOMAP__::iterator i = bmap.find(orderId);
	if (i != bmap.end())
	{
		TRAILING_ORDER *o = i->second;
		GetOrderDescriptions(szT, o);
		bmap.erase(i);
		char szStockCode[64];
		sprintf(szStockCode, "%s.%s", o->szStockCode, o->szBoard);
		std::pair<__AOMAP__::iterator, __AOMAP__::iterator> iret = amap.equal_range(szStockCode);
		for (__AOMAP__::iterator j = iret.first; j != iret.second; ++j)
			if (0 == strcmp(j->second->szId, orderId))
			{
				delete o;
				amap.erase(j);
				received->WriteLog("[Trailing Order][WITHDRAW] - %s", szT);
				break;
			}
	}
	pthread_mutex_unlock(&mtex);
}

void CTrailingOrder::CTrailingProcess::WriteExecutedOrderLog(TRAILING_ORDER *o)
{
	char szT[4096];
	GetOrderDescriptions(szT, o);
	executed->WriteLog("CnnId:%s; %s", szIdentity, szT);
}

void CTrailingOrder::CTrailingProcess::ComposeOrderPackage(TRAILING_ORDER *o)
{
	CTEOrderPackage *opkg = new CTEOrderPackage();
	opkg->AllocateBuffer(155 + ORDER_REF_LEN);
	if (0 == strcmp(RG_BOARD, o->szBoard))
		opkg->SetPackageId(PK_TE_NEW_REGULAR_ORDER);
	else if (0 == strcmp(TN_BOARD, o->szBoard))
		opkg->SetPackageId(PK_TE_NEW_CASH_ORDER);
	else if (0 == strcmp(ADV_BOARD, o->szBoard))
		opkg->SetPackageId(PK_TE_NEW_ADVERTISEMENT_ORDER);
	opkg->SetServerFd(0);
	opkg->AddString(o->szCustId, 8);
	opkg->AddString("", 25); //	PIN
	opkg->AddByte(3);		 //	Automatic Order
	opkg->AddByte(o->cmd);
	opkg->AddByte(0);			  //	Day Session
	opkg->AddByte(0);			  //	Randomized
	opkg->AddDWord(1);			  //	N Split Order
	opkg->AddDWord(o->ExecPrice); //	Price
	opkg->AddInt64(o->volume);	  //	Volume
	opkg->AddString(o->szStockCode, 30);
	opkg->AddString(o->szCustId, 8);
	opkg->AddString(o->szId, 30);
	opkg->AddString(o->szId, ORDER_REF_LEN);
	opkg->AddString(szIdentity, 25);
	opkg->AddByte(0);						 //	bPriceStep
	opkg->AddDWord(0);						 //	Price Step
	opkg->AddByte(o->autoPriceStep ? 1 : 0); //	bAutoPriceStep
	opkg->AddDWord(o->autoPriceStep);		 //	Auto Price Step
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char *)*opkg, opkg->GetPackageLength(), 0);
	WriteExecutedOrderLog(o);
	pthread_mutex_unlock(&ztex);
	delete opkg;
	opkg = NULL;
}

void CTrailingOrder::CTrailingProcess::SendHeartBeat()
{
	CTEOrderPackage *opkg = new CTEOrderPackage();
	opkg->AllocateBuffer(0);
	opkg->SetPackageId(PK_HTS_HEART_BEAT);
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char *)*opkg, opkg->GetPackageLength(), 0);
	pthread_mutex_unlock(&ztex);
	delete opkg;
	opkg = NULL;
}

void CTrailingOrder::CTrailingProcess::RequestResendAllTrailingOrder()
{
	CTEOrderPackage *opkg = new CTEOrderPackage();
	opkg->AllocateBuffer(0);
	opkg->SetPackageId(PK_ZMQ_REQ_ALL_OPEN_TRAILING);
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char *)*opkg, opkg->GetPackageLength(), 0);
	pthread_mutex_unlock(&ztex);
	delete opkg;
	opkg = NULL;
}

void CTrailingOrder::CTrailingProcess::SendUpdateTrailingState(TRAILING_ORDER *o)
{
	CTEOrderPackage *opkg = new CTEOrderPackage();
	opkg->AllocateBuffer(59);
	opkg->SetPackageId(PK_ZMQ_UPDATE_TRAILING_ORDER);
	opkg->AddString(o->szId, 12);
	opkg->AddString(o->szStockCode, 30);
	opkg->AddString(o->szBoard, 5);
	opkg->AddDWord(o->TPrice);
	opkg->AddDWord(o->StopPrice);
	opkg->AddDWord(o->ExecPrice);
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char *)*opkg, opkg->GetPackageLength(), 0);
	pthread_mutex_unlock(&ztex);
	delete opkg;
	opkg = NULL;
}

OL_UINT32 CTrailingOrder::CTrailingProcess::ProcessData(QUOTE *q)
{
	if (NULL == q || bHoliday)
		return 0;
	OL_UINT32 nOrder = 0;
	pthread_mutex_lock(&mtex);
	std::pair<__AOMAP__::iterator, __AOMAP__::iterator> iret = amap.equal_range(q->szStockCode);
	for (__AOMAP__::iterator i = iret.first; i != iret.second; ++i)
	{
		TRAILING_ORDER *o = i->second;
		if (NULL == o)
			continue;
		if ((1 != o->status) || (lCurDate < o->startDate || lCurDate > o->dueDate))
			continue;
		OL_UINT32 TPrice = (0 == o->TPriceType) ? q->lBestBidPrice : (1 == o->TPriceType) ? q->lBestOfferPrice
																 : (2 == o->TPriceType)	  ? q->lLastPrice
																 : (3 == o->TPriceType)	  ? q->lAvgPrice
																 : (4 == o->TPriceType)	  ? q->lHighPrice
																						  : q->lLowPrice;
		if (0 == TPrice)
			continue;
		if ((0 == o->cmd) || (2 == o->cmd)) //	Trailing Buy
		{
			if (TPrice < o->TPrice)
			{
				o->TPrice = TPrice;
				o->StopPrice = pf->GetFracPrice((1.0 + (o->TStepPercentage / 10000.0)) * o->TPrice);
				o->ExecPrice = pf->GetFracPrice(o->StopPrice, true, o->ExecPriceType);
				SendUpdateTrailingState(o);
			}
		}
		else //	Trailing Sell
		{
			if (TPrice > o->TPrice)
			{
				o->TPrice = TPrice;
				o->StopPrice = pf->GetFracPrice((1.0 - (o->TStepPercentage / 10000.0)) * o->TPrice);
				o->ExecPrice = pf->GetFracPrice(o->StopPrice, false, o->ExecPriceType);
				SendUpdateTrailingState(o);
			}
		}
		bool bProcess = (0 == o->cmd || 2 == o->cmd) ? (q->lLastPrice >= o->StopPrice) && (q->lLastPrice < o->TPriceDrop) : q->lLastPrice <= o->StopPrice;
		if (bProcess)
		{
			++nOrder;
			o->status = 2;
			if ((0 == o->cmd) || (2 == o->cmd))
				o->ExecPrice = (0 == o->ExecPriceType) ? q->lBestOfferPrice : pf->GetFracPrice(q->lBestOfferPrice, true, o->ExecPriceType);
			else
				o->ExecPrice = (0 == o->ExecPriceType) ? q->lBestBidPrice : pf->GetFracPrice(q->lBestBidPrice, false, o->ExecPriceType);
			if (0 == o->ExecPrice)
				o->ExecPrice = q->lLastPrice;
			//	Lakukan pengiriman order ke TE
			ComposeOrderPackage(o);
		}
	}
	pthread_mutex_unlock(&mtex);
	return nOrder;
}

void CTrailingOrder::CTrailingProcess::CleanupAllTrailingOrder()
{
	static unsigned int counter = 0;
	static pthread_mutex_t ctex = PTHREAD_MUTEX_INITIALIZER;
	journal->WriteLog("[%s] - Receiving Cleanup Trailing Order Signal, cleaning up all Trailing Orders ...\n", szIdentity);
	pthread_mutex_lock(&mtex);
	for (__AOMAP__::iterator i = amap.begin(); i != amap.end(); ++i)
		delete i->second;
	amap.clear();
	bmap.clear();
	pthread_mutex_unlock(&mtex);
	pthread_mutex_lock(&ctex);
	if (26 == ++counter)
	{
		counter = 0;
		journal->WriteLog("[%s] - End of cleanup Trailing Order Process, requesting all open Trailing Orders ...\n", szIdentity);
		sleep(30);
		RequestResendAllTrailingOrder();
	}
	pthread_mutex_unlock(&ctex);
}
