#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "header/journal.h"
#include "header/miscfunc.h"
#include "header/idxdtfeed.h"
#include "header/ItchDefine.h"
#include "header/find_itch.h"
#include "header/InlineOrder.h"
#include "header/PriceFraction.h"
#include "header/TEOrderPackage.h"

CInlineOrder::CInlineOrder(OL_UINT32 lotSize, CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, CPriceFraction *pfrac, const char *addr) :
	SHARES_LOT(lotSize), limitVolRatio(5), orderBookDepth(5),
	pf(pfrac), journal(pjournal), received(preceived), executed(pexecuted) 	//	Address to connect
	
{
	nThread = 8;
	bThreadRunning = false;
	arrProcess.reserve(26);
	for (OL_UINT32 i = 0; i < 26; ++i)
	{
		char szIdentity[32];
		sprintf(szIdentity, "LimitOrder_%c", (char)'A' + i);
		arrProcess[i] = new CInlineProcess(lotSize, pjournal, preceived, pexecuted, pfrac, szIdentity, addr);
		arrProcess[i]->SetParams(limitVolRatio, orderBookDepth);
	}
	arrProcess[0]->RequestResendAllInlineOrder();
}

CInlineOrder::~CInlineOrder()
{
	StopThread();
	for (OL_UINT32 i = 0; i < 26; ++i)
		delete arrProcess[i];
	arrProcess.clear();
	queue.Release();
}

void CInlineOrder::AddQuote(QUOTE *q)
{
	queue.AddtoQueue(q);
}

void CInlineOrder::StartThread(OL_UINT16 n)
{
	if (bThreadRunning)
		return;
	bThreadRunning = true;
	nThread = (0 == n)?8:n;
	ptid = new pthread_t[nThread];
	journal->WriteLog("[CInlineOrder::StartThread] - Started %u quote_worker Threads.\n", nThread);
	for (OL_UINT16 i = 0; i < nThread; ++i)
	{
		if (pthread_create(ptid + i, NULL, (void* (*)(void*))quote_worker, (void*)this))
			journal->WriteLog("[CInlineOrder::StartThread] - Can not create Quote Worker Thread_%u...\n", i);
	}
}

void CInlineOrder::StopThread()
{
	if (!bThreadRunning)
		return;
	bThreadRunning = false;
	for (OL_UINT16 i = 0; i < nThread; ++i)
		pthread_join(*(ptid + i), NULL);
	delete []ptid; ptid = NULL;
}

void CInlineOrder::PrintQuote(QUOTE *q)
{
	journal->WriteLog("Quote : Stock:%s; Time:%u; Prev:%u %d/%d; Last:%u %d/%d; Open:%u; High:%u; Low:%u; Avg:%u; "
		"Freq:%u; Vol:%llu; Val:%llu; M.Cap:%llu; BidPrice:%u; BidVol:%llu; OfferPrice:%u; OfferVol:%llu.\n",
		q->szStockCode, q->lTime, q->lPrevPrice, q->lPrevChg, q->lPrevChgRate, q->lLastPrice, q->lChg, q->lChgRate, q->lOpenPrice,
		q->lHighPrice, q->lLowPrice, q->lAvgPrice, q->lFreq, q->lVolume, q->lValue, q->lMarketCap, q->lBestBidPrice, q->lBestBidVolume, 
		q->lBestOfferPrice, q->lBestOfferVolume);
}

void CInlineOrder::quote_worker(void *args)
{
	CInlineOrder *p = (CInlineOrder *)args;
	while (p->bThreadRunning)
	{
		QUOTE *q = p->queue.GetfromQueue();
		if (q)
		{
			if (q->szStockCode[0] >= 'A' && q->szStockCode[0] <= 'Z')
			{
				//p->PrintQuote(q);
				p->arrProcess[q->szStockCode[0] - 'A']->ProcessData(q);
			}
			delete q; q = NULL;
		}
		else
			usleep(500000);
	}
}

void CInlineOrder::CInlineProcess::ProcessOrderReply(CTEOrderPackage *pkg)
{
	char szRef[ORDER_REF_LEN + 1] = {};
	OL_UINT16 errcode = pkg->GetErrorCode();
	pkg->GetString(ORDER_REF_LEN, szRef);
	if (0 != errcode)
	{
		pthread_mutex_lock(&mtex);
		__IOMAP__::const_iterator i = imap.find(szRef);
		if (i != imap.end())
			i->second->status = 0;
		pthread_mutex_unlock(&mtex);
	}
}

void CInlineOrder::CInlineProcess::GetOrderDescriptions(char *szT, INLINE_ORDER *o)
{
	sprintf(szT, "OrderId:%s; CustId:%s; Cmd:%u; StockCode:%s.%s; LimitPrice:%u; LimitVolume:%ld; Volume:%ld;, UpperExecRatio:%u; BelowExecRatio:%u; IsNOW:%u; "
		"srcId:%u; expiry:%u; OrderDate:%u; InputUser:%s;\n",
		o->szId, o->szCustId, o->cmd, o->szStockCode, o->szBoard, o->LimitPrice, o->LimitVolume, o->OVolume, o->UpperExecRatio, o->BelowExecRatio, o->Flags,
		o->sourceId, o->expiry, o->ODate, o->szInputUser
	);
}

void CInlineOrder::CInlineProcess::ProcessSubscInlineOrder(CTEOrderPackage *pkg)
{
	INLINE_ORDER *o = new INLINE_ORDER;
	OL_UINT16 slen = pkg->GetWord(); pkg->GetString(slen, o->szId);
	slen = pkg->GetWord(); pkg->GetString(slen, o->szCustId);
	o->sourceId = pkg->GetByte();
	o->expiry = pkg->GetByte();
	o->orderType = pkg->GetByte();
	o->orderOption = pkg->GetByte();
	o->cmd = pkg->GetByte();
	slen = pkg->GetWord(); pkg->GetString(slen, o->szBoard);
	slen = pkg->GetWord(); pkg->GetString(slen, o->szStockCode);
	o->status = 1;	//	Open Order
	o->LimitPrice = pkg->GetDWord();
	o->OVolume = pkg->GetInt64();
	o->LimitVolume = o->OVolume * limitVolRatio / 100;
	o->LimitVolume -= o->LimitVolume % SHARES_LOT;
	o->UpperExecRatio = pkg->GetWord();
	o->BelowExecRatio = pkg->GetWord();
	o->Flags = pkg->GetDWord();
	slen = pkg->GetWord(); pkg->GetString(slen, o->szInputUser);
	o->ODate = pkg->GetDWord();
	lCurDate = GetCurrentDate();
	o->TVolume = 
	o->ExecVolume = 
	o->TradedValue = 
	o->CapturedMarketVolume = 
	o->TradedMarketVolume = 
	o->LastExecVolume = 
	o->LastExecPrice = 0;
	if (lCurDate == o->ODate)
		AddOrder(o);
	else
		delete o;
}

void CInlineOrder::CInlineProcess::ProcessUnsubscInlineOrder(CTEOrderPackage *pkg)
{
	char szOrderId[32] = {};
	OL_UINT16 slen = pkg->GetWord(); pkg->GetString(slen, szOrderId);
	WithdrawOrder(szOrderId);
}

void CInlineOrder::CInlineProcess::client_worker(void *args)
{
	zmq_msg_t msg;
	int size = 0, counter = 0;
	CInlineOrder::CInlineProcess *p = (CInlineOrder::CInlineProcess *)args;
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
			case PK_ZMQ_SUBSC_INLINE_ORDER:
				p->ProcessSubscInlineOrder(pkg);
				break;
			case PK_ZMQ_UNSUB_INLINE_ORDER:
				p->ProcessUnsubscInlineOrder(pkg);
				break;
			case PK_ZMQ_RESET_INLINE_ORDER:
				p->CleanupAllInlineOrder();
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

CInlineOrder::CInlineProcess::CInlineProcess(uint32_t lotSize, CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, CPriceFraction *pfrac, 
	const char *identity, const char *szAddress) : 
	bRunning(true), SHARES_LOT(lotSize), limitVolRatio(5), orderBookDepth(5), journal(pjournal), received(preceived), executed(pexecuted), pf(pfrac)
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
	zmq_setsockopt(client, ZMQ_RCVTIMEO, &hwm, sizeof(hwm));	//	Receive time out 1 second
	pthread_mutex_init(&mtex, NULL);
	pthread_mutex_init(&ztex, NULL);
	if (-1 == shm.Create(SHM_KEY_ITCH, SZ_ITCH_SHM))
	{
		journal->WriteLog("[CInlineOrder::CInlineProcess::CInlineProcess][Id:%s][Addr:%s] - Failed to create Shared Memory.\n", szIdentity, szAddress);
		return;
	}
	if (NULL == (pidxdata = shm.AttachMem()))
	{
		journal->WriteLog("[CInlineOrder::CInlineProcess::CInlineProcess][Id:%s][Addr:%s] - Failed to attach Shared Memory.\n", szIdentity, szAddress);
		return;
	}
	if (-1 == zmq_connect(client, szAddress))
	{
		journal->WriteLog("[CInlineOrder::CInlineProcess::CInlineProcess][Id:%s][Addr:%s] - Failed to connect to frontend %s.\n", szIdentity, szAddress);
		return;
	}
	if (pthread_create(&tid, NULL, (void* (*)(void*))client_worker, (void*)this))
		journal->WriteLog("[CInlineOrder::CInlineProcess::CInlineProcess] - Can not create Server Worker Thread ...\n");
}

CInlineOrder::CInlineProcess::~CInlineProcess()
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

void CInlineOrder::CInlineProcess::Release()
{
	pthread_mutex_lock(&mtex);
	for (__AOMAP__::const_iterator i = amap.begin(); i != amap.end(); ++i)
		if (i->second)
			delete i->second;
	amap.clear();
	imap.clear();
	pthread_mutex_unlock(&mtex);
}

OL_UINT64 CInlineOrder::CInlineProcess::GetAccumOBVolume(IDX_STOCK *ps, OL_UINT8 cmd, OL_UINT16 depth, OL_UINT32 &lPrice)
{
	OL_UINT64 totOBVolume = 0;
	IDX_ORDER_BOOK *pob = ps->orderBook;
	if (0 == depth)
		depth = 1;
	lPrice = 0;
	if (1 == cmd || 3 == cmd)	//	Sell
	{
		if (0 == pob->lBidDepth)
			return 0;
		if (depth > pob->lBidDepth)
			depth = pob->lBidDepth;
		for (OL_UINT16 i = 0; i < depth; ++i)
		{
			lPrice = pob->bid[i].lPrice / 100;
			totOBVolume += pob->bid[i].lVolume;
		}
	}
	else	//	Buy
	{
		if (0 == pob->lOfferDepth)
			return 0;
		if (depth > pob->lOfferDepth)
			depth = pob->lOfferDepth;
		for (OL_UINT16 i = 0; i < depth; ++i)
		{
			lPrice = pob->offer[i].lPrice / 100;
			totOBVolume += pob->offer[i].lVolume;
		}
	}
	return totOBVolume;
}

bool CInlineOrder::CInlineProcess::CheckOrder(INLINE_ORDER *o)
{
	if ((1 != o->status) || (0 == o->OVolume) || (0 == o->BelowExecRatio && 0 == o->UpperExecRatio))
	{
		journal->WriteLog("[CInlineOrder::CInlineProcess::CheckOrder] - Stock:%s.%s; Status:%u; OVol:%lu; BelowRatio:%ld; UpperRatio:%ld;\n",
			o->szStockCode, o->szBoard, o->status, o->OVolume, o->BelowExecRatio, o->UpperExecRatio);
		return true;
	}
	//if ((55 == pidxdata->idxState.fStatus) || ('d' == pidxdata->idxState.fStatus) || bHoliday)	//	End Sending Records
	//{
	//	journal->WriteLog("[CInlineOrder::CInlineProcess::CheckOrder] - Stock:%s.%s; Status:%u - %c;\n",
	//		o->szStockCode, o->szBoard, pidxdata->idxState.fStatus, pidxdata->idxState.fStatus);
	//	return false;
	//}
	if ((SEC_END_OF_SYSTEM_HOURS == pidxdata->idxState.fStatus) || (SEC_END_OF_MESSAGES == pidxdata->idxState.fStatus) || bHoliday)	//	End Sending Records
		return false;
	IDX_STOCK *ps = pidxdata->idxStock;
	OL_UINT32 lTotStk = pidxdata->idxState.lTotalStock;
	OL_INT32 ib = find_idx_board(o->szBoard), 
			 is = find_idx_stock(o->szStockCode, ps, lTotStk, SZ_IDX_STOCK);
	if (-1 == ib || -1 == is)
	{
		journal->WriteLog("[CInlineOrder::CInlineProcess::CheckOrder] - Can not Find Stock:%s.%s; ib:%ld; is:%ld;\n",
			o->szStockCode, o->szBoard, ib, is);
		return false;
	}
	IDX_STOCK_DATA *psd = &ps[is].stockData[ib];
	if (o->Flags & 0x01) //	IsNOW
		o->CapturedMarketVolume = o->TradedMarketVolume = psd->lTradedVolume;	//	Dalam lembar saham
	return false;
}

void CInlineOrder::CInlineProcess::AddOrder(INLINE_ORDER *o)
{
	if (NULL == o)
		return;
	char szStockCode[64], szT[4096];
	GetOrderDescriptions(szT, o);
	sprintf(szStockCode, "%s.%s", o->szStockCode, o->szBoard);
	pthread_mutex_lock(&mtex);
	__IOMAP__::const_iterator i = imap.find(o->szId);
	if (i == imap.end())
	{
		received->WriteLog("[Inline Order] - %s", szT);
		if (false == CheckOrder(o))
		{
			journal->WriteLog("[CInlineOrder::CInlineProcess::AddOrder] - Order:%s; Stock:%s; has been added to map.\n",
				o->szId, szStockCode);
			imap.insert(std::pair<std::string, INLINE_ORDER*>(o->szId, o));
			amap.insert(std::pair<std::string, INLINE_ORDER*>(szStockCode, o));
		}
		else
		{
			journal->WriteLog("[CInlineOrder::CInlineProcess::AddOrder] - Order:%s; Stock:%s; has been DELETED.\n",
				o->szId, szStockCode);
			delete o;
		}
	}
	else
	{
		received->WriteLog("[Inline Order][DUPLICATE] - %s", szT);
		delete o; o = NULL;
	}
	pthread_mutex_unlock(&mtex);
}

void CInlineOrder::CInlineProcess::WithdrawOrder(const char *orderId)
{
	char szT[4096];
	pthread_mutex_lock(&mtex);
	__IOMAP__::iterator i = imap.find(orderId);
	if (i != imap.end())
	{
		INLINE_ORDER *o = i->second;
		GetOrderDescriptions(szT, o);
		imap.erase(i);
		char szStockCode[64] = {};
		sprintf(szStockCode, "%s.%s", o->szStockCode, o->szBoard);
		std::pair<__AOMAP__::iterator, __AOMAP__::iterator> iret = amap.equal_range(szStockCode);
		for (__AOMAP__::iterator j = iret.first; j != iret.second; ++j)
			if (0 == strcmp(j->second->szId, orderId))
			{
				amap.erase(j);
				delete o; o = NULL;
				received->WriteLog("[Inline Order][WITHDRAW] - %s", szT);
				break;
			}
	}
	pthread_mutex_unlock(&mtex);
}

void CInlineOrder::CInlineProcess::WriteExecutedOrderLog(INLINE_ORDER *o)
{
	char szT[4096];
	GetOrderDescriptions(szT, o);
	executed->WriteLog("CnnId:%s; %s", szIdentity, szT);
}

void CInlineOrder::CInlineProcess::ComposeOrderPackage(INLINE_ORDER *o)
{
	CTEOrderPackage *opkg = new CTEOrderPackage();
	opkg->AllocateBuffer(224 + ORDER_REF_LEN);
	if (0 == strcmp(RG_BOARD, o->szBoard))
		opkg->SetPackageId(PK_TE_NEW_REGULAR_ORDER);
	else if (0 == strcmp(TN_BOARD, o->szBoard))
		opkg->SetPackageId(PK_TE_NEW_CASH_ORDER);
	else if (0 == strcmp(ADV_BOARD, o->szBoard))
		opkg->SetPackageId(PK_TE_NEW_ADVERTISEMENT_ORDER);
	opkg->SetServerFd(0);
	opkg->AddString(o->szCustId, 8);
	opkg->AddString("", 25);	//	PIN
	opkg->AddByte(SRC_LIMIT_ORDER);	//	Market Inline Order
	opkg->AddByte(o->cmd);
	opkg->AddByte(0);	//	Day Session
	opkg->AddByte(0);	//	Randomized
	opkg->AddDWord(1);	//	N Split Order
	opkg->AddDWord(o->LastExecPrice);	//	Price
	opkg->AddInt64(o->LastExecVolume);	//	Volume
	opkg->AddString(o->szStockCode, 30);
	opkg->AddString(o->szCustId, 8);
	opkg->AddString(o->szId, 30);
	opkg->AddString(o->szId, ORDER_REF_LEN);
	opkg->AddString(szIdentity, 25);
	opkg->AddByte(0);	//	bPriceStep
	opkg->AddDWord(0);	//	Price Step
	opkg->AddByte(0);	//	bAutoPriceStep
	opkg->AddDWord(0);		//	Auto Price Step
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char*)*opkg, opkg->GetPackageLength(), 0);
	WriteExecutedOrderLog(o);
	pthread_mutex_unlock(&ztex);
	delete opkg; opkg = NULL;
	o->TVolume += o->LastExecVolume;
	o->ExecVolume += o->LastExecVolume;
	o->LastExecVolume = o->LastExecPrice = 0;
}

void CInlineOrder::CInlineProcess::SendHeartBeat()
{
	CTEOrderPackage *opkg = new CTEOrderPackage(0, PK_HTS_HEART_BEAT);
	if (NULL == opkg)
		return;
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char *)*opkg, opkg->GetPackageLength(), 0);
	pthread_mutex_unlock(&ztex);
	delete opkg; opkg = NULL;
}

void CInlineOrder::CInlineProcess::RequestResendAllInlineOrder()
{
	CTEOrderPackage *opkg = new CTEOrderPackage(0, PK_ZMQ_REQ_ALL_OPEN_INLINE);
	if (NULL == opkg)
		return;
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char *)*opkg, opkg->GetPackageLength(), 0);
	pthread_mutex_unlock(&ztex);
	delete opkg; opkg = NULL;
}

void CInlineOrder::CInlineProcess::SendUpdateInlineState(INLINE_ORDER *o)
{
	CTEOrderPackage *opkg = new CTEOrderPackage(59, PK_ZMQ_UPDATE_INLINE_ORDER);
	//opkg->AddString(o->szId, 12);
	//opkg->AddString(o->szStockCode, 30);
	//opkg->AddString(o->szBoard, 5);
	//opkg->AddDWord(o->TPrice);
	//opkg->AddDWord(o->StopPrice);
	//opkg->AddDWord(o->ExecPrice);
	pthread_mutex_lock(&ztex);
	zmq_send(client, (const char *)*opkg, opkg->GetPackageLength(), 0);
	pthread_mutex_unlock(&ztex);
	delete opkg; opkg = NULL;
}

OL_UINT32 CInlineOrder::CInlineProcess::ProcessData(QUOTE *q)
{
	if (NULL == q || bHoliday)
	{
		journal->WriteLog("[CInlineOrder::CInlineProcess::ProcessData] - received NULL Quote Data or Holiday.\n");
		return 0;
	}
	char stockCode[32] = {}, board[8] = {},
		*ch = strchr(q->szStockCode, '.');
	if (NULL == ch)
	{
		strcpy(board, IDX_RG_BOARD);
		strcpy(stockCode, q->szStockCode);
	}
	else
	{
		strcpy(board, ch + 1);
		strncpy(stockCode, q->szStockCode, ch - q->szStockCode);
	}
	OL_UINT32 nOrder = 0, lPrice = 0,
		lTotStk = pidxdata->idxState.lTotalStock;
	IDX_STOCK *ps = pidxdata->idxStock;
	pthread_mutex_lock(&mtex);
	std::pair<__AOMAP__::iterator, __AOMAP__::iterator> iret = amap.equal_range(q->szStockCode);
	journal->WriteLog("[CInlineOrder::CInlineProcess::ProcessData] - Process Stock:%s; pairSize:%lu;\n", q->szStockCode, amap.count(q->szStockCode));
	for (__AOMAP__::iterator i = iret.first; i != iret.second; ++i)
	{
		INLINE_ORDER *o = i->second;
		if (NULL == o)
		{
			journal->WriteLog("[CInlineOrder::CInlineProcess::ProcessData] - received NULL Inline Order Data.\n");
			continue;
		}
		if (1 != o->status)
		{
			journal->WriteLog("[CInlineOrder::CInlineProcess::ProcessData] - OrderId:%s; Status:%u.\n", o->szId, o->status);
			continue;
		}
		OL_INT32 ib = find_idx_board(board), 
				 is = find_idx_stock(stockCode, ps, lTotStk, SZ_IDX_STOCK);
		if (-1 == ib || -1 == is)
		{
			journal->WriteLog("Can not found Stock:%s; Board:%s; is:%ld; ib:%ld;\n", stockCode, board, is, ib);
			continue;
		}
		IDX_STOCK_DATA *psd = &ps[is].stockData[ib];
		OL_INT64 tradedVolume = psd->lTradedVolume,	//	Dalam lembar saham
				inlineVolume = tradedVolume - (o->CapturedMarketVolume + o->TVolume);
		if (inlineVolume <= 0)
		{
			journal->WriteLog("Stock:%s.%s; MktTradedVol:%ld; inlineVol:%ld; TVol:%ld;\n",
				stockCode, board, tradedVolume, inlineVolume, o->TVolume);
			continue;
		}
		if (o->LimitVolume)
			if (inlineVolume <= (OL_INT64)o->LimitVolume)
			{
				journal->WriteLog("Stock:%s.%s; inlineVol:%ld; LimitVol:%ld;\n",
					stockCode, board, inlineVolume, o->LimitVolume);
				continue;
			}
		OL_UINT64 execVolume = 0,
			obAccumVol = GetAccumOBVolume(ps + is, o->cmd, orderBookDepth, lPrice);	//	Dalam lembar saham
		//journal->WriteLog("[ProcessData] - StockCode:%s; AccumVol:%lu; Price:%lu; inlineVol:%lu;\n", obAccumVol, lPrice, inlineVol);
		if (0 == obAccumVol || 0 == lPrice)
		{
			journal->WriteLog("Stock:%s.%s; lPrice:%ld; ovAccumVol:%ld;\n", stockCode, board, lPrice, obAccumVol);
			continue;
		}
		if ((0 == o->cmd) || (2 == o->cmd))	//	Limit Buy
		{
			OL_UINT32 bestPrice = psd->lBestOfferPrice / 100;
			if (0 == bestPrice)
			{
				journal->WriteLog("Stock:%s.%s; bestPrice:%ld;\n", stockCode, board, bestPrice);
				continue;
			}
			if (0 == o->LimitPrice)
				execVolume = inlineVolume * o->BelowExecRatio / 100;
			else
				execVolume = (bestPrice <= o->LimitPrice) ?
					(inlineVolume * o->BelowExecRatio / 100) : 
					(inlineVolume * o->UpperExecRatio / 100);
		}
		else	//	Limit Sell
		{
			OL_UINT32 bestPrice = psd->lBestBidPrice / 100;
			if (0 == bestPrice)
			{
				journal->WriteLog("Stock:%s.%s; bestPrice:%ld;\n", stockCode, board, bestPrice);
				continue;
			}
			if (0 == o->LimitPrice)
				execVolume = inlineVolume * o->UpperExecRatio / 100;
			else
				execVolume = (bestPrice <= o->LimitPrice) ?
					(inlineVolume * o->BelowExecRatio / 100) : 
					(inlineVolume * o->UpperExecRatio / 100);
		}
		OL_INT64 RVolume = o->OVolume - o->TVolume;
		execVolume -= execVolume % SHARES_LOT;
		if (execVolume <= 0)
		{
			journal->WriteLog("Stock:%s.%s; execVolume:%ld;\n", stockCode, board, execVolume);
			continue;
		}
		if (o->LimitVolume)
			if (execVolume <= o->LimitVolume)
			{
				journal->WriteLog("Stock:%s.%s; execVolume:%ld; LimitVolume:%ld;\n", stockCode, board, execVolume, o->LimitVolume);
				continue;
			}
		if (RVolume <= 0)
		{
			o->status = (o->TVolume > 0) ? 2 : 0;
				journal->WriteLog("Stock:%s.%s; RVolume:%ld;\n", stockCode, board, RVolume);
			continue;
		}
		if (RVolume < (int64_t)execVolume)
			execVolume = RVolume;
		//if (execVolume > RVolume)
		//	execVolume = RVolume;
		if (execVolume > obAccumVol)
			execVolume = obAccumVol;
		o->LastExecPrice = lPrice;
		o->LastExecVolume = execVolume;
		ComposeOrderPackage(o);
		o->CapturedMarketVolume = tradedVolume;
	}
	pthread_mutex_unlock(&mtex);
	return nOrder;
}

void CInlineOrder::CInlineProcess::CleanupAllInlineOrder()
{
	static unsigned int counter = 0;
	static pthread_mutex_t ctex = PTHREAD_MUTEX_INITIALIZER;
	journal->WriteLog("[%s] - Receiving Cleanup Inline Order Signal, cleaning up all Inline Orders ...\n", szIdentity);
	pthread_mutex_lock(&mtex);
	for (__AOMAP__::iterator i = amap.begin(); i != amap.end(); ++i)
		delete i->second;
	amap.clear();
	imap.clear();
	pthread_mutex_unlock(&mtex);
	pthread_mutex_lock(&ctex);
	if (26 == ++counter)
	{
		counter = 0;
		journal->WriteLog("[%s] - End of cleanup Inline Order Process, requesting all open Inline Orders ...\n", szIdentity);
		sleep(30);
		RequestResendAllInlineOrder();
	}
	pthread_mutex_unlock(&ctex);
}
