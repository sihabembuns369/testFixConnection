#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "header/journal.h"
#include "header/AutoOrder.h"
#include "header/miscfunc.h"
#include "header/ItchDefine.h"
#include "header/find_itch.h"
#include "header/TEOrderPackage.h"

CAutoOrder::CAutoOrder(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, const char *addr):ptid(NULL), 
	journal(pjournal), received(preceived), executed(pexecuted)	//	Address to connect
{
	nThread = 8;
	bThreadRunning = false;
	arrProcess.reserve(26);
	for (OL_UINT32 i = 0; i < 26; ++i)
	{
		char szIdentity[64];
		sprintf(szIdentity, "AutoOrder_%c", (char)'A' + i);
		arrProcess[i] = new CAutoProcess(pjournal, preceived, pexecuted, szIdentity, addr);
	}
	arrProcess[0]->RequestResendAllBreakOrder();
}

CAutoOrder::~CAutoOrder()
{
	StopThread();
	for (OL_UINT32 i = 0; i < 26; ++i)
		delete arrProcess[i];
	arrProcess.clear();
	queue.Release();
}

void CAutoOrder::AddQuote(QUOTE *q)
{
	queue.AddtoQueue(q);
}

void CAutoOrder::StartThread(OL_UINT16 n)
{
	if (bThreadRunning)
		return;
	bThreadRunning = true;
	nThread = (0 == n)?8:n;
	ptid = new pthread_t[nThread];
	journal->WriteLog("[CAutoOrder::StartThread] - Started %u quote_worker Threads.\n", nThread);
	for (OL_UINT16 i = 0; i < nThread; ++i)
	{
		if (pthread_create(ptid + i, NULL, (void* (*)(void*))quote_worker, (void*)this))
			journal->WriteLog("[CAutoOrder::StartThread] - Can not create Quote Worker Thread_%u...\n", i);
	}
}

void CAutoOrder::StopThread()
{
	if (!bThreadRunning)
		return;
	bThreadRunning = false;
	for (OL_UINT16 i = 0; i < nThread; ++i)
		pthread_join(*(ptid + i), NULL);
	delete []ptid; ptid = NULL;
}

void CAutoOrder::quote_worker(void *args)
{
	CAutoOrder *p = (CAutoOrder *)args;
	while (p->bThreadRunning)
	{
		QUOTE *q = p->queue.GetfromQueue();
		if (q)
		{
			if (q->szStockCode[0] >= 'A' && q->szStockCode[0] <= 'Z')
				p->arrProcess[q->szStockCode[0] - 'A']->ProcessData(q);
			delete q; q = NULL;
		}
		else
			usleep(100000);
	}
}

void CAutoOrder::CAutoProcess::ProcessOrderReply(CTEOrderPackage *pkg)
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

void CAutoOrder::CAutoProcess::ProcessSubscBreakOrder(CTEOrderPackage *pkg)
{
	AUTO_ORDER *o = new AUTO_ORDER;
	OL_UINT16 slen = pkg->GetWord(); pkg->GetString(slen, o->szId);
	slen = pkg->GetWord(); pkg->GetString(slen, o->szCustId);
	o->cmd = pkg->GetByte();
	slen = pkg->GetWord(); pkg->GetString(slen, o->szBoard);
	slen = pkg->GetWord(); pkg->GetString(slen, o->szStockCode);
	o->status = 1;
	o->price = pkg->GetDWord();
	o->volume = pkg->GetInt64();
	o->autoPriceStep = pkg->GetDWord();
	o->TPriceType = pkg->GetByte();
	o->TPriceCriteria = pkg->GetByte();
	o->TPrice = pkg->GetDWord();
	o->TVolumeType = pkg->GetByte();
	o->TVolumeCriteria = pkg->GetByte();
	o->TVolume = pkg->GetInt64();
	o->startDate = pkg->GetDWord();
	o->dueDate = pkg->GetDWord();
	lCurDate = GetCurrentDate();
	if ((lCurDate >= o->startDate) && (lCurDate <= o->dueDate))
		AddOrder(o);
	else
		delete o;
}

void CAutoOrder::CAutoProcess::ProcessUnsubscBreakOrder(CTEOrderPackage *pkg)
{
	char szOrderId[32];
	OL_UINT16 slen = pkg->GetWord(); pkg->GetString(slen, szOrderId);
	WithdrawOrder(szOrderId);
}

void CAutoOrder::CAutoProcess::client_worker(void *args)
{
	zmq_msg_t msg;
	int size = 0, counter = 0;
	CAutoOrder::CAutoProcess *p = (CAutoOrder::CAutoProcess *)args;
	p->journal->WriteLog("client_worker %s is running.\n", p->szIdentity);
	while (p->bRunning)
	{
		if (-1 != size)
			zmq_msg_init(&msg);
		size = zmq_msg_recv(&msg, p->client, 0);
		if (-1 == size)
		{
			OL_UINT32 lDate = GetCurrentDate();
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
			case PK_ZMQ_SUBSC_BREAK_ORDER:
				p->ProcessSubscBreakOrder(pkg);
				break;
			case PK_ZMQ_UNSUB_BREAK_ORDER:
				p->ProcessUnsubscBreakOrder(pkg);
				break;
			case PK_ZMQ_RESET_BREAK_ORDER:
				p->CleanupAllBreakOrder();
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

CAutoOrder::CAutoProcess::CAutoProcess(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, const char *identity, const char *szAddress):
	bRunning(true), journal(pjournal), received(preceived), executed(pexecuted)
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
	zmq_setsockopt(client, ZMQ_LINGER, &hwm, sizeof(hwm));	//	Linger Period of Socket Shutdown
	zmq_setsockopt(client, ZMQ_RCVTIMEO, &hwm, sizeof(hwm));	//	Receive time out 1 second
	pthread_mutex_init(&mtex, NULL);
	pthread_mutex_init(&ztex, NULL);
	if (-1 == shm.Create(SHM_KEY_ITCH, SZ_ITCH_SHM))
	{
		journal->WriteLog("[CAutoOrder::CAutoProcess::CAutoProcess][Id:%s][Addr:%s] - Failed to create Shared Memory.\n", szIdentity, szAddress);
		return;
	}
	if (NULL == (pidxdata = shm.AttachMem()))
	{
		journal->WriteLog("[CAutoOrder::CAutoProcess::CAutoProcess][Id:%s][Addr:%s] - Failed to attach Shared Memory.\n", szIdentity, szAddress);
		return;
	}
	if (-1 == zmq_connect(client, szAddress))
	{
		journal->WriteLog("[CAutoOrder::CAutoProcess::CAutoProcess][Id:%s][Addr:%s] - Failed to connect to frontend %s.\n", szIdentity, szAddress);
		return;
	}
	if (pthread_create(&tid, NULL, (void* (*)(void*))client_worker, (void*)this))
		journal->WriteLog("[CAutoOrder::CAutoProcess::CAutoProcess] - Can not create Server Worker Thread ...\n");
}

CAutoOrder::CAutoProcess::~CAutoProcess()
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

void CAutoOrder::CAutoProcess::Release()
{
	pthread_mutex_lock(&mtex);
	for (__AOMAP__::const_iterator i = amap.begin(); i != amap.end(); ++i)
		delete i->second;
	amap.clear();
	bmap.clear();
	pthread_mutex_unlock(&mtex);
}

void CAutoOrder::CAutoProcess::GetOrderDescriptions(char *szT, AUTO_ORDER *o)
{
	char szCondition[2048];
	sprintf(szCondition, "%s IF %s %s %u",
		(0 == o->cmd || 2 == o->cmd)?"BUY":"SELL",
		(0 == o->TPriceType)?"Best Bid Price":(1 == o->TPriceType)?"Best Offer Price":(2 == o->TPriceType)?"Last Price":"Avg Price",
		(0 == o->TPriceCriteria)?"Equal to":(1 == o->TPriceCriteria)?"Less than":"Greater than", o->TPrice);
	if (0 != o->TVolumeType)
	{
		sprintf(szCondition + strlen(szCondition), " AND %s %s %ld", 
			(1 == o->TVolumeType)?"Traded Volume":(2 == o->TVolumeType)?"Best Bid Volume":"Best Offer Volume",
			(0 == o->TVolumeCriteria)?"Less Than Equal to":"Greater Than Equal to", o->TVolume);
	}
	sprintf(szT, "OrderId:%s; CustId:%s; Cmd:%u; StockCode:%s.%s; Price:%u; Volume:%ld; AutoPriceStep:%u; StartDate:%u; DueDate:%u; Condition:%s;\n",
		o->szId, o->szCustId, o->cmd, o->szStockCode, o->szBoard, o->price, o->volume, o->autoPriceStep, o->startDate, o->dueDate, szCondition);
}

bool CAutoOrder::CAutoProcess::CheckOrder(AUTO_ORDER *o)
{
	if ( (1 != o->status) || (lCurDate < o->startDate || lCurDate > o->dueDate) )
		return true;	//	return true to delete order
	//	FIX4 DataFeed
	//if ((55 == pidxdata->idxState.fStatus) || ('d' == pidxdata->idxState.fStatus) || bHoliday)	//	End Sending Records
	//	return false;
	
	//	ITCH
	if ((SEC_END_OF_SYSTEM_HOURS == pidxdata->idxState.fStatus) || (SEC_END_OF_MESSAGES == pidxdata->idxState.fStatus) || bHoliday)	//	End Sending Records
		return false;
	IDX_STOCK *ps = pidxdata->idxStock;
	OL_UINT32 lTotStk = pidxdata->idxState.lTotalStock;
	OL_INT32 ib = find_idx_board(o->szBoard), is = find_idx_stock(o->szStockCode, ps, lTotStk, SZ_IDX_STOCK);
	if (-1 == ib || -1 == is)
	{
		journal->WriteLog("Can not find Stock : %s; Board : %s;\n", o->szStockCode, o->szBoard);
		return false;
	}
	IDX_STOCK_DATA *psd = &ps[is].stockData[ib];
	OL_UINT32 TPrice = ((0 == o->TPriceType)?psd->lBestBidPrice:(1 == o->TPriceType)?psd->lBestOfferPrice:(2 == o->TPriceType)?psd->lLastPrice:psd->lAvgPrice) / 100;
	if (0 == TPrice)
		return false;
	bool bProcess = (0 == o->TPriceCriteria)?o->TPrice == TPrice:(1 == o->TPriceCriteria)?TPrice < o->TPrice:TPrice > o->TPrice;
	if (o->TVolumeType) // > 0
	{
		OL_UINT64 TVolume = (1 == o->TVolumeType)?psd->lTradedVolume:(2 == o->TVolumeType)?psd->lBestBidVolume:psd->lBestOfferVolume;
		bProcess &= (0 == o->TVolumeCriteria)?TVolume <= o->TVolume:TVolume >= o->TVolume;
	}
	if (bProcess)
	{
		o->status = 2;	//	Sent
		//	Lakukan pengiriman order ke TE
		ComposeOrderPackage(o);
	}
	return bProcess;
}

void CAutoOrder::CAutoProcess::AddOrder(AUTO_ORDER *o)
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
		received->WriteLog("[Break Order] - %s", szT);
		if (false == CheckOrder(o))
		{
			bmap.insert(std::pair<std::string, AUTO_ORDER*>(o->szId, o));
			amap.insert(std::pair<std::string, AUTO_ORDER*>(szStockCode, o));
		}
		else
			delete o;
	}
	else
	{
		received->WriteLog("[Break Order][DUPLICATE] - %s", szT);
		delete o; o = NULL;
	}
	pthread_mutex_unlock(&mtex);
}

void CAutoOrder::CAutoProcess::WithdrawOrder(const char *orderId)
{
	char szT[4096];
	pthread_mutex_lock(&mtex);
	__BOMAP__::iterator i = bmap.find(orderId);
	if (i != bmap.end())
	{
		AUTO_ORDER *o = i->second;
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
				received->WriteLog("[Break Order][WITHDRAW] - %s", szT);
				break;
			}
	}
	pthread_mutex_unlock(&mtex);
}

void CAutoOrder::CAutoProcess::WriteExecutedOrderLog(AUTO_ORDER *o)
{
	char szT[4096];
	GetOrderDescriptions(szT, o);
	executed->WriteLog("CnnId:%s; %s", szIdentity, szT);
}

void CAutoOrder::CAutoProcess::ComposeOrderPackage(AUTO_ORDER *o)
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
	opkg->AddByte(3);	//	Automatic Order
	opkg->AddByte(o->cmd);
	opkg->AddByte(0);	//	Day Session
	opkg->AddByte(0);	//	Randomized
	opkg->AddDWord(1);	//	N Split Order
	opkg->AddDWord(o->price);	//	Price
	opkg->AddInt64(o->volume);	//	Volume
	opkg->AddString(o->szStockCode, 30);
	opkg->AddString(o->szCustId, 8);
	opkg->AddString(o->szId, 30);
	opkg->AddString(o->szId, ORDER_REF_LEN);
	opkg->AddString(szIdentity, 25);
	opkg->AddByte(0);	//	bPriceStep
	opkg->AddDWord(0);	//	Price Step
	opkg->AddByte(o->autoPriceStep?1:0);	//	bAutoPriceStep
	opkg->AddDWord(o->autoPriceStep);	//	Auto Price Step
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char*)*opkg, opkg->GetPackageLength(), 0);
	WriteExecutedOrderLog(o);
	pthread_mutex_unlock(&ztex);
	delete opkg; opkg = NULL;
}

void CAutoOrder::CAutoProcess::SendHeartBeat()
{
	CTEOrderPackage *opkg = new CTEOrderPackage();
	opkg->AllocateBuffer(0);
	opkg->SetPackageId(PK_HTS_HEART_BEAT);
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char *)*opkg, opkg->GetPackageLength(), 0);
	pthread_mutex_unlock(&ztex);
	delete opkg; opkg = NULL;
}

void CAutoOrder::CAutoProcess::RequestResendAllBreakOrder()
{
	CTEOrderPackage *opkg = new CTEOrderPackage();
	opkg->AllocateBuffer(0);
	opkg->SetPackageId(PK_ZMQ_REQ_ALL_OPEN_BREAK);
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char *)*opkg, opkg->GetPackageLength(), 0);
	pthread_mutex_unlock(&ztex);
	delete opkg; opkg = NULL;
}

OL_UINT32 CAutoOrder::CAutoProcess::ProcessData(QUOTE *q)
{
	if (NULL == q || bHoliday)
		return 0;
	OL_UINT32 nOrder = 0;
	pthread_mutex_lock(&mtex);
	std::pair<__AOMAP__::iterator, __AOMAP__::iterator> iret = amap.equal_range(q->szStockCode);
	for (__AOMAP__::iterator i = iret.first; i != iret.second; ++i)
	{
		AUTO_ORDER *o = i->second;
		if (NULL == o)
			continue;
		if ( (1 != o->status) || (lCurDate < o->startDate || lCurDate > o->dueDate) )
			continue;
		OL_UINT32 TPrice = (0 == o->TPriceType)?q->lBestBidPrice:(1 == o->TPriceType)?q->lBestOfferPrice:(2 == o->TPriceType)?q->lLastPrice:q->lAvgPrice;
		if (0 == TPrice)
			continue;
		bool bProcess = (0 == o->TPriceCriteria)?o->TPrice == TPrice:(1 == o->TPriceCriteria)?TPrice < o->TPrice:TPrice > o->TPrice;
		if (o->TVolumeType) // > 0
		{
			OL_UINT64 TVolume = (1 == o->TVolumeType)?q->lVolume:(2 == o->TVolumeType)?q->lBestBidVolume:q->lBestOfferVolume;
			bProcess &= (0 == o->TVolumeCriteria)?TVolume <= o->TVolume:TVolume >= o->TVolume;
		}
		if (bProcess)
		{
			++nOrder;
			o->status = 2;	//	Sent
			//	Lakukan pengiriman order ke TE
			ComposeOrderPackage(o);
		}
	}
	pthread_mutex_unlock(&mtex);
	return nOrder;
}

void CAutoOrder::CAutoProcess::CleanupAllBreakOrder()
{
	static unsigned int counter = 0;
	static pthread_mutex_t ctex = PTHREAD_MUTEX_INITIALIZER;
	journal->WriteLog("[%s] - Receiving Cleanup Break Order Signal, cleaning up all Break Orders ...\n", szIdentity);
	pthread_mutex_lock(&mtex);
	for (__AOMAP__::iterator i = amap.begin(); i != amap.end(); ++i)
		if (i->second)
			delete i->second;
	amap.clear();
	bmap.clear();
	pthread_mutex_unlock(&mtex);
	pthread_mutex_lock(&ctex);
	if (26 == ++counter)
	{
		counter = 0;
		journal->WriteLog("[%s] - End of cleanup Break Order Process, requesting all open Break Order ...\n", szIdentity);
		sleep(30);
		RequestResendAllBreakOrder();
	}
	pthread_mutex_unlock(&ctex);
}
