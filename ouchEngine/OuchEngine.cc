#include <zmq.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "header/OuchEngine.h"
#include "header/config.h"
#include "header/miscfunc.h"
#include "header/TEOrderPackage.h"
#include "header/ClientConverter.h"

#define ZMQ_AOPROXY_IDENTITY "ORDER_ENGINE"

#define CFG_SERVER_SECTION "server"
#define CFG_SERVER_LOT_SIZE "lot_size"
#define CFG_SERVER_REQ_RESEND_ORDERS "request_resend_orders"

#define CFG_ZMQ_SECTION "zmq"
#define CFG_ZMQ_THREAD_COUNT "thread_count"
#define CFG_ZMQ_FRONTEND_THREAD_COUNT "frontend_thread_count"
#define CFG_ZMQ_AOPROXY_FRONTEND "AOProxy_frontend"
#define CFG_ZMQ_PUBLISH_ADDRESS "itch_publisher_address"

#define CFG_FILENAME "ouchEngine.cfg"
#define CFG_OUCH_SECTION "OUCH"
#define CFG_OUCH_PROCESS_THREAD_COUNT "ouch_process_thread_count"
#define CFG_OUCH_XX_SECTION "OUCH_%02u"
#define CFG_OUCH_XX_SERVER_XX_SECTION "OUCH_%02u_SERVER_%02u"

#define CFG_OUCH_INSTANCES "ouch_instances"
#define CFG_OUCH_LOG_MESSAGE "log_ouch_messages"

#define CFG_OUCH_SERVERS "ouch_servers"

#define CFG_UID "uid"
#define CFG_PWD "pwd"
#define CFG_SERVER_PORT "port"
#define CFG_SERVER_ADDRESS "address"

#define OUCH_ENGINE_LOG_FILE "ouchEngine.log"
#define OUCH_IN_MSG_LOG_FILE "ouchInMsg_%u.log"
#define OUCH_OUT_MSG_LOG_FILE "ouchOutMsg_%u.log"
#define OUCH_SENT_ORDER_LOG_FILE "ouchSent_%u.log"
#define OUCH_ORDER_LOG_FILE "ouchOrder_%u.log"
#define OUCH_AMEND_LOG_FILE "ouchAmend_%u.log"
#define OUCH_WITHDRAW_LOG_FILE "ouchWithdraw_%u.log"

const uint32_t USLEEP_DELAY = 100000;

COuchEngine::COuchNode::COuchNode(COuchEngine *parent) : curServer(0), nThreadsRunning(0), isThreadRunning(false), isConnectionActive(false), pparent(parent)
{
	pthread_mutex_init(&mutex, NULL);
	memset(&ouchCfg, 0x00, sizeof(SOUPBIN_CFG_DATA));
}

COuchEngine::COuchNode::~COuchNode()
{
	DisconnectFromOuchServer();
	pthread_mutex_destroy(&mutex);
}

bool COuchEngine::COuchNode::isOuchUserId(const char *szId)
{
	return (0 == strcmp(szId, ouchCfg.szUserId));
}

void COuchEngine::COuchNode::SetSoupBinCfgData(SOUPBIN_CFG_DATA *o)
{
	memcpy(&ouchCfg, o, sizeof(SOUPBIN_CFG_DATA));
}

void COuchEngine::COuchNode::SubscribeOUCHTradingLimitMessage()
{
	{
		COuchTradingLimitRequest o(TL_TYPE_POOL);
		ouchSocket.WriteBlock(o, o.GetPackageLength());
	}
	{
		COuchTradingLimitRequest o(TL_TYPE_EQUITY);
		ouchSocket.WriteBlock(o, o.GetPackageLength());
	}
	{
		COuchTradingLimitRequest o(TL_TYPE_DERIVATIVE);
		ouchSocket.WriteBlock(o, o.GetPackageLength());
	}
}

long COuchEngine::COuchNode::SendOuchClientHeartBeat()
{
	CSoupBin o(0, SB_PKG_CLIENT_HEART_BEAT);
	return ouchSocket.WriteBlock(o, o.GetPackageLength());
}

int COuchEngine::COuchNode::SendIDXOUCHLoginPackage(const char *uid, const char *pwd, unsigned long lseq)
{
	COuch pkg;
	pkg.ComposeLoginRequest(uid, pwd, "          ", lseq);
	pparent->journal.WriteLog("Sending Login Package, Uid : %s; Seq : %lu;\n", uid, lseq);
	return ouchSocket.WriteBlock(pkg, pkg.GetPackageLength());
}

uint16_t COuchEngine::COuchNode::GetCurrentActiveServerAddr(char *szAddr)
{
	uint16_t port = ouchCfg.serverPort[curServer];
	strcpy(szAddr, ouchCfg.szServerIPAddress[curServer]);
	if (++curServer >= ouchCfg.servers)
		curServer = 0;
	return port;
}

void COuchEngine::COuchNode::RequeueOuchMessage(CSoupBin *msg, bool isNonImmediate, bool isPreop, bool isCallAuction, __OUCH_OUTQ__::QUEUE_PRIOR prior)
{
	if (NULL == msg)
		return;
	if (isNonImmediate)
	{
		if (isPreop)
			pparent->outgoingQ.AddPreoptoQueue(msg, prior);
		else if (isCallAuction)
			pparent->callOutgoingQ.AddtoQueue(msg, prior);
		else
			pparent->outgoingQ.AddtoQueue(msg, prior);
	}
	else
		pparent->outgoingQ.AddtoQueue(msg, prior);
}

bool COuchEngine::COuchNode::SendOuchEnterOrderMessage(COuchEnterOrder *o, bool isNonImmediate, bool ispreop, bool isCallAuction)
{
	if (0 == o->GetOrderBookId())
		o->SetOrderbookId(pparent->orderBookDir.GetOrderBookId(o->GetStockCode()));
	uint32_t orderToken = 0;
	uint16_t pkgLen = o->GetPackageLength();
	const char *psz = *o,
			   *orderId = o->GetOrderId();
	pthread_mutex_lock(&mutex);
	pparent->tokenMap.RegisterToken(o->SetOrderToken(orderToken = pparent->GetNextTokenId()), orderId);
	long nWrite = ouchSocket.WriteBlock(psz, pkgLen);
	pthread_mutex_unlock(&mutex);
	switch (nWrite)
	{
	case 0:
		pparent->journal.WriteLog("[COuchEngine::COuchNode::SendOuchEnterOrderMessage][Id:%s] - OUCH Connection closed.\n", ouchCfg.szUserId);
		break;
	case SOCKET_ERROR:
		pparent->PrintErrNoMessage(errno, "[COuchEngine::COuchNode::SendOuchEnterOrderMessage]");
		break;
	default:
		if (nWrite == pkgLen)
		{
			pparent->ouchSentJournal.WriteLog("%s|%s|%u|\r\n", ouchCfg.szUserId, orderId, orderToken);
			if (pparent->oConfig.ouchConfig.logOuchMessages)
			{
				char szDesc[2048];
				o->GetDescriptions(szDesc);
				pparent->ouchOutJournal.WriteOuchOutLog(o->GetMessageType(), ouchCfg.szUserId, szDesc, psz, pkgLen);
			}
			delete o;
			o = NULL;
			return true;
		}
		pparent->journal.WriteLog("[COuchEngine::COuchNode::SendOuchEnterOrderMessage][Id:%s] - Incomplete write operation.\n", ouchCfg.szUserId);
		break;
	};
	COuchEngine::COuchNode::RequeueOuchMessage(o, isNonImmediate, ispreop, isCallAuction);
	return false;
}

bool COuchEngine::COuchNode::SendOuchReplaceOrderMessage(COuchReplaceOrder *o, bool isNonImmediate, bool ispreop, bool isCallAuction)
{
	uint32_t newOrderToken = 0;
	uint16_t pkgLen = o->GetPackageLength();
	const char *psz = *o,
			   *orderId = o->GetOrderId();
	pthread_mutex_lock(&mutex);
	pparent->tokenMap.RegisterToken(o->SetNewOrderToken(newOrderToken = pparent->GetNextTokenId()), orderId);
	long nWrite = ouchSocket.WriteBlock(psz, pkgLen);
	pthread_mutex_unlock(&mutex);
	switch (nWrite)
	{
	case 0:
		pparent->journal.WriteLog("[COuchEngine::COuchNode::SendOuchReplaceOrderMessage][Id:%s] - OUCH Connection closed.\n", ouchCfg.szUserId);
		break;
	case SOCKET_ERROR:
		pparent->PrintErrNoMessage(errno, "[COuchEngine::COuchNode::SendOuchReplaceOrderMessage]");
		break;
	default:
		if (nWrite == pkgLen)
		{
			pparent->ouchSentJournal.WriteLog("%s|%s|%u|%s|\r\n", ouchCfg.szUserId, o->GetAmendId(), newOrderToken, orderId);
			if (pparent->oConfig.ouchConfig.logOuchMessages)
			{
				char szDesc[2048];
				o->GetDescriptions(szDesc);
				pparent->ouchOutJournal.WriteOuchOutLog(o->GetMessageType(), ouchCfg.szUserId, szDesc, psz, pkgLen);
			}
			delete o;
			o = NULL;
			return true;
		}
		pparent->journal.WriteLog("[COuchEngine::COuchNode::SendOuchReplaceOrderMessage][Id:%s] - Incomplete write operation.\n", ouchCfg.szUserId);
		break;
	};
	RequeueOuchMessage(o, isNonImmediate, ispreop, isCallAuction);
	return false;
}

bool COuchEngine::COuchNode::SendOuchCancelOrderMessage(COuchCancelOrder *o, bool isNonImmediate, bool ispreop)
{
	const char *psz = *o;
	uint16_t pkgLen = o->GetPackageLength();
	long nWrite = ouchSocket.WriteBlock(psz, pkgLen);
	switch (nWrite)
	{
	case 0:
		pparent->journal.WriteLog("[COuchEngine::COuchNode::SendOuchCancelOrderMessage][Id:%s] - OUCH Connection closed.\n", ouchCfg.szUserId);
		break;
	case -1:
		pparent->PrintErrNoMessage(errno, "[COuchEngine::COuchNode::SendOuchCancelOrderMessage]");
		break;
	default:
		if (nWrite == pkgLen)
		{
			pparent->ouchSentJournal.WriteLog("%s|%s|%u|%s|\r\n",
											  ouchCfg.szUserId, o->GetWithdrawId(), o->GetOrderToken(), o->GetOrderId());
			if (pparent->oConfig.ouchConfig.logOuchMessages)
			{
				char szDesc[2048];
				o->GetDescriptions(szDesc);
				pparent->ouchOutJournal.WriteOuchOutLog(o->GetMessageType(), ouchCfg.szUserId, szDesc, psz, pkgLen);
			}
			delete o;
			o = NULL;
			return true;
		}
		pparent->journal.WriteLog("[COuchEngine::COuchNode::SendOuchCancelOrderMessage][Id:%s] - Incomplete write operation.\n", ouchCfg.szUserId);
		break;
	};
	RequeueOuchMessage(o, isNonImmediate, ispreop, false);
	return false;
}

void COuchEngine::COuchNode::ConnectToOuchServer()
{
	if (isThreadRunning)
		return;
	pthread_t tid = 0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, (void *(*)(void *))OuchClientThread, (void *)this);
	pthread_attr_destroy(&attr);
}

void COuchEngine::COuchNode::DisconnectFromOuchServer()
{
	if (!isThreadRunning)
		return;
	isThreadRunning = false;
}

void COuchEngine::COuchNode::WaitForThreadsEnded()
{
	while (nThreadsRunning)
		sleep(1);
}

void COuchEngine::COuchNode::OuchSendThread(void *p)
{
	COuchNode *node = (COuchNode *)p;
	uint32_t nId = __sync_add_and_fetch(&node->nThreadsRunning, 1);
	bool ispreop = (0 == (nId % 2)),
		 iscallauction = (3 == nId);
	CTSession *session = iscallauction ? &(node->pparent->callsession) : ispreop ? &(node->pparent->psession)
																				 : &(node->pparent->ssession);
	if (iscallauction)
		node->pparent->journal.WriteLog("[COuchEngine::COuchNode::OuchSendThread][Id:%s] - Call Auction OuchSendThread has been started, nId : %lu;\n", node->ouchCfg.szUserId, nId);
	else
	{
		if (ispreop)
			node->pparent->journal.WriteLog("[COuchEngine::COuchNode::OuchSendThread][Id:%s] - OuchSendThread has been started, nId : %lu;\n", node->ouchCfg.szUserId, nId);
		else
			node->pparent->journal.WriteLog("[COuchEngine::COuchNode::OuchSendThread][Id:%s] - Regular Session OuchSendThread has been started, nId : %lu;\n", node->ouchCfg.szUserId, nId);
	}
	CSoupBin *msg = NULL;
	uint16_t lsession = 0;
	bool isNonImmediate = true;
	__OUCH_OUTQ__ *queue = iscallauction ? &node->pparent->callOutgoingQ : &node->pparent->outgoingQ;
	while (node->isThreadRunning)
	{
		if (!node->isConnectionActive)
		{
			usleep(USLEEP_DELAY);
			continue;
		}
		if ((isNonImmediate = (NULL == (msg = node->pparent->outgoingQ.GetImmediately()))))
		{
			if (0 == (lsession = session->CheckSession()))
			{
				session->Suspend(100);
				session->SynchronizeState();
				continue;
			}
			msg = ispreop ? queue->GetPreopFromQueue(0 != (lsession & TSES_SESSIONS)) : queue->GetfromQueue();
			if (NULL == msg)
			{
				usleep(USLEEP_DELAY); //	100 ms
				continue;
			}
		}
		switch (msg->GetMessageType())
		{
		case OUCH_TYPE_ENTER_ORDER_MSG:
			node->SendOuchEnterOrderMessage((COuchEnterOrder *)msg, isNonImmediate, ispreop, iscallauction);
			break;
		case OUCH_TYPE_REPLACED_ORDER_MSG:
		{
			COuchNode *instance = node->pparent->GetOuchInstanceByClientId(((COuchReplaceOrder *)msg)->GetClientId());
			if (instance)
			{
				if (!instance->SendOuchReplaceOrderMessage((COuchReplaceOrder *)msg, isNonImmediate, ispreop, iscallauction))
					usleep(USLEEP_DELAY);
			}
			else
			{
				node->RequeueOuchMessage(msg, isNonImmediate, ispreop, iscallauction);
				usleep(USLEEP_DELAY);
			}
		}
		break;
		case OUCH_TYPE_CANCEL_ORDER_MSG:
		{
			COuchNode *instance = node->pparent->GetOuchInstanceByClientId(((COuchCancelOrder *)msg)->GetClientId());
			if (instance)
			{
				if (!instance->SendOuchCancelOrderMessage((COuchCancelOrder *)msg, isNonImmediate, ispreop))
					usleep(USLEEP_DELAY);
			}
			else
			{
				node->RequeueOuchMessage(msg, isNonImmediate, ispreop, false);
				usleep(USLEEP_DELAY);
			}
		}
		break;
		}
	}
	if (iscallauction)
		node->pparent->journal.WriteLog("[COuchEngine::COuchNode::OuchSendThread][Id:%s] - Call Auction OuchSendThread has been ended, nId : %lu;\n", node->ouchCfg.szUserId, nId);
	else
	{
		if (ispreop)
			node->pparent->journal.WriteLog("[COuchEngine::COuchNode::OuchSendThread][Id:%s] - OuchSendThread has been ended, nId : %lu;\n", node->ouchCfg.szUserId, nId);
		else
			node->pparent->journal.WriteLog("[COuchEngine::COuchNode::OuchSendThread][Id:%s] - Regular Session OuchSendThread has been ended, nId : %lu;\n", node->ouchCfg.szUserId, nId);
	}
	__sync_sub_and_fetch(&node->nThreadsRunning, 1);
}

void COuchEngine::COuchNode::StartOuchSendThreads()
{
	if (!isThreadRunning)
		return;
	pthread_t tid = 0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, (void *(*)(void *))OuchSendThread, (void *)this);
	pthread_attr_destroy(&attr);
}

void COuchEngine::COuchNode::OuchClientThread(void *p)
{
	long lread = 0;
	uint16_t port;
	char szAddr[64];
	bool bClientRunning = false;
	COuchNode *node = (COuchNode *)p;
	uint32_t nSeq = 0, nRetryReconnect = 0,
			 nId = __sync_add_and_fetch(&node->nThreadsRunning, 1);
	node->isThreadRunning = true;
	node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - Starting OUCH Client Thread, nId : %lu;\n", node->ouchCfg.szUserId, nId);
	while (node->isThreadRunning)
	{
		if (0 == node->ouchSocket.Create())
		{
			node->isThreadRunning = false;
			node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - Can not create socket.\n", node->ouchCfg.szUserId);
			continue;
		}
		port = node->GetCurrentActiveServerAddr(szAddr);
		node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - Trying to connect OUCH Server %s:%u, nRetry : %u.\n", node->ouchCfg.szUserId, szAddr, port, nRetryReconnect);
		if (0 == node->ouchSocket.Connect(szAddr, port))
			node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - Can not connect to OUCH Server %s:%u\n", node->ouchCfg.szUserId, szAddr, port);
		else
		{
			node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Client has been connected to %s:%u\n", node->ouchCfg.szUserId, szAddr, port);
			lread = node->SendIDXOUCHLoginPackage(node->ouchCfg.szUserId, node->ouchCfg.szPassword, nSeq + 1);
			if (lread <= 0)
				node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - Send OUCH Login Package Failed.\n", node->ouchCfg.szUserId);
			else
			{
				bClientRunning = true;
				node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Login Package has been sent succesfully.\n", node->ouchCfg.szUserId);
				break;
			}
		}
		node->ouchSocket.CloseSocket();
		if (nRetryReconnect++ > 20)
		{
			if (node->pparent->bHoliday || GetCurrentTime() > 170000)
			{
				if (node->pparent->bHoliday)
					node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - Today is a Holiday, Can not connect to OUCH Server %s:%u.\n", node->ouchCfg.szUserId, szAddr, port);
				else
					node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - Not in Trading Time, Can not connect to OUCH Server %s:%u.\n", node->ouchCfg.szUserId, szAddr, port);
				node->isThreadRunning = false;
				node->pparent->StopItchSubscriber();
				node->pparent->StopOuchProcessThread();
			}
		}
		sleep(5);
	}
	if (bClientRunning)
		node->StartOuchSendThreads();
	//      --------------------------------------------------------------------------------------------------
	char *psz = NULL;
	time_t lastTime = time(NULL);
	uint16_t nSize = 0, nCount = 0;
	while (bClientRunning && node->isThreadRunning)
	{
		if (lread > 0)
			if (difftime(time(NULL), lastTime) >= 1)
			{
				lastTime = time(NULL);
				node->SendOuchClientHeartBeat();
				if (nCount++ > 15)
				{
					nCount = 0;
					node->SubscribeOUCHTradingLimitMessage();
				}
			}
		if ((lread = node->ouchSocket.Read((char *)&nSize, SZ_INT16, MSG_WAITALL, 1)) > 0)
		{
			if (SZ_INT16 == lread)
			{
				OL_UINT16 n = nSize;
				if ((nSize = ntohs(nSize)) > MAX_OUCH_PACKAGE_LENGTH)
				{
					node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Server Read Error, OUCH Package Length Oversized, nSize:%u, connection will be terminated, and try to reconnect.\n", node->ouchCfg.szUserId, nSize);
					lread = -1;
				}
				else
				{
					if (NULL == (psz = new char[nSize + SZ_INT16]))
					{
						node->isThreadRunning = false;
						node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - Memory Full, service will be terminated.\n", node->ouchCfg.szUserId);
						continue;
					}
					memcpy(psz, &n, SZ_INT16);
					lread = node->ouchSocket.ReadBlock(psz + SZ_INT16, nSize);
					if (lread != nSize)
					{
						node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Server Socket Error - lread : %ld - nSize : %ld, Incomplete Read OUCH Package Body Size, connection will be terminated, and try to reconnect.\n", node->ouchCfg.szUserId, lread, nSize);
						lread = -1;
					}
				}
			}
			else
			{
				node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Server Socket Error - lread:%ld, Incomplete Read ITCH Package Header Size, connection will be terminated, and try to reconnect.\n", node->ouchCfg.szUserId, lread);
				lread = -1;
			}
		}
		if (lread <= 0)
		{
			if (0 == lread)
				node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Socket Closed, try to reconnect.\n", node->ouchCfg.szUserId);
			else if (SOCKET_TIMEOUT == lread)
			{
				if (difftime(time(NULL), lastTime) >= 1)
				{
					lastTime = time(NULL);
					lread = node->SendOuchClientHeartBeat();
					if (nCount++ > 15)
					{
						nCount = 0;
						node->SubscribeOUCHTradingLimitMessage();
					}
				}
				continue;
			}
			else if (lread < 0)
				node->pparent->PrintErrNoMessage(errno, "OUCH SOCKET ERROR");
			node->isConnectionActive = false;
			node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Datafeed Socket has been disconnected at Seq %u, try to reconnect ...\n", node->ouchCfg.szUserId, nSeq);
			while (node->isThreadRunning)
			{
				node->ouchSocket.CloseSocket();
				sleep(5);
				if (0 == node->ouchSocket.Create())
				{
					node->isThreadRunning = false;
					node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - Can not create socket\n", node->ouchCfg.szUserId);
					continue;
				}
				port = node->GetCurrentActiveServerAddr(szAddr);
				node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - Trying to connect OUCH Server %s:%u.\n", node->ouchCfg.szUserId, szAddr, port);
				if (0 == node->ouchSocket.Connect(szAddr, port))
					node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - Can not connect to %s:%u\n", node->ouchCfg.szUserId, szAddr, port);
				else
				{
					node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Client has been connected to %s:%u\n", node->ouchCfg.szUserId, szAddr, port);
					lread = node->SendIDXOUCHLoginPackage(node->ouchCfg.szUserId, node->ouchCfg.szPassword, nSeq + 1);
					if (lread <= 0)
						node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - Send OUCH Login Package Failed.\n", node->ouchCfg.szUserId);
					else
					{
						node->isConnectionActive = bClientRunning = true;
						node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Login Package has been sent succesfully.\n", node->ouchCfg.szUserId);
						break;
					}
				}
			}
			continue;
		}
		switch (psz[2])
		{
		case SB_PKG_LOGIN_ACCEPTED:
		{
			char szSession[16] = {}, szSeq[24] = {};
			CSoupBin p(psz, nSize + SZ_INT16, NULL, false);
			p.GetString(10, szSession);
			p.GetString(20, szSeq);
			psz = NULL;
			node->isConnectionActive = true;
			node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Login Accepted With Session : %s; Seq : %s;\n", node->ouchCfg.szUserId, szSession, szSeq);
		}
		break;
		case SB_PKG_LOGIN_REJECTED:
			switch (psz[3])
			{
			case LOGIN_NOT_AUTHORIZED:
				node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Server Login Failed, Login not authorized.\n", node->ouchCfg.szUserId);
				break;
			case LOGIN_SESSION_UNAVAILABLE:
				node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Server Login Failed, Login Session Unavailable.\n", node->ouchCfg.szUserId);
				break;
			default:
				node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Server Login Failed, undefined.\n", node->ouchCfg.szUserId);
				break;
			}
			node->isThreadRunning = false;
			break;
		case SB_PKG_SERVER_HEARTBEAT:
			if (difftime(time(NULL), lastTime) >= 1)
			{
				lastTime = time(NULL);
				lread = node->SendOuchClientHeartBeat();
				if (nCount++ > 15)
				{
					nCount = 0;
					node->SubscribeOUCHTradingLimitMessage();
				}
			}
			break;
		case SB_PKG_SEQUENCED:
			++nSeq;
			// node->incomingQ.AddtoQueue(new COuch(psz, nSize + SZ_INT16, node->ouchCfg.szUserId, false));
			node->pparent->ouchIncomingQ.AddtoQueue(new COuch(psz, nSize + SZ_INT16, node->ouchCfg.szUserId, false));
			psz = NULL;
			break;
		case SB_PKG_UNSEQUENCED:
			// node->incomingQ.AddtoQueue(new COuch(psz, nSize + SZ_INT16, node->ouchCfg.szUserId, false));
			node->pparent->ouchIncomingQ.AddtoQueue(new COuch(psz, nSize + SZ_INT16, node->ouchCfg.szUserId, false));
			psz = NULL;
			break;
		case SB_PKG_END_OF_SESSION:
			node->isThreadRunning = false;
			node->pparent->isEndOfSession = true;
			node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Server Session Ended.\n", node->ouchCfg.szUserId);
			node->pparent->StopOuchProcessThread();
			break;
		case SB_PKG_DEBUG:
			node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Debug Package Received.\n", node->ouchCfg.szUserId);
			node->pparent->journal.Binary_Dump(psz, nSize + SZ_INT16);
			break;
		default:
			node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - Unknown OUCH Package Received, package will be dump.\n", node->ouchCfg.szUserId);
			node->pparent->journal.Binary_Dump(psz, nSize + SZ_INT16);
			break;
		}
		if (psz)
		{
			delete[] psz;
			psz = NULL;
		}
	}
	node->isThreadRunning = node->isConnectionActive = false;
	while (node->nThreadsRunning > 1)
		sleep(1);
	node->ouchSocket.CloseSocket();
	node->pparent->journal.WriteLog("[OuchClientThread][Id:%s] - OUCH Client Thread Ended, nId : %lu.\n", node->ouchCfg.szUserId, nId);
	__sync_sub_and_fetch(&node->nThreadsRunning, 1);
}

//-------------------------------------------------------------------------------

COuchEngine::COuchEngine() : isSchedulerRunning(false), isItchSubscRunning(false), isOuchProcessRunning(false), isTEThreadRunning(false), isEndOfSession(false), bHoliday(isHoliday(lCurDate, weekDay)), bReqResendOrders(true),
							 tid_schedulerThread(0), tid_itchSubscriber(0), lTodayScheduleState(0), lScheduleState(0), tokenId(0), SHARES_LOT(100), nZMQFrontEndThreadsRunning(0), nOuchProcessThreadsRunning(0),
							 psession(TSES_PREOPENING), ssession(TSES_SESSIONS), callsession(TSES_CALL_AUCTION_SESSIONS)
{
	ReloadJournalFile();
	sprintf(szCurDate, "%u", lCurDate % 1000000);
	journal.WriteLog("Init Current Date : %u - %s;\n", lCurDate, szCurDate);
	memset(&systemEventMsg, 0x00, sizeof(OUCH_SYSTEM_EVENT_MSG));
}

COuchEngine::~COuchEngine()
{
	ReleaseAllPackages();
	StopSchedulerThread();
	StopItchSubscriber();
	ReleaseOuchInstances();
}

void COuchEngine::ReleaseAllPackages()
{
	// for (size_t i = 0; i < oConfig.ouchConfig.vouch.size(); i++)
	// {
	// 	if (!oConfig.ouchConfig.vouch[i]->isAlive())
	// 		oConfig.ouchConfig.vouch[i]->ReleaseQueue();
	// }
	ouchIncomingQ.Release();
	outgoing_te_packages.Release();
	incoming_te_packages.Release();
	outgoingQ.Release();
	callOutgoingQ.Release();
	outgoingQ.ReleaseImmediately();
	callOutgoingQ.ReleaseImmediately();
}

uint32_t COuchEngine::ConvertTIF(uint8_t expiry)
{
	return (EXP_DAY_ORDER == expiry) ? TIF_DAY : (EXP_SES_ORDER == expiry) ? TIF_SESSION
																		   : TIF_IMMEDIATE;
}

uint8_t COuchEngine::ConvertOrderVerb(uint8_t command)
{
	return (CMD_BUY == command) ? VERB_BUY : (CMD_SELL == command)	   ? VERB_SELL
										 : (CMD_MARGIN_BUY == command) ? VERB_MARGIN_BUY
										 : (CMD_SHORT_SELL == command) ? VERB_SHORT_SELL
																	   : VERB_PRICE_STABILIZATION;
}

COuchEngine::COuchNode *COuchEngine::GetOuchInstanceByClientId(const char *szClientId)
{
	COuchNode *o = NULL;
	for (size_t i = 0; i < oConfig.ouchConfig.vouch.size(); i++)
		if (NULL != (o = oConfig.ouchConfig.vouch.at(i)))
			if (o->isAlive() && o->isOuchUserId(szClientId))
				return o;
	return NULL;
}

void COuchEngine::PrintErrNoMessage(int e, const char *s)
{
	if (EAGAIN == e)
		journal.WriteLog("%s - EAGAIN\n", s);
	else if (EWOULDBLOCK == e)
		journal.WriteLog("%s - EWOULDBLOCK\n", s);
	else if (EBADF == e)
		journal.WriteLog("%s - EBADF\n", s);
	else if (ECONNRESET == e)
		journal.WriteLog("%s - ECONNRESET\n", s);
	else if (EDESTADDRREQ == e)
		journal.WriteLog("%s - EDESTADDRREQ\n", s);
	else if (EMSGSIZE == e)
		journal.WriteLog("%s - EMSGSIZE\n", s);
	else if (EOPNOTSUPP == e)
		journal.WriteLog("%s - EOPNOTSUPP\n", s);
	else if (EPIPE == e)
		journal.WriteLog("%s - EPIPE\n", s);
	else if (EACCES == e)
		journal.WriteLog("%s - EACCES\n", s);
	else if (EIO == e)
		journal.WriteLog("%s - EIO\n", s);
	else if (ENETDOWN == e)
		journal.WriteLog("%s - ENETDOWN\n", s);
	else if (ENETUNREACH == e)
		journal.WriteLog("%s - ENETUNREACH\n", s);
	else if (ENOBUFS == e)
		journal.WriteLog("%s - ENOBUFS\n", s);
	else if (ETIMEDOUT == e)
		journal.WriteLog("%s - ETIMEDOUT\n", s);
	else if (ECONNREFUSED == e)
		journal.WriteLog("%s - ECONNREFUSED\n", s);
	else if (EFAULT == e)
		journal.WriteLog("%s - EFAULT\n", s);
	else if (EINTR == e)
		journal.WriteLog("%s - EINTR\n", s);
	else if (EINVAL == e)
		journal.WriteLog("%s - EINVAL\n", s);
	else if (ENOMEM == e)
		journal.WriteLog("%s - ENOMEM\n", s);
	else if (ENOTCONN == e)
		journal.WriteLog("%s - ENOTCONN\n", s);
	else if (ENOTSOCK == e)
		journal.WriteLog("%s - ENOTSOCK\n", s);
	else
		journal.WriteLog("%s - errno : %d\n", s, e);
}

bool COuchEngine::ReloadJournalFile()
{
	uint32_t curDate = GetCurrentDate();
	char szT[2048] = {}, szFileName[256] = {};

	GetLogDir(szT, OUCH_ENGINE_LOG_FILE);
	bool b = (-1 != journal.OpenJournal(szT));

	sprintf(szFileName, OUCH_ORDER_LOG_FILE, curDate);
	GetOUCHLogDir(szT, szFileName);
	b &= (-1 != orderJournal.OpenJournal(szT));

	sprintf(szFileName, OUCH_AMEND_LOG_FILE, curDate);
	GetOUCHLogDir(szT, szFileName);
	b &= (-1 != amendJournal.OpenJournal(szT));

	sprintf(szFileName, OUCH_WITHDRAW_LOG_FILE, curDate);
	GetOUCHLogDir(szT, szFileName);
	b &= (-1 != withdrawJournal.OpenJournal(szT));

	sprintf(szFileName, OUCH_OUT_MSG_LOG_FILE, curDate);
	GetOUCHLogDir(szT, szFileName);
	b &= (-1 != ouchOutJournal.OpenJournal(szT));

	sprintf(szFileName, OUCH_IN_MSG_LOG_FILE, curDate);
	GetOUCHLogDir(szT, szFileName);
	b &= (-1 != ouchInJournal.OpenJournal(szT));

	sprintf(szFileName, OUCH_SENT_ORDER_LOG_FILE, curDate);
	GetOUCHLogDir(szT, szFileName);
	b &= (-1 != ouchSentJournal.OpenJournal(szT));

	return b;
}

bool COuchEngine::ReloadSentOrderId()
{
	uint32_t curDate = GetCurrentDate();
	char szT[2048] = {}, szFileName[256] = {};
	sprintf(szFileName, OUCH_SENT_ORDER_LOG_FILE, curDate);
	GetOUCHLogDir(szT, szFileName);
	FILE *fp = fopen(szT, "r"); //  Open Empty File
	if (NULL == fp)
	{
		journal.WriteLog("[COuchEngine::ReloadSentOrderId] - Unable to open %s\n", szT);
		return false;
	}
	uint32_t token = 0, nDatas = 0;
	char szOrderId[32] = {}, szId[32] = {};
	journal.WriteLog("[COuchEngine::ReloadSentOrderId] - Reloading Sent OrderId and Registering Token Id.\n");
	//	OrderId/AmendId|TokenId|OrderId|
	while (NULL != fgets(szT, sizeof(szT), fp))
	{
		char *pc = strchr(szT, '|');
		if (NULL == pc)
			continue;
		sscanf(pc + 1, "%[^|]|%u|%[^|]|", szOrderId, &token, szId);
		if (token > tokenId)
			tokenId = token;
		order_check.insert(szOrderId);
		tokenMap.RegisterToken(token, (strlen(szId) > 0) ? szId : szOrderId);
		nDatas++;
	}
	fclose(fp);
	journal.WriteLog("[COuchEngine::ReloadSentOrderId] - Sent OrderId and Register Token Id has been successfully Reloaded With %u Datas.\n", nDatas);
	return true;
}

bool COuchEngine::LoadOrderBookDirectoryFromFile()
{
	char szT[2048];
	GetDataDatsDir(szT, ORDERBOOK_DIRECTORY_LIST_NAME);
	FILE *fp = fopen(szT, "r"); //  Open Empty File
	if (NULL == fp)
	{
		journal.WriteLog("[COuchEngine::LoadOrderBookDirectoryFromFile] - Unable to open %s\n", szT);
		return false;
	}
	journal.WriteLog("[COuchEngine::LoadOrderBookDirectoryFromFile] - Releasing Order Book Directory and Stock Datas.\n");
	rgsi.Release();
	tnsi.Release();
	orderBookDir.Release();
	uint32_t orderbookId, sectorId, sharesPerLot, nDatas = 0;
	char szStockCode[32], szBoard[8], szInstrument[32], szRemark2[64];
	journal.WriteLog("[COuchEngine::LoadOrderBookDirectoryFromFile] - Reloading Order Book Directory and Stock Datas.\n");
	//	orderbookId|SecurityCode|Group|Instrument|Remark2|SectorId|SharesPerLot|
	while (NULL != fgets(szT, sizeof(szT), fp))
	{
		sscanf(szT, "%u|%[^|]|%[^|]|%[^|]|%[^|]|%u|%u|",
			   &orderbookId, szStockCode, szBoard, szInstrument, szRemark2, &sectorId, &sharesPerLot);
		orderBookDir.SetOrderBook(orderbookId, szStockCode, szBoard);
		if (0 == strcmp(szBoard, BOARD_TN))
			tnsi.SetStockInfo(orderbookId, szStockCode, szBoard, szInstrument, szRemark2, sectorId, sharesPerLot);
		else
			rgsi.SetStockInfo(orderbookId, szStockCode, szBoard, szInstrument, szRemark2, sectorId, sharesPerLot);
		nDatas++;
	}
	fclose(fp);
	journal.WriteLog("[COuchEngine::LoadOrderBookDirectoryFromFile] - Order Book Directory and Stock Datas has been successfully reloaded with %u stock datas.\n", nDatas);
	return true;
}

bool COuchEngine::LoadConfigFile()
{
	CConfig cfg;
	char szFileName[1024] = {}, szSection[128] = {};
	GetConfigDir(szFileName, CFG_FILENAME);
	journal.WriteLog("Loading config file %s.\n", szFileName);
	if (0 == cfg.LoadConfig(szFileName))
	{
		journal.WriteLog("Can not open config File %s\n", szFileName);
		return false;
	}
	bReqResendOrders = cfg.GetValue(CFG_SERVER_SECTION, CFG_SERVER_REQ_RESEND_ORDERS, 1);
	journal.WriteLog("[LoadConfigFile] - Request Resend Orders is %s.\n", bReqResendOrders ? "Active" : "Inactive");
	callOutgoingQ.SetLotSize(
		outgoingQ.SetLotSize(SHARES_LOT = cfg.GetValue(CFG_SERVER_SECTION, CFG_SERVER_LOT_SIZE, 100)));
	journal.WriteLog("[LoadConfigFile] - Server LOT Size : %u;\n", SHARES_LOT);
	oConfig.ouchConfig.ouchInstances = cfg.GetValue(CFG_OUCH_SECTION, CFG_OUCH_INSTANCES, 1);
	oConfig.ouchConfig.logOuchMessages = cfg.GetValue(CFG_OUCH_SECTION, CFG_OUCH_LOG_MESSAGE, 1);
	oConfig.ouchConfig.ouchProcess_thread_count = cfg.GetValue(CFG_OUCH_SECTION, CFG_OUCH_PROCESS_THREAD_COUNT, 5);
	oConfig.zmqConfig.zmq_thread_count = cfg.GetValue(CFG_ZMQ_SECTION, CFG_ZMQ_THREAD_COUNT, 2);
	oConfig.zmqConfig.frontend_process_thread_count = cfg.GetValue(CFG_ZMQ_SECTION, CFG_ZMQ_FRONTEND_THREAD_COUNT, 5);
	cfg.GetValue(CFG_ZMQ_SECTION, CFG_ZMQ_AOPROXY_FRONTEND, "tcp://localhost:5580", oConfig.zmqConfig.zmq_AOProxy_Frontend, sizeof(oConfig.zmqConfig.zmq_AOProxy_Frontend));
	cfg.GetValue(CFG_ZMQ_SECTION, CFG_ZMQ_PUBLISH_ADDRESS, "tcp://localhost:5050", oConfig.zmqConfig.zmq_ITCH_Publisher_Address, sizeof(oConfig.zmqConfig.zmq_ITCH_Publisher_Address));
	SOUPBIN_CFG_DATA soupbinData;
	size_t vouch_size = oConfig.ouchConfig.vouch.size();
	for (uint16_t i = 0; i < oConfig.ouchConfig.ouchInstances; i++)
	{
		sprintf(szSection, CFG_OUCH_XX_SECTION, i);
		if (i >= vouch_size)
			oConfig.ouchConfig.vouch.push_back(new COuchNode(this));
		memset(&soupbinData, 0x00, sizeof(SOUPBIN_CFG_DATA));
		soupbinData.servers = cfg.GetValue(szSection, CFG_OUCH_SERVERS, 1);
		cfg.GetValue(szSection, CFG_UID, "uid", soupbinData.szUserId, sizeof(soupbinData.szUserId));
		cfg.GetValue(szSection, CFG_PWD, "pwd", soupbinData.szPassword, sizeof(soupbinData.szPassword));
		if (soupbinData.servers > 5)
			soupbinData.servers = 5;
		for (uint16_t j = 0; j < soupbinData.servers; j++)
		{
			sprintf(szSection, CFG_OUCH_XX_SERVER_XX_SECTION, i, j);
			soupbinData.serverPort[j] = cfg.GetValue(szSection, CFG_SERVER_PORT, 1);
			cfg.GetValue(szSection, CFG_SERVER_ADDRESS, "127.0.0.1", soupbinData.szServerIPAddress[j], sizeof(soupbinData.szServerIPAddress[j]));
		}
		if (!oConfig.ouchConfig.vouch[i]->isOuchThreadsRunning())
			oConfig.ouchConfig.vouch[i]->SetSoupBinCfgData(&soupbinData);
		else
			journal.WriteLog("OUCH Instance %s is Active and connected, can not reload config for this OUCH instance.\n", soupbinData.szUserId);
	}
	LoadTradingSchedule(oConfig.schedule);
	journal.WriteLog("Config file %s has been loaded.\n", szFileName);
	return true;
}

void COuchEngine::ReleaseOuchInstances()
{
	for (size_t i = 0; i < oConfig.ouchConfig.vouch.size(); i++)
		if (oConfig.ouchConfig.vouch[i]->isOuchThreadsRunning())
			oConfig.ouchConfig.vouch[i]->DisconnectFromOuchServer();
	for (size_t i = 0; i < oConfig.ouchConfig.vouch.size(); i++)
	{
		// if (oConfig.ouchConfig.vouch[i]->isOuchThreadsRunning())
		// 	oConfig.ouchConfig.vouch[i]->DisconnectFromOuchServer();
		// oConfig.ouchConfig.vouch[i]->ReleaseQueue();
		oConfig.ouchConfig.vouch[i]->WaitForThreadsEnded();
		delete oConfig.ouchConfig.vouch[i];
	}
	oConfig.ouchConfig.vouch.clear();
	oConfig.ouchConfig.ouchInstances = 0;
}

void COuchEngine::ConnectToOuchServers(int iSvr)
{
	if (iSvr < 0) //	Connect All Instances
	{
		for (size_t i = 0; i < oConfig.ouchConfig.vouch.size(); i++)
			if (!oConfig.ouchConfig.vouch[i]->isOuchThreadsRunning())
				oConfig.ouchConfig.vouch[i]->ConnectToOuchServer();
	}
	else
	{
		if (iSvr >= (int)oConfig.ouchConfig.vouch.size())
			return;
		if (!oConfig.ouchConfig.vouch[iSvr]->isOuchThreadsRunning())
			oConfig.ouchConfig.vouch[iSvr]->ConnectToOuchServer();
	}
	StartOuchProcessThread();
}

void COuchEngine::DisconnectFromOuchServers(int iSvr)
{
	if (iSvr < 0) //	Connect All Instances
	{
		for (size_t i = 0; i < oConfig.ouchConfig.vouch.size(); i++)
			if (oConfig.ouchConfig.vouch[i]->isOuchThreadsRunning())
				oConfig.ouchConfig.vouch[i]->DisconnectFromOuchServer();
		for (size_t i = 0; i < oConfig.ouchConfig.vouch.size(); i++)
			oConfig.ouchConfig.vouch[i]->WaitForThreadsEnded();
	}
	else
	{
		if (iSvr >= (int)oConfig.ouchConfig.vouch.size())
			return;
		if (oConfig.ouchConfig.vouch[iSvr]->isOuchThreadsRunning())
		{
			oConfig.ouchConfig.vouch[iSvr]->DisconnectFromOuchServer();
			oConfig.ouchConfig.vouch[iSvr]->WaitForThreadsEnded();
		}
	}
}

bool COuchEngine::isOuchInstanceAlive()
{
	for (size_t i = 0; i < oConfig.ouchConfig.vouch.size(); i++)
		if (oConfig.ouchConfig.vouch[i]->isAlive())
			return true;
	return false;
}

bool COuchEngine::IsAllOuchInstancesAlive()
{
	bool bRet = true;
	for (size_t i = 0; i < oConfig.ouchConfig.vouch.size(); i++)
		bRet &= oConfig.ouchConfig.vouch[i]->isAlive();
	return bRet;
}

bool COuchEngine::isAllOuchInstancesThreadRunning()
{
	bool bRet = true;
	for (size_t i = 0; i < oConfig.ouchConfig.vouch.size(); i++)
		bRet &= oConfig.ouchConfig.vouch[i]->isOuchThreadsRunning();
	return bRet;
}

void COuchEngine::StartSchedulerThread()
{
	if (isSchedulerRunning)
		return;
	pthread_create(&tid_schedulerThread, NULL, (void *(*)(void *))SchedulerThread, (void *)this);
}

void COuchEngine::StopSchedulerThread()
{
	if (!isSchedulerRunning)
		return;
	isSchedulerRunning = false;
	pthread_join(tid_schedulerThread, NULL);
	tid_schedulerThread = 0;
}

void COuchEngine::StartItchSubscriber()
{
	if (isItchSubscRunning)
		return;
	pthread_create(&tid_itchSubscriber, NULL, (void *(*)(void *))ZMQProcessItchSubscription, (void *)this);
}

void COuchEngine::StopItchSubscriber()
{
	if (!isItchSubscRunning)
		return;
	isItchSubscRunning = false;
	pthread_join(tid_itchSubscriber, NULL);
	tid_itchSubscriber = 0;
}

void COuchEngine::StartOuchProcessThread()
{
	if (isOuchProcessRunning)
		return;
	pthread_t tid = 0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	for (uint16_t i = 0; i < oConfig.ouchConfig.ouchProcess_thread_count; i++)
		pthread_create(&tid, &attr, (void *(*)(void *))ProcessOuchIncomingMessages, (void *)this);
	pthread_attr_destroy(&attr);
}

void COuchEngine::StopOuchProcessThread()
{
	if (!isOuchProcessRunning)
		return;
	isOuchProcessRunning = false;
	while (nOuchProcessThreadsRunning)
		usleep(USLEEP_DELAY);
}

void COuchEngine::StartTradeEngineThread()
{
	if (isTEThreadRunning)
		return;
	pthread_t tid = 0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, (void *(*)(void *))ZMQAOProxyFrontendThread, (void *)this);
	for (uint16_t i = 0; i < oConfig.zmqConfig.frontend_process_thread_count; i++)
		pthread_create(&tid, &attr, (void *(*)(void *))ProcessIncomingTEPackages, (void *)this);
	pthread_attr_destroy(&attr);
}

void COuchEngine::StopTradeEngineThread()
{
	if (!isTEThreadRunning)
		return;
	isTEThreadRunning = false;
	while (nZMQFrontEndThreadsRunning)
		usleep(USLEEP_DELAY);
}

void COuchEngine::SendStartSendingOrderPackage()
{
	CTEOrderPackage *o = new CTEOrderPackage(0, PK_RT_START_SENDING_ORDERS);
	o->SetErrorCode(bReqResendOrders ? 1 : 0);
	outgoing_te_packages.AddtoQueue(o, __TEORDER_QUEUE__::PRIOR_TAIL);
	// outgoing_te_packages.AddtoQueue(o, __TEORDER_QUEUE__::PRIOR_HEAD);
}

void COuchEngine::StartServices()
{
	StartSchedulerThread();
	StartItchSubscriber();
	ConnectToOuchServers(-1);
	StartTradeEngineThread();
	SendStartSendingOrderPackage();
}

void COuchEngine::StopServices()
{
	StopItchSubscriber();
	journal.WriteLog("[COuchEngine::StopServices] - Stop Itch Subscriber.\n");
	StopSchedulerThread();
	journal.WriteLog("[COuchEngine::StopServices] - Stop Scheduler Thread.\n");
	DisconnectFromOuchServers(-1);
	journal.WriteLog("[COuchEngine::StopServices] - Disconnect From OUCH Servers.\n");
	StopOuchProcessThread();
	journal.WriteLog("[COuchEngine::StopServices] - Stop OUCH Process Thread.\n");
	StopTradeEngineThread();
	journal.WriteLog("[COuchEngine::StopServices] - Stop TradeEngine Thread.\n");
}

void COuchEngine::WaitProcesses()
{
	sleep(1);
	while (nZMQFrontEndThreadsRunning)
		sleep(1);
}

void COuchEngine::CheckOUCHClientRunning()
{
	if (bHoliday || isEndOfSession)
		return;
	uint32_t lTime = GetCurrentTime();
	if (lTime < 80000 || lTime > 170000)
		return;
	// if (!IsAllOuchInstancesAlive())
	if (!isAllOuchInstancesThreadRunning())
	{
		if (!LoadConfigFile())
			return;
		StartItchSubscriber();
		ConnectToOuchServers(-1); //	Connect All Ouch Instances
	}
}

void COuchEngine::SchedulerThread(void *param)
{
	uint32_t lCurTime = 0, ldate = 0;
	COuchEngine *p = (COuchEngine *)param;
	// OL_UINT32 nId = __sync_add_and_fetch(&nConnected, 1);
	// journal.WriteLog("[SchedulerThread] - Starting SchedulerThread, nId : %lu.\n", nId);
	p->isSchedulerRunning = true;
	p->bHoliday = isHoliday(ldate, p->weekDay);
	time_t lastTime = time(NULL), curTime = time(NULL);
	p->journal.WriteLog("[COuchEngine::SchedulerThread] - Starting SchedulerThread.\n");
	while (p->isSchedulerRunning)
	{
		lCurTime = GetCurrentTimeMs(ldate); //	hhmmssxxx
		if (lCurTime < 100000)				// < 00:01:00.000
			p->bHoliday = isHoliday(ldate, p->weekDay);
		if (p->lCurDate < ldate) //	Do Init Day
		{
			//	p->bHoliday = isHoliday(ldate, p->weekDay);
			p->lCurDate = ldate;
			sprintf(p->szCurDate, "%u", ldate % 1000000);
			p->journal.WriteLog("[COuchEngine::SchedulerThread] - Begin of The Day, Initializing Trading Day, curDate : %s.\n", p->szCurDate);
			//	Init Day Routines
			p->isEndOfSession = false;
			p->order_check.clear();
			p->tokenMap.Release();
			p->lTodayScheduleState = p->lScheduleState = 0;
			p->ResetTokenId();
			p->ReleaseAllPackages();
			p->ReloadJournalFile();
			LoadTradingSchedule(p->oConfig.schedule);
			p->LoadOrderBookDirectoryFromFile();
			if (p->bHoliday)
				p->journal.WriteLog("[SchedulerThread] - Today is a holiday.\n");
		}
		if (p->oConfig.schedule[p->weekDay].before_market_open_active)
			if ((lCurTime / 1000) == (p->oConfig.schedule[p->weekDay].before_market_open / 1000))
			{
				if (0 == (p->lTodayScheduleState & TSES_BEFORE_MARKET_OPEN))
				{
					p->lScheduleState |= TSES_BEFORE_MARKET_OPEN;
					p->lTodayScheduleState |= TSES_BEFORE_MARKET_OPEN;
					p->journal.WriteLog("Doing Morning Jobs.\n");
					p->LoadOrderBookDirectoryFromFile();
					p->StartItchSubscriber();
					p->ConnectToOuchServers(-1);
				}
			}
		if (p->oConfig.schedule[p->weekDay].after_market_closed_active)
			if ((lCurTime / 1000) == (p->oConfig.schedule[p->weekDay].after_market_closed / 1000))
			{
				if (0 == (p->lTodayScheduleState & TSES_AFTER_MARKET_CLOSED))
				{
					p->lScheduleState |= TSES_AFTER_MARKET_CLOSED;
					p->lTodayScheduleState |= TSES_AFTER_MARKET_CLOSED;
					p->journal.WriteLog("Doing Market Closed jobs.\n");
				}
			}
		p->lScheduleState = p->SynchronizeTimeStateLite(lCurTime, p->lTodayScheduleState);
		if (difftime(curTime = time(NULL), lastTime) > 10)
		{
			lastTime = curTime;
			p->CheckOUCHClientRunning();
		}
		usleep(USLEEP_DELAY);
	}
	p->isSchedulerRunning = false;
	p->journal.WriteLog("[COuchEngine::SchedulerThread] - SchedulerThread Ended.\n");
	// journal.WriteLog("SchedulerThread Ended, nId : %lu.\n", nId);
	// __sync_sub_and_fetch(&nConnected, 1);
}

uint16_t COuchEngine::SynchronizeTimeStateLite(uint32_t lcurTime, uint16_t &lScheduleState)
{
	if (oConfig.schedule[weekDay].preopening_active)
	{
		if (lcurTime >= oConfig.schedule[weekDay].end_preopening)
		{
			if (lScheduleState & TSES_PREOP)
			{
				lScheduleState &= ~TSES_PREOP;
				psession.SetCurrentSession(TSES_BREAK);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - End of Preopening Trading Time.\n", lcurTime);
			}
		}
		else if ((lcurTime >= oConfig.schedule[weekDay].begin_preopening) && (lcurTime < oConfig.schedule[weekDay].end_preopening))
		{
			if (0 == (lScheduleState & TSES_PREOP))
			{
				lScheduleState |= TSES_PREOP;
				psession.SetCurrentSession(TSES_PREOP);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - Preopening Trading Time.\n", lcurTime);
			}
		}
	}
	if (oConfig.schedule[weekDay].session_1_active)
	{
		if (lcurTime > oConfig.schedule[weekDay].end_session_1)
		{
			if (lScheduleState & TSES_SESSION1)
			{
				lScheduleState &= ~TSES_SESSION1;
				psession.SetCurrentSession(
					ssession.SetCurrentSession(TSES_BREAK));
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - End of Session 1 Trading Time.\n", lcurTime);
			}
		}
		else if (lcurTime >= oConfig.schedule[weekDay].begin_session_1)
		{
			if (0 == (lScheduleState & TSES_SESSION1))
			{
				lScheduleState |= TSES_SESSION1;
				psession.SetCurrentSession(
					ssession.SetCurrentSession(TSES_SESSION1));
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - Session 1 Trading Time.\n", lcurTime);
			}
		}
	}
	if (oConfig.schedule[weekDay].session_2_active)
	{
		if (lcurTime > oConfig.schedule[weekDay].end_session_2)
		{
			if (lScheduleState & TSES_SESSION2)
			{
				lScheduleState &= ~TSES_SESSION2;
				psession.SetCurrentSession(
					ssession.SetCurrentSession(TSES_BREAK));
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - End of Session 2 Trading Time.\n", lcurTime);
			}
		}
		else if (lcurTime >= oConfig.schedule[weekDay].begin_session_2)
		{
			if (0 == (lScheduleState & TSES_SESSION2))
			{
				lScheduleState |= TSES_SESSION2;
				psession.SetCurrentSession(
					ssession.SetCurrentSession(TSES_SESSION2));
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - Session 2 Trading Time.\n", lcurTime);
			}
		}
	}
	if (oConfig.schedule[weekDay].post_trading_active)
	{
		if (lcurTime > oConfig.schedule[weekDay].end_post_trading)
		{
			if (lScheduleState & TSES_POST_TRADING)
			{
				lScheduleState &= ~TSES_POST_TRADING;
				psession.SetCurrentSession(
					ssession.SetCurrentSession(TSES_BREAK));
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - End of Post Trading Time.\n", lcurTime);
			}
		}
		else if (lcurTime >= oConfig.schedule[weekDay].begin_post_trading)
		{
			if (0 == (lScheduleState & TSES_POST_TRADING))
			{
				lScheduleState |= TSES_POST_TRADING;
				psession.SetCurrentSession(
					ssession.SetCurrentSession(TSES_POST_TRADING));
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - Post Trading Time.\n", lcurTime);
			}
		}
	}
	//	Call Auction Schedule
	if (oConfig.schedule[weekDay].call_auction_session_1_active)
	{
		if (lcurTime > oConfig.schedule[weekDay].end_call_auction_session_1)
		{
			if (lScheduleState & TSES_CALL_SESSION1)
			{
				lScheduleState &= ~TSES_CALL_SESSION1;
				callsession.SetCurrentSession(TSES_BREAK);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - End of Call Auction Session 1 Trading Time.\n", lcurTime);
			}
		}
		else if (lcurTime >= oConfig.schedule[weekDay].begin_call_auction_session_1)
		{
			if (0 == (lScheduleState & TSES_CALL_SESSION1))
			{
				lScheduleState |= TSES_CALL_SESSION1;
				callsession.SetCurrentSession(TSES_CALL_SESSION1);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - Call Auction Session 1 Trading Time.\n", lcurTime);
			}
		}
	}
	if (oConfig.schedule[weekDay].call_auction_session_2_active)
	{
		if (lcurTime > oConfig.schedule[weekDay].end_call_auction_session_2)
		{
			if (lScheduleState & TSES_CALL_SESSION2)
			{
				lScheduleState &= ~TSES_CALL_SESSION2;
				callsession.SetCurrentSession(TSES_BREAK);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - End of Call Auction Session 2 Trading Time.\n", lcurTime);
			}
		}
		else if (lcurTime >= oConfig.schedule[weekDay].begin_call_auction_session_2)
		{
			if (0 == (lScheduleState & TSES_CALL_SESSION2))
			{
				lScheduleState |= TSES_CALL_SESSION2;
				callsession.SetCurrentSession(TSES_CALL_SESSION2);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - Call Auction Session 2 Trading Time.\n", lcurTime);
			}
		}
	}
	if (oConfig.schedule[weekDay].call_auction_session_3_active)
	{
		if (lcurTime > oConfig.schedule[weekDay].end_call_auction_session_3)
		{
			if (lScheduleState & TSES_CALL_SESSION3)
			{
				lScheduleState &= ~TSES_CALL_SESSION3;
				callsession.SetCurrentSession(TSES_BREAK);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - End of Call Auction Session 3 Trading Time.\n", lcurTime);
			}
		}
		else if (lcurTime >= oConfig.schedule[weekDay].begin_call_auction_session_3)
		{
			if (0 == (lScheduleState & TSES_CALL_SESSION3))
			{
				lScheduleState |= TSES_CALL_SESSION3;
				callsession.SetCurrentSession(TSES_CALL_SESSION3);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - Call Auction Session 3 Trading Time.\n", lcurTime);
			}
		}
	}
	if (oConfig.schedule[weekDay].call_auction_session_4_active)
	{
		if (lcurTime > oConfig.schedule[weekDay].end_call_auction_session_4)
		{
			if (lScheduleState & TSES_CALL_SESSION4)
			{
				lScheduleState &= ~TSES_CALL_SESSION4;
				callsession.SetCurrentSession(TSES_BREAK);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - End of Call Auction Session 4 Trading Time.\n", lcurTime);
			}
		}
		else if (lcurTime >= oConfig.schedule[weekDay].begin_call_auction_session_4)
		{
			if (0 == (lScheduleState & TSES_CALL_SESSION4))
			{
				lScheduleState |= TSES_CALL_SESSION4;
				callsession.SetCurrentSession(TSES_CALL_SESSION4);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - Call Auction Session 4 Trading Time.\n", lcurTime);
			}
		}
	}
	if (oConfig.schedule[weekDay].call_auction_session_5_active)
	{
		if (lcurTime > oConfig.schedule[weekDay].end_call_auction_session_5)
		{
			if (lScheduleState & TSES_CALL_SESSION5)
			{
				lScheduleState &= ~TSES_CALL_SESSION5;
				callsession.SetCurrentSession(TSES_BREAK);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - End of Call Auction Session 5 Trading Time.\n", lcurTime);
			}
		}
		else if (lcurTime >= oConfig.schedule[weekDay].begin_call_auction_session_5)
		{
			if (0 == (lScheduleState & TSES_CALL_SESSION5))
			{
				lScheduleState |= TSES_CALL_SESSION5;
				callsession.SetCurrentSession(TSES_CALL_SESSION5);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - Call Auction Session 5 Trading Time.\n", lcurTime);
			}
		}
	}
	if (oConfig.schedule[weekDay].call_auction_post_trading_active)
	{
		if (lcurTime > oConfig.schedule[weekDay].end_call_auction_post_trading)
		{
			if (lScheduleState & TSES_CALL_POST_TRADING)
			{
				lScheduleState &= ~TSES_CALL_POST_TRADING;
				callsession.SetCurrentSession(TSES_BREAK);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - End of Call Auction Post Trading Time.\n", lcurTime);
			}
		}
		else if (lcurTime >= oConfig.schedule[weekDay].begin_call_auction_post_trading)
		{
			if (0 == (lScheduleState & TSES_CALL_POST_TRADING))
			{
				lScheduleState |= TSES_CALL_POST_TRADING;
				callsession.SetCurrentSession(TSES_CALL_POST_TRADING);
				journal.WriteLog("[COuchEngine::SynchronizeTimeStateLite] - CurTime:%u - Call Auction Post Trading Time.\n", lcurTime);
			}
		}
	}
	return lScheduleState;
}

void COuchEngine::ProcessItchSystemEventMsg(uint32_t timeStamp, ITCH_SYSTEM_EVENT_MSG &o)
{
	switch (o.eventCode)
	{
	case 'O': //	Start of ITCH Messages, This is the first message sent.
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - Start of ITCH Messages - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'S': //	Start of System Hours.
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - Start of ITCH System Hours - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'Q': //	Start of Market Hours. Trading Session. It indicates the Exchange is open and ready to start accepting orders.
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - Start of ITCH Market Hours - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'M': //	End of Market Hours. End of Trading Session.
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - End of ITCH Market Hours - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'P': //	Opening Auction Starts.
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - Opening ITCH Auction Starts - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'K': //	Closing Auction Starts.
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - Closing ITCH Auction Starts - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'V': //	Scheduled Auction Starts - a manually scheduled auction outside of defined trading schedule.
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - Scheduled ITCH Auction Starts - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'U': //	Scheduled Auction closes.
		if (0 == strcmp(o.szGroup, BOARD_RG))
		{
			if (0 == strcmp(o.szInstrument, INSTRUMENT_WATCH_CALL))
			{
				if (lScheduleState & TSES_CALL_SESSION5)
				{
					lScheduleState &= ~TSES_CALL_SESSION5;
					callsession.SetCurrentSession(TSES_BREAK);
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - End of Call Auction Session 5 Trading Time.\n", timeStamp);
				}
				else if (lScheduleState & TSES_CALL_SESSION4)
				{
					lScheduleState &= ~TSES_CALL_SESSION4;
					callsession.SetCurrentSession(TSES_BREAK);
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - End of Call Auction Session 4 Trading Time.\n", timeStamp);
				}
				else if (lScheduleState & TSES_CALL_SESSION3)
				{
					lScheduleState &= ~TSES_CALL_SESSION3;
					callsession.SetCurrentSession(TSES_BREAK);
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - End of Call Auction Session 3 Trading Time.\n", timeStamp);
				}
				else if (lScheduleState & TSES_CALL_SESSION2)
				{
					lScheduleState &= ~TSES_CALL_SESSION2;
					callsession.SetCurrentSession(TSES_BREAK);
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - End of Call Auction Session 2 Trading Time.\n", timeStamp);
				}
				else if (lScheduleState & TSES_CALL_SESSION1)
				{
					lScheduleState &= ~TSES_CALL_SESSION1;
					callsession.SetCurrentSession(TSES_BREAK);
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - End of Call Auction Session 1 Trading Time.\n", timeStamp);
				}
			}
			else if (0 == strcmp(o.szInstrument, INSTRUMENT_ORDI))
			{
				if (lScheduleState & TSES_SESSION2)
				{
					lScheduleState &= ~TSES_SESSION2;
					psession.SetCurrentSession(
						ssession.SetCurrentSession(TSES_BREAK));
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - End of Session 2 Trading Time.\n", timeStamp);
				}
				else if (lScheduleState & TSES_SESSION1)
				{
					lScheduleState &= ~TSES_SESSION1;
					psession.SetCurrentSession(
						ssession.SetCurrentSession(TSES_BREAK));
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - End of Session 1 Trading Time.\n", timeStamp);
				}
			}
		}
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - Scheduled ITCH Auction Closes - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'T': //	Start of Post Trading Session.
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - Start of ITCH Post Trading Session - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'A': //	Scheduled Break Starts.
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - Scheduled ITCH Break Starts - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'B': //	Schedule Break Ends.
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - Scheduled ITCH Break Ends - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'E': //	End of System Hours. It indicates that the Exchange is closed.
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - End of ITCH System Hours - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'C': //	End of Messages. Last Message Sent.
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - End of ITCH Messages - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'X': //	Trading Suspension
		if (0 == strcmp(o.szGroup, BOARD_RG))
		{
			if (0 == strcmp(o.szInstrument, INSTRUMENT_ORDI))
			{
				if (lScheduleState & TSES_SESSION2)
				{
					lScheduleState &= ~TSES_SESSION2;
					psession.SetCurrentSession(
						ssession.SetCurrentSession(TSES_BREAK));
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - Trading Suspension of Session 2 Trading Time.\n", timeStamp);
				}
				else if (lScheduleState & TSES_SESSION1)
				{
					lScheduleState &= ~TSES_SESSION1;
					psession.SetCurrentSession(
						ssession.SetCurrentSession(TSES_BREAK));
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - Trading Suspension of Session 1 Trading Time.\n", timeStamp);
				}
			}
			else if (0 == strcmp(o.szInstrument, INSTRUMENT_WATCH_CALL))
			{
				if (lScheduleState & TSES_CALL_SESSION5)
				{
					lScheduleState &= ~TSES_CALL_SESSION5;
					callsession.SetCurrentSession(TSES_BREAK);
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - Trading Suspension of Call Auction Session 5 Trading Time.\n", timeStamp);
				}
				else if (lScheduleState & TSES_CALL_SESSION4)
				{
					lScheduleState &= ~TSES_CALL_SESSION4;
					callsession.SetCurrentSession(TSES_BREAK);
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - Trading Suspension of Call Auction Session 4 Trading Time.\n", timeStamp);
				}
				else if (lScheduleState & TSES_CALL_SESSION3)
				{
					lScheduleState &= ~TSES_CALL_SESSION3;
					callsession.SetCurrentSession(TSES_BREAK);
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - Trading Suspension of Call Auction Session 3 Trading Time.\n", timeStamp);
				}
				else if (lScheduleState & TSES_CALL_SESSION2)
				{
					lScheduleState &= ~TSES_CALL_SESSION2;
					callsession.SetCurrentSession(TSES_BREAK);
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - Trading Suspension of Call Auction Session 2 Trading Time.\n", timeStamp);
				}
				else if (lScheduleState & TSES_CALL_SESSION1)
				{
					lScheduleState &= ~TSES_CALL_SESSION1;
					callsession.SetCurrentSession(TSES_BREAK);
					journal.WriteLog("[COuchEngine::ProcessItchSystemEventMsg] - CurTime:%u - Trading Suspension of Call Auction Session 1 Trading Time.\n", timeStamp);
				}
			}
		}
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - ITCH Trading Suspension - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	case 'Y': //	Trading Activation
		if (0 == strcmp(o.szGroup, BOARD_RG))
			SynchronizeTimeStateLite(GetCurrentTimeMs(), lScheduleState);
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - ITCH Trading Activation - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	default:
		journal.WriteLog("[ITCH_TYPE_SYSTEM_EVENT_MSG] - Undefined ITCH System Event Message - TimeStamp:%u; EventCode:%c; OrderBookId:%u; Group:%s; Instrument:%s;\n",
						 timeStamp, o.eventCode, o.orderbookId, o.szGroup, o.szInstrument);
		break;
	}
}

void COuchEngine::ProcessItchMessages(void *param)
{
	CItch *pkg = NULL;
	uint32_t itchTimeStamp = 0;
	COuchEngine *p = (COuchEngine *)param;
	p->journal.WriteLog("[COuchEngine::ProcessItchMessages] - ProcessItchMessages Thread started.\n");
	while (p->isItchSubscRunning)
	{
		if (NULL == (pkg = p->itchQ.GetfromQueue()))
		{
			usleep(USLEEP_DELAY);
			continue;
		}
		switch (pkg->GetMessageType())
		{
		case ITCH_TYPE_TIMESTAMP:
		{
			ITCH_TIME_STAMP o;
			pkg->GetTimeStamp(o);
			itchTimeStamp = o.seconds;
			// p->journal.WriteLog("[COuchEngine::ProcessItchMessages][ITCH_TYPE_TIMESTAMP] - MsgType:%c; Seconds : %u - %u;\n",
			//	pkg->GetMessageType(), itchTimeStamp = o.seconds, ConvertTimeStamp(o.seconds));
		}
		break;
		case ITCH_TYPE_SYSTEM_EVENT_MSG:
		{
			ITCH_SYSTEM_EVENT_MSG o;
			pkg->GetSystemEventMsg(o);
			uint32_t tStamp = ConvertTimeStamp(itchTimeStamp, o.timeStamp);
			p->ProcessItchSystemEventMsg(tStamp, o);
			if ('C' == o.eventCode) //	End of Messages. Last Message Sent.
			{
				p->isEndOfSession = true;
				p->isItchSubscRunning = false;
				p->journal.WriteLog("[COuchEngine::ProcessItchMessages][ITCH_TYPE_SYSTEM_EVENT_MSG] - End of ITCH Messages, Disconnect ITCH Subscriber.\n");
			}
		}
		break;
		}
		delete pkg;
		pkg = NULL;
	}
	p->journal.WriteLog("[COuchEngine::ProcessItchMessages] - ProcessItchMessages Thread ended.\n");
}

void COuchEngine::ZMQProcessItchSubscription(void *param)
{
	COuchEngine *p = (COuchEngine *)param;
	p->journal.WriteLog("[COuchEngine::ZMQProcessItchSubscription] - ZMQProcessItchSubscription Thread started.\n");
	void *context = zmq_ctx_new();
	if (NULL == context)
	{
		p->journal.WriteLog("[COuchEngine::ZMQProcessItchSubscription] - Can not create ZMQ Context.\n");
		p->isItchSubscRunning = false;
		return;
	}
	zmq_ctx_set(context, ZMQ_IO_THREADS, p->oConfig.zmqConfig.zmq_thread_count);
	void *zsock = zmq_socket(context, ZMQ_SUB);
	if (NULL == zsock)
	{
		p->journal.WriteLog("[COuchEngine::ZMQProcessItchSubscription] - Can not create zmq socket.\n");
		p->isItchSubscRunning = false;
		return;
	}
	int nRead = 1000;
	zmq_setsockopt(zsock, ZMQ_LINGER, &nRead, sizeof(nRead));	//	Linger Period of Socket Shutdown
	zmq_setsockopt(zsock, ZMQ_RCVTIMEO, &nRead, sizeof(nRead)); //	Receive time out 1 second
	if (0 != zmq_connect(zsock, p->oConfig.zmqConfig.zmq_ITCH_Publisher_Address))
	{
		p->journal.WriteLog("[COuchEngine::ZMQProcessItchSubscription] - Can not connect to zmq : %s\n", p->oConfig.zmqConfig.zmq_ITCH_Publisher_Address);
		p->isItchSubscRunning = false;
		return;
	}
	p->isItchSubscRunning = true;
	zmq_setsockopt(zsock, ZMQ_SUBSCRIBE, "", 0);
	nRead = 0;
	zmq_msg_t msg;
	pthread_t tid = 0;
	pthread_create(&tid, NULL, (void *(*)(void *))ProcessItchMessages, p);
	while (p->isItchSubscRunning)
	{
		if (-1 != nRead)
			zmq_msg_init(&msg);
		if (-1 == (nRead = zmq_msg_recv(&msg, zsock, 0)))
		{
			usleep(USLEEP_DELAY);
			continue;
		}
		p->itchQ.AddtoQueue(new CItch((char *)zmq_msg_data(&msg), nRead, true));
		zmq_msg_close(&msg);
		nRead = 0;
	}
	p->isItchSubscRunning = false;
	if (nRead > 0)
		zmq_msg_close(&msg);
	pthread_join(tid, NULL);
	zmq_close(zsock);
	zmq_ctx_destroy(context);
	p->journal.WriteLog("[COuchEngine::ZMQProcessItchSubscription] - ZMQProcessItchSubscription Thread ended.\n");
}

void COuchEngine::ProcessOuchIncomingMessages(void *param)
{
	COuch *pkg = NULL;
	COuchEngine *p = (COuchEngine *)param;
	p->isOuchProcessRunning = true;
	uint32_t nId = __sync_add_and_fetch(&p->nOuchProcessThreadsRunning, 1);
	p->journal.WriteLog("[COuchEngine::ProcessOuchIncomingMessages] - ProcessOuchIncomingMessages Thread started, nId : %lu;\n", nId);
	while (p->isOuchProcessRunning)
	{
		if (NULL == (pkg = p->ouchIncomingQ.GetfromQueue()))
		{
			usleep(USLEEP_DELAY);
			continue;
		}
		switch (pkg->GetMessageType())
		{
		case OUCH_TYPE_ACCEPTED_ORDER_MSG:
		{
			OUCH_ACCEPTED_ORDER_MSG o;
			pkg->GetOuchAcceptedOrderMsg(o);
			p->outgoing_te_packages.AddtoQueue(CClientConverter::ComposeOuchAcceptedOrderMsg(o));
			if (p->oConfig.ouchConfig.logOuchMessages)
				p->ouchInJournal.WriteOuchInLog(*pkg, pkg->GetPackageLength(), "[Id:%s] - [OUCH_ACCEPTED_ORDER_MSG] - TimeStamp:%lu; Type:%c; Verb:%c; Domicile:%c; OrderState:%c; OrderToken:%u; OrderBookId:%u; TimeInForce:%u; ClientId:%lu; "
																			   "Price:%u; Qty:%lu; OrderNumber:%lu; ExtOrderNumber:%lu; UserId:%s; BrokerRef:%s; InvId:%s; OrderSrc:%s;",
												pkg->GetIdentity(), ConvertNSTimeStamp(o.timeStamp), o.type, o.orderVerb, o.domicile, o.orderState, o.orderToken, o.orderbookId, o.timeInForce, o.clientId,
												o.price, o.quantity, o.orderNumber, o.extOrderNumber, o.szUserId, o.szBrokerRef, o.szInvestorId, o.szOrderSource);
		}
		break;
		case OUCH_TYPE_REPLACED_ORDER_MSG:
		{
			OUCH_REPLACED_ORDER_MSG o;
			pkg->GetOuchReplacedOrderMsg(o);
			p->outgoing_te_packages.AddtoQueue(CClientConverter::ComposeOuchReplacedOrderMsg(o));
			if (p->oConfig.ouchConfig.logOuchMessages)
				p->ouchInJournal.WriteOuchInLog(*pkg, pkg->GetPackageLength(), "[Id:%s] - [OUCH_TYPE_REPLACED_ORDER_MSG] - TimeStamp:%lu; Type:%c; Verb:%c; OrderState:%c; PrevOrderToken:%u; NewOrderToken:%u; OrderBookId:%u; TimeInForce:%u; "
																			   "Price:%u; Qty:%lu; OrderNumber:%lu; ExtOrderNumber:%lu; UserId:%s;",
												pkg->GetIdentity(), ConvertNSTimeStamp(o.timeStamp), o.type, o.orderVerb, o.orderState, o.prevOrderToken, o.replacementOrderToken, o.orderbookId, o.timeInForce,
												o.price, o.quantity, o.orderNumber, o.extOrderNumber, o.szUserId);
		}
		break;
		case OUCH_TYPE_CANCELED_ORDER_MSG:
		{
			OUCH_CANCELED_ORDER_MSG o;
			pkg->GetOuchCanceledOrderMsg(o);
			p->outgoing_te_packages.AddtoQueue(CClientConverter::ComposeOuchCanceledOrderMsg(o));
			if (p->oConfig.ouchConfig.logOuchMessages)
				p->ouchInJournal.WriteOuchInLog(*pkg, pkg->GetPackageLength(), "[Id:%s] - [OUCH_TYPE_CANCELED_ORDER_MSG] - TimeStamp:%lu; Type:%c; OrderToken:%u; Qty:%lu; Reason:%c; UserId:%s;",
												pkg->GetIdentity(), ConvertNSTimeStamp(o.timeStamp), o.type, o.orderToken, o.quantity, o.reason, o.szUserId);
		}
		break;
		case OUCH_TYPE_EXECUTED_ORDER_MSG:
		{
			OUCH_EXECUTED_ORDER_MSG o;
			pkg->GetOuchExecutedOrderMsg(o);
			p->tokenMap.GetOrderIdByToken(o.orderToken, o.szOrderId, sizeof(o.szOrderId));
			p->outgoing_te_packages.AddtoQueue(CClientConverter::ComposeOuchExecutedOrderMsg(o));
			if (p->oConfig.ouchConfig.logOuchMessages)
				p->ouchInJournal.WriteOuchInLog(*pkg, pkg->GetPackageLength(), "[Id:%s] - [OUCH_TYPE_EXECUTED_ORDER_MSG] - TimeStamp:%lu; Type:%c; OrderToken:%u - %s; Price:%u; Qty:%lu; LqFlag:%c; MatchNumber:%lu; CounterPartUid:%u; UserId:%s;",
												pkg->GetIdentity(), ConvertNSTimeStamp(o.timeStamp), o.type, o.orderToken, o.szOrderId, o.executedPrice, o.executedQty, o.liquidityFlag, o.matchNumber, o.counterPartyId, o.szUserId);
		}
		break;
		case OUCH_TYPE_REJECTED_ORDER_MSG:
		{
			OUCH_REJECTED_ORDER_MSG o;
			pkg->GetOuchRejectedOrderMsg(o);
			p->tokenMap.GetOrderIdByToken(o.orderToken, o.szOrderId, sizeof(o.szOrderId));
			p->order_check.erase(o.szOrderId);
			p->outgoing_te_packages.AddtoQueue(CClientConverter::ComposeOuchRejectedOrderMsg(o));
			if (p->oConfig.ouchConfig.logOuchMessages)
				p->ouchInJournal.WriteOuchInLog(*pkg, pkg->GetPackageLength(), "[Id:%s] - [OUCH_TYPE_REJECTED_ORDER_MSG] - TimeStamp:%lu; Type:%c; OrderToken:%u - %s; Reason:%c; UserId:%s;",
												pkg->GetIdentity(), ConvertNSTimeStamp(o.timeStamp), o.type, o.orderToken, o.szOrderId, o.reason, o.szUserId);
		}
		break;
		case OUCH_TYPE_RESTATEMENT_MSG:
		{
			OUCH_RESTATEMENT_MSG o;
			pkg->GetOuchRestatementMsg(o);
			p->tokenMap.GetOrderIdByToken(o.orderToken, o.szOrderId, sizeof(o.szOrderId));
			p->outgoing_te_packages.AddtoQueue(CClientConverter::ComposeOuchRestatementMsg(o));
			if (p->oConfig.ouchConfig.logOuchMessages)
				p->ouchInJournal.WriteOuchInLog(*pkg, pkg->GetPackageLength(), "[Id:%s] - [OUCH_TYPE_RESTATEMENT_MSG] - TimeStamp : %lu; Type:%c; OrderToken:%u - %s; Price:%u; Reason:%c;",
												pkg->GetIdentity(), ConvertNSTimeStamp(o.timeStamp), o.type, o.orderToken, o.szOrderId, o.price, o.reason);
		}
		break;
		case OUCH_TYPE_BROKEN_TRADE_MSG:
		{
			OUCH_BROKEN_TRADE_MSG o;
			pkg->GetOuchBrokenTradeMsg(o);
			p->tokenMap.GetOrderIdByToken(o.orderToken, o.szOrderId, sizeof(o.szOrderId));
			p->outgoing_te_packages.AddtoQueue(CClientConverter::ComposeOuchBrokenTradeMsg(o));
			if (p->oConfig.ouchConfig.logOuchMessages)
				p->ouchInJournal.WriteOuchInLog(*pkg, pkg->GetPackageLength(), "[Id:%s] - [OUCH_TYPE_BROKEN_TRADE_MSG] - TimeStamp : %lu; Type:%c; OrderToken:%u - %s; MatchNumber:%llu; Reason:%c;",
												pkg->GetIdentity(), ConvertNSTimeStamp(o.timeStamp), o.type, o.orderToken, o.szOrderId, o.matchNumber, o.reason);
		}
		break;
		case OUCH_TYPE_SYSTEM_EVENT_MSG:
			pkg->GetOuchSystemEventMsg(p->systemEventMsg);
			p->outgoing_te_packages.AddtoQueue(CClientConverter::ComposeOuchSystemEventMsg(p->systemEventMsg));
			if (p->oConfig.ouchConfig.logOuchMessages)
				p->ouchInJournal.WriteOuchInLog(*pkg, pkg->GetPackageLength(), "[Id:%s] - [OUCH_TYPE_SYSTEM_EVENT_MSG] - TimeStamp : %lu; Type:%c; EventCode:%c;",
												pkg->GetIdentity(), ConvertNSTimeStamp(p->systemEventMsg.timeStamp), p->systemEventMsg.type, p->systemEventMsg.eventCode);
			break;
		case OUCH_TYPE_TRADING_LIMIT_MSG:
		{
			OUCH_TRADING_LIMIT_MSG o;
			pkg->GetOuchTradingLimitMsg(o);
			if (TL_NOT_SET_VALUE == o.limit)
				o.limit = 0;
			if (TL_NOT_SET_VALUE == o.used)
				o.used = 0;
			if (TL_NOT_SET_VALUE == o.traded)
				o.traded = 0;
			p->outgoing_te_packages.AddtoQueue(CClientConverter::ComposeOuchTradingLimitMsg(o));
			// if (p->oConfig.ouchConfig.logOuchMessages)
			//	p->ouchInJournal.WriteOuchInLog(*pkg, pkg->GetPackageLength(), "[Id:%s] - [OUCH_TYPE_TRADING_LIMIT_MSG] - TimeStamp:%lu; Type:%c; Used:%ld; Traded:%ld; Limit:%ld;",
			//  		pkg->GetIdentity(), ConvertNSTimeStamp(o.timeStamp), o.tradingLimitType, o.used, o.traded, o.limit);
		}
		break;
		}
		delete pkg;
		pkg = NULL;
	}
	p->isOuchProcessRunning = false;
	p->journal.WriteLog("[COuchEngine::ProcessOuchIncomingMessages] - ProcessOuchIncomingMessages Thread ended, nId : %lu;\n", nId);
	__sync_sub_and_fetch(&p->nOuchProcessThreadsRunning, 1);
}

void COuchEngine::ProcessNewRegularOrder(CTEOrderPackage *pkg)
{
	char szOrderId[32] = {}, szAlt[32] = {}, szCustId[32] = {}, szStockCode[32] = {}, szInputUser[64] = {}, szComplianceId[64] = {}, szOrderSource[16] = {};
	uint8_t command = pkg->GetByte(),
			expire = pkg->GetByte(),
			nationality = pkg->GetByte();
	uint32_t flags = 0,
			 price = pkg->GetDWord();
	int64_t volume = pkg->GetInt64();
	uint16_t slen = pkg->GetWord();
	pkg->GetString(slen, szOrderId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szAlt);
	slen = pkg->GetWord();
	pkg->GetString(slen, szCustId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szStockCode);
	slen = pkg->GetWord();
	pkg->GetString(slen, szInputUser);
	slen = pkg->GetWord();
	pkg->GetString(slen, szComplianceId);
	uint32_t orderToken = pkg->GetDWord(),
			 orderBookId = 0;
	// orderJournal.WriteLog("OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
	//	szOrderId, command, szStockCode, BOARD_RG, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
	if (strncmp(szOrderId, szCurDate, 6)) //hari sabtu orderan di tolak
	{
		char szRejectText[256] = {};
		orderJournal.WriteLog("[REJECTED ORDER][PREVIOUS DAY ORDER] - OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
							  szOrderId, command, szStockCode, BOARD_RG, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
		strcpy(szRejectText, "Can not send previous day order, order rejected.");
		CTEOrderPackage *p = new CTEOrderPackage(292, PK_RT_RPL_REJECTED_ORDER);
		if (NULL == p)
			return;
		p->AddString(szOrderId);
		p->AddString(szRejectText);
		outgoing_te_packages.AddtoQueue(p);
		return;
	}
	if (!rgsi.GetStockFlags(szStockCode, flags)) //	Stock Code not found
	{
		char szRejectText[256] = {};
		orderJournal.WriteLog("[REJECTED ORDER][STOCK NOT FOUND] - OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
							  szOrderId, command, szStockCode, BOARD_RG, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
		strcpy(szRejectText, "Stock Code not found in Regular Market.");
		CTEOrderPackage *p = new CTEOrderPackage(292, PK_RT_RPL_REJECTED_ORDER);
		if (NULL == p)
			return;
		p->AddString(szOrderId);
		p->AddString(szRejectText);
		outgoing_te_packages.AddtoQueue(p);
		return;
	}
	char szSecurityCode[64] = {};
	sprintf(szSecurityCode, "%s.%s", szStockCode, BOARD_RG);
	orderBookId = orderBookDir.GetOrderBookId(szSecurityCode);
	if (!order_check.insert(szOrderId))
	{
		orderJournal.WriteLog("[DUPLICATE ORDER] - OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
							  szOrderId, command, szStockCode, BOARD_RG, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
		return;
	}
	// COuchEnterOrder *o = new COuchEnterOrder(orderToken, szOrderId, szComplianceId, ConvertOrderVerb(command), szOrderSource,
	// 	(0 == nationality) ? DOMICILE_INDONESIA : DOMICILE_ASING, volume / SHARES_LOT, orderBookId, price, ConvertTIF(expire), 0,
	// 	(EXP_FOK_ORDER == expire) ? volume / SHARES_LOT : (EXP_IOC_ORDER == expire) ? 1 : 0, szCustId, szSecurityCode, szInputUser);
	COuchEnterOrder *o = new COuchEnterOrder(orderToken, szOrderId, szComplianceId, ConvertOrderVerb(command), szOrderSource,
											 (0 == nationality) ? DOMICILE_INDONESIA : DOMICILE_ASING, volume / SHARES_LOT, orderBookId, (0 == price) ? MAX_PRICE : price, ConvertTIF(expire), 0,
											 (EXP_FOK_ORDER == expire) ? volume / SHARES_LOT : 0, szCustId, szSecurityCode, szInputUser);
	if (NULL == o)
	{
		journal.WriteLog("[ProcessNewRegularOrder] - Memory Exception.\n");
		orderJournal.WriteLog("[MEMORY EXCEPTION] - OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
							  szOrderId, command, szStockCode, BOARD_RG, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
		return;
	}
	if (flags & CStockInfo::SI_WATCH_CALL)
		callOutgoingQ.AddtoQueue(o, pkg->isFirstPriority() ? __OUCH_OUTQ__::PRIOR_HEAD : __OUCH_OUTQ__::PRIOR_TAIL);
	else
	{
		if (flags & CStockInfo::SI_PREOPENING)
			outgoingQ.AddPreoptoQueue(o, pkg->isFirstPriority() ? __OUCH_OUTQ__::PRIOR_HEAD : __OUCH_OUTQ__::PRIOR_TAIL);
		else
			outgoingQ.AddtoQueue(o, pkg->isFirstPriority() ? __OUCH_OUTQ__::PRIOR_HEAD : __OUCH_OUTQ__::PRIOR_TAIL);
	}
	orderJournal.WriteLog("OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
						  szOrderId, command, szStockCode, BOARD_RG, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
}

void COuchEngine::ProcessNewCashOrder(CTEOrderPackage *pkg)
{
	char szOrderId[32] = {}, szAlt[32] = {}, szCustId[32] = {}, szStockCode[32] = {}, szInputUser[64] = {}, szComplianceId[64] = {}, szOrderSource[16] = {};
	uint8_t command = pkg->GetByte(),
			expire = pkg->GetByte(),
			nationality = pkg->GetByte();
	uint32_t flags = 0,
			 price = pkg->GetDWord();
	int64_t volume = pkg->GetInt64();
	uint16_t slen = pkg->GetWord();
	pkg->GetString(slen, szOrderId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szAlt);
	slen = pkg->GetWord();
	pkg->GetString(slen, szCustId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szStockCode);
	slen = pkg->GetWord();
	pkg->GetString(slen, szInputUser);
	slen = pkg->GetWord();
	pkg->GetString(slen, szComplianceId);
	uint32_t orderBookId = 0,
			 orderToken = pkg->GetDWord();
	if (strncmp(szOrderId, szCurDate, 6))
	{
		char szRejectText[256] = {};
		orderJournal.WriteLog("[REJECTED ORDER][PREVIOUS DAY ORDER] - OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
							  szOrderId, command, szStockCode, BOARD_TN, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
		strcpy(szRejectText, "Can not send previous day order, order rejected.");
		CTEOrderPackage *p = new CTEOrderPackage(292, PK_RT_RPL_REJECTED_ORDER);
		if (NULL == p)
			return;
		p->AddString(szOrderId);
		p->AddString(szRejectText);
		outgoing_te_packages.AddtoQueue(p);
		return;
	}
	if (!tnsi.GetStockFlags(szStockCode, flags))
	{
		char szRejectText[256] = {};
		orderJournal.WriteLog("[REJECTED ORDER][STOCK NOT FOUND] - OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
							  szOrderId, command, szStockCode, BOARD_TN, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
		strcpy(szRejectText, "Stock Code not found in Cash Market.");
		CTEOrderPackage *p = new CTEOrderPackage(292, PK_RT_RPL_REJECTED_ORDER);
		if (NULL == p)
			return;
		p->AddString(szOrderId);
		p->AddString(szRejectText);
		outgoing_te_packages.AddtoQueue(p);
		return;
	}
	char szSecurityCode[64] = {};
	sprintf(szSecurityCode, "%s.%s", szStockCode, BOARD_TN);
	orderBookId = orderBookDir.GetOrderBookId(szSecurityCode);
	if (!order_check.insert(szOrderId))
	{
		orderJournal.WriteLog("[DUPLICATE ORDER] - OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
							  szOrderId, command, szStockCode, BOARD_TN, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
		return;
	}
	// COuchEnterOrder *o = new COuchEnterOrder(orderToken, szOrderId, szComplianceId, ConvertOrderVerb(command), szOrderSource,
	// 	(0 == nationality) ? DOMICILE_INDONESIA : DOMICILE_ASING, volume / SHARES_LOT, orderBookId, price, ConvertTIF(expire), 0,
	// 	(EXP_FOK_ORDER == expire) ? volume / SHARES_LOT : (EXP_IOC_ORDER == expire) ? 1 : 0, szCustId, szSecurityCode, szInputUser);
	COuchEnterOrder *o = new COuchEnterOrder(orderToken, szOrderId, szComplianceId, ConvertOrderVerb(command), szOrderSource,
											 (0 == nationality) ? DOMICILE_INDONESIA : DOMICILE_ASING, volume / SHARES_LOT, orderBookId, (0 == price) ? MAX_PRICE : price, ConvertTIF(expire), 0,
											 (EXP_FOK_ORDER == expire) ? volume / SHARES_LOT : 0, szCustId, szSecurityCode, szInputUser);
	if (NULL == o)
	{
		journal.WriteLog("[ProcessNewCashOrder] - Memory Exception.\n");
		orderJournal.WriteLog("[MEMORY EXCEPTION] - OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
							  szOrderId, command, szStockCode, BOARD_TN, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
		return;
	}
	if (flags & CStockInfo::SI_WATCH_CALL)
		callOutgoingQ.AddtoQueue(o, pkg->isFirstPriority() ? __OUCH_OUTQ__::PRIOR_HEAD : __OUCH_OUTQ__::PRIOR_TAIL);
	else
		outgoingQ.AddtoQueue(o, pkg->isFirstPriority() ? __OUCH_OUTQ__::PRIOR_HEAD : __OUCH_OUTQ__::PRIOR_TAIL);
	orderJournal.WriteLog("OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
						  szOrderId, command, szStockCode, BOARD_TN, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
}

void COuchEngine::ProcessAmendRegularOrder(const char *szAmendId, const char *szOrderId, const char *szJSXId, const char *szCustId, const char *szAlt, const char *szStockCode,
										   const char *szInputUser, const char *szComplianceId, const char *szClientId, uint32_t oldTokenId, uint8_t command, uint8_t expire, uint8_t nationality,
										   uint32_t newPrice, int64_t newVolume, int64_t tradedVolume, bool isFirstPriority)
{
	if (!order_check.insert(szAmendId))
	{
		amendJournal.WriteLog("[DUPLICATE AMEND] - TokenId:%lu; AmendId:%s; OrderId:%s; Cmd:%d; JSXId:%s; CustId:%s; Alt:%s; Stock:%s.%s; New Price:%lu; New Volume:%ld; ComplianceId:%s; Input:%s; ClientId:%s;\n",
							  oldTokenId, szAmendId, szOrderId, command, szJSXId, szCustId, szAlt, szStockCode, BOARD_RG, newPrice, newVolume, szComplianceId, szInputUser, szClientId);
		return;
	}
	uint32_t stockflags = rgsi.GetStockFlags(szStockCode);
	if (0 == atoll(szJSXId))
	{
		uint16_t ecode = (stockflags & CStockInfo::SI_WATCH_CALL) ? (
																		(lTodayScheduleState & TSES_CALL_AUCTION_SESSIONS) ? 0x01 : (callOutgoingQ.AmendOrderInQueue(szOrderId, newPrice, newVolume, false) ? 0 : 0x01))
																  : (
																		(lTodayScheduleState & TSES_SESSIONS) ? 0x01 : (outgoingQ.AmendOrderInQueue(szOrderId, newPrice, newVolume, 0 != (stockflags & CStockInfo::SI_PREOPENING)) ? 0 : 0x01));
		CTEOrderPackage *p = new CTEOrderPackage(114, PK_RT_AMEND_BEFORE_PASSING);
		if (NULL == p)
			return;
		p->AddString(szAmendId);
		p->AddString(szOrderId);
		p->AddString(szJSXId);
		p->AddDWord(newPrice);
		p->AddInt64(newVolume);
		p->SetErrorCode(ecode);
		outgoing_te_packages.AddtoQueue(p);
	}
	else
	{
		COuchReplaceOrder *o = new COuchReplaceOrder(oldTokenId, 0,
													 (0 == newPrice) ? MAX_PRICE : newPrice, newVolume / SHARES_LOT,
													 szAmendId, szOrderId, szClientId);
		if (NULL == o)
		{
			journal.WriteLog("[ProcessAmendRegularOrder] - Memory Exception.\n");
			amendJournal.WriteLog("[MEMORY EXCEPTION] - TokenId:%lu; AmendId:%s; OrderId:%s; Cmd:%d; JSXId:%s; CustId:%s; Alt:%s; Stock:%s.%s; New Price:%lu; New Volume:%ld; ComplianceId:%s; Input:%s; ClientId:%s;\n",
								  oldTokenId, szAmendId, szOrderId, command, szJSXId, szCustId, szAlt, szStockCode, BOARD_RG, newPrice, newVolume, szComplianceId, szInputUser, szClientId);
			return;
		}
		if (stockflags & CStockInfo::SI_WATCH_CALL)
			callOutgoingQ.AddtoQueue(o, isFirstPriority ? __OUCH_OUTQ__::PRIOR_HEAD : __OUCH_OUTQ__::PRIOR_TAIL);
		else
		{
			if (stockflags & CStockInfo::SI_PREOPENING)
				outgoingQ.AddPreoptoQueue(o, isFirstPriority ? __OUCH_OUTQ__::PRIOR_HEAD : __OUCH_OUTQ__::PRIOR_TAIL);
			else
				outgoingQ.AddtoQueue(o, isFirstPriority ? __OUCH_OUTQ__::PRIOR_HEAD : __OUCH_OUTQ__::PRIOR_TAIL);
		}
	}
	amendJournal.WriteLog("TokenId:%lu; AmendId:%s; OrderId:%s; Cmd:%d; JSXId:%s; CustId:%s; Alt:%s; Stock:%s.%s; New Price:%lu; New Volume:%ld; ComplianceId:%s; Input:%s; ClientId:%s;\n",
						  oldTokenId, szAmendId, szOrderId, command, szJSXId, szCustId, szAlt, szStockCode, BOARD_RG, newPrice, newVolume, szComplianceId, szInputUser, szClientId);
}

void COuchEngine::ProcessAmendRegularOrder(CTEOrderPackage *pkg)
{
	char szAmendId[32] = {}, szOrderId[32] = {}, szJSXId[32] = {}, szCustId[32] = {}, szAlt[32] = {}, szStockCode[32] = {}, szInputUser[64] = {}, szComplianceId[32] = {}, szClientId[24] = {};
	uint8_t command = pkg->GetByte(),
			expire = pkg->GetByte(),
			nationality = pkg->GetByte();
	uint32_t newPrice = pkg->GetDWord();
	int64_t newVolume = pkg->GetInt64(),
			tradedVolume = pkg->GetInt64();
	uint16_t slen = pkg->GetWord();
	pkg->GetString(slen, szAmendId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szOrderId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szJSXId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szAlt);
	slen = pkg->GetWord();
	pkg->GetString(slen, szCustId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szStockCode);
	slen = pkg->GetWord();
	pkg->GetString(slen, szInputUser);
	slen = pkg->GetWord();
	pkg->GetString(slen, szComplianceId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szClientId);
	uint32_t oldTokenId = pkg->GetDWord();
	//	if (isOuchUser(szClientId))
	ProcessAmendRegularOrder(szAmendId, szOrderId, szJSXId, szCustId, szAlt, szStockCode, szInputUser, szComplianceId, szClientId, oldTokenId,
							 command, expire, nationality, newPrice, newVolume, tradedVolume, pkg->isFirstPriority());
	// else
	// 	jonecEngine.ProcessAmendRegularOrder(szAmendId, szOrderId, szJSXId, szCustId, szAlt, szStockCode, szInputUser, szComplianceId, szClientId,
	//		command, expire, nationality, newPrice, newVolume, tradedVolume, pkg->isFirstPriority());
}

void COuchEngine::ProcessAmendCashOrder(const char *szAmendId, const char *szOrderId, const char *szJSXId, const char *szCustId, const char *szAlt, const char *szStockCode,
										const char *szInputUser, const char *szComplianceId, const char *szClientId, uint32_t oldTokenId, uint8_t command, uint8_t expire,
										uint8_t nationality, uint32_t newPrice, int64_t newVolume, int64_t tradedVolume, bool isFirstPriority)
{
	if (!order_check.insert(szAmendId))
	{
		amendJournal.WriteLog("[DUPLICATE AMEND] - TokenId:%lu; AmendId:%s; OrderId:%s; Cmd:%d; JSXId:%s; CustId:%s; Alt:%s; Stock:%s.%s; New Price:%lu; New Volume:%ld; ComplianceId:%s; Input:%s; ClientId:%s;\n",
							  oldTokenId, szAmendId, szOrderId, command, szJSXId, szCustId, szAlt, szStockCode, BOARD_TN, newPrice, newVolume, szComplianceId, szInputUser, szClientId);
		return;
	}
	uint32_t stockflags = tnsi.GetStockFlags(szStockCode);
	if (0 == atoll(szJSXId))
	{
		uint16_t ecode = (stockflags & CStockInfo::SI_WATCH_CALL) ? (
																		(lTodayScheduleState & TSES_CALL_AUCTION_SESSIONS) ? 0x01 : (callOutgoingQ.AmendOrderInQueue(szOrderId, newPrice, newVolume, false) ? 0 : 0x01))
																  : (
																		(lTodayScheduleState & TSES_SESSIONS) ? 0x01 : (outgoingQ.AmendOrderInQueue(szOrderId, newPrice, newVolume, false) ? 0 : 0x01));
		CTEOrderPackage *p = new CTEOrderPackage(114, PK_RT_AMEND_BEFORE_PASSING);
		if (NULL == p)
			return;
		p->AddString(szAmendId);
		p->AddString(szOrderId);
		p->AddString(szJSXId);
		p->AddDWord(newPrice);
		p->AddInt64(newVolume);
		p->SetErrorCode(ecode);
		outgoing_te_packages.AddtoQueue(p);
	}
	else
	{
		COuchReplaceOrder *o = new COuchReplaceOrder(oldTokenId, 0,
													 (0 == newPrice) ? MAX_PRICE : newPrice, newVolume / SHARES_LOT,
													 szAmendId, szOrderId, szClientId);
		if (NULL == o)
		{
			journal.WriteLog("[ProcessAmendCashOrder] - Memory Exception.\n");
			amendJournal.WriteLog("[MEMORY EXCEPTION] - TokenId:%lu; AmendId:%s; OrderId:%s; Cmd:%d; JSXId:%s; CustId:%s; Alt:%s; Stock:%s.%s; New Price:%lu; New Volume:%ld; ComplianceId:%s; Input:%s; ClientId:%s;\n",
								  oldTokenId, szAmendId, szOrderId, command, szJSXId, szCustId, szAlt, szStockCode, BOARD_TN, newPrice, newVolume, szComplianceId, szInputUser, szClientId);
			return;
		}
		if (stockflags & CStockInfo::SI_WATCH_CALL)
			callOutgoingQ.AddtoQueue(o, isFirstPriority ? __OUCH_OUTQ__::PRIOR_HEAD : __OUCH_OUTQ__::PRIOR_TAIL);
		else
			outgoingQ.AddtoQueue(o, isFirstPriority ? __OUCH_OUTQ__::PRIOR_HEAD : __OUCH_OUTQ__::PRIOR_TAIL);
	}
	amendJournal.WriteLog("TokenId:%lu; AmendId:%s; OrderId:%s; Cmd:%d; JSXId:%s; CustId:%s; Alt:%s; Stock:%s.%s; New Price:%lu; New Volume:%ld; ComplianceId:%s; Input:%s; ClientId:%s;\n",
						  oldTokenId, szAmendId, szOrderId, command, szJSXId, szCustId, szAlt, szStockCode, BOARD_TN, newPrice, newVolume, szComplianceId, szInputUser, szClientId);
}

void COuchEngine::ProcessAmendCashOrder(CTEOrderPackage *pkg)
{
	char szAmendId[32] = {}, szOrderId[32] = {}, szJSXId[32] = {}, szCustId[32] = {}, szAlt[32] = {}, szStockCode[32] = {}, szInputUser[64] = {}, szComplianceId[32] = {}, szClientId[24] = {};
	uint8_t command = pkg->GetByte(),
			expire = pkg->GetByte(),
			nationality = pkg->GetByte();
	uint32_t newPrice = pkg->GetDWord();
	int64_t newVolume = pkg->GetInt64(),
			tradedVolume = pkg->GetInt64();
	uint16_t slen = pkg->GetWord();
	pkg->GetString(slen, szAmendId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szOrderId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szJSXId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szAlt);
	slen = pkg->GetWord();
	pkg->GetString(slen, szCustId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szStockCode);
	slen = pkg->GetWord();
	pkg->GetString(slen, szInputUser);
	slen = pkg->GetWord();
	pkg->GetString(slen, szComplianceId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szClientId);
	uint32_t oldTokenId = pkg->GetDWord();
	// if (isOuchUser(szClientId))
	ProcessAmendCashOrder(szAmendId, szOrderId, szJSXId, szCustId, szAlt, szStockCode, szInputUser, szComplianceId, szClientId, oldTokenId,
						  command, expire, nationality, newPrice, newVolume, tradedVolume, pkg->isFirstPriority());
	// else
	// 	jonecEngine.ProcessAmendCashOrder(szAmendId, szOrderId, szJSXId, szCustId, szAlt, szStockCode, szInputUser, szComplianceId, szClientId,
	// 		command, expire, nationality, newPrice, newVolume, tradedVolume, TRUE == pkg->isFirstPriority());
}

void COuchEngine::ProcessWithdrawRegularOrder(const char *szWithdrawId, const char *szOrderId, const char *szJSXId, const char *szCustId, const char *szAlt, const char *szStockCode,
											  const char *szInputUser, const char *szBrokerId, const char *szClientId, uint32_t orderToken, uint8_t command, uint8_t nationality,
											  uint32_t price)
{
	if (!order_check.insert(szWithdrawId))
	{
		withdrawJournal.WriteLog("[DUPLICATE WITHDRAW] - TokenId:%lu; WithdrawId:%s; OrderId:%s; JSXId:%s; CustId:%s; Alt:%s; Cmd:%d; Stock:%s.%s; Input:%s; ClientId:%s;\n",
								 orderToken, szWithdrawId, szOrderId, szJSXId, szCustId, szAlt, command, szStockCode, BOARD_RG, szInputUser, szClientId);
		return;
	}
	uint32_t stockflags = rgsi.GetStockFlags(szStockCode);
	if (0 == atoll(szJSXId))
	{
		uint16_t ecode = (stockflags & CStockInfo::SI_WATCH_CALL) ? (
																		(lTodayScheduleState & TSES_CALL_AUCTION_SESSIONS) ? 0x01 : (callOutgoingQ.WithdrawOrderInQueue(szOrderId, false) ? 0 : 0x01))
																  : (
																		(lTodayScheduleState & TSES_SESSIONS) ? 0x01 : (outgoingQ.WithdrawOrderInQueue(szOrderId, 0 != (stockflags & CStockInfo::SI_PREOPENING)) ? 0 : 0x01));
		CTEOrderPackage *p = new CTEOrderPackage(102, PK_RT_WITHDRAW_BEFORE_PASSING);
		if (NULL == p)
			return;
		p->AddString(szWithdrawId);
		p->AddString(szOrderId);
		p->AddString(szJSXId);
		p->SetErrorCode(ecode);
		outgoing_te_packages.AddtoQueue(p);
	}
	else
	{
		COuchCancelOrder *o = new COuchCancelOrder(orderToken, szWithdrawId, szOrderId, szClientId);
		if (NULL == o)
		{
			journal.WriteLog("[ProcessWithdrawRegularOrder] - Memory Exception.\n");
			withdrawJournal.WriteLog("[MEMORY EXCEPTION] - TokenId:%lu; WithdrawId:%s; OrderId:%s; JSXId:%s; CustId:%s; Alt:%s; Cmd:%d; Stock:%s.%s; Input:%s; ClientId:%s;\n",
									 orderToken, szWithdrawId, szOrderId, szJSXId, szCustId, szAlt, command, szStockCode, BOARD_RG, szInputUser, szClientId);
			return;
		}
		outgoingQ.AddImmediately(o);
	}
	withdrawJournal.WriteLog("TokenId:%lu; WithdrawId:%s; OrderId:%s; JSXId:%s; CustId:%s; Alt:%s; Cmd:%d; Stock:%s.%s; Input:%s; ClientId:%s;\n",
							 orderToken, szWithdrawId, szOrderId, szJSXId, szCustId, szAlt, command, szStockCode, BOARD_RG, szInputUser, szClientId);
}

void COuchEngine::ProcessWithdrawRegularOrder(CTEOrderPackage *pkg)
{
	char szOrderId[32] = {}, szWithdrawId[32] = {}, szJSXId[32] = {}, szCustId[32] = {}, szAlt[32] = {}, szStockCode[32] = {}, szInputUser[64] = {}, szClientId[32] = {}, szBrokerId[8] = {};
	uint8_t command = pkg->GetByte(),
			nationality = pkg->GetByte();
	uint32_t price = pkg->GetDWord();
	uint16_t slen = pkg->GetWord();
	pkg->GetString(slen, szWithdrawId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szOrderId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szJSXId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szAlt);
	slen = pkg->GetWord();
	pkg->GetString(slen, szCustId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szStockCode);
	slen = pkg->GetWord();
	pkg->GetString(slen, szInputUser);
	slen = pkg->GetWord();
	pkg->GetString(slen, szBrokerId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szClientId);
	uint32_t orderToken = pkg->GetDWord();
	// if (isOuchUser(szClientId))
	ProcessWithdrawRegularOrder(szWithdrawId, szOrderId, szJSXId, szCustId, szAlt, szStockCode, szInputUser, szBrokerId, szClientId, orderToken, command, nationality, price);
	// else
	//	jonecEngine.ProcessWithdrawRegularOrder(szWithdrawId, szOrderId, szJSXId, szCustId, szAlt, szStockCode, szInputUser, szBrokerId, szClientId, command, nationality, price);
}

void COuchEngine::ProcessWithdrawCashOrder(const char *szWithdrawId, const char *szOrderId, const char *szJSXId, const char *szCustId, const char *szAlt, const char *szStockCode,
										   const char *szInputUser, const char *szBrokerId, const char *szClientId, uint32_t orderToken, uint8_t command, uint8_t nationality,
										   uint32_t price)
{
	if (!order_check.insert(szWithdrawId))
	{
		withdrawJournal.WriteLog("[DUPLICATE WITHDRAW] - TokenId:%lu; WithdrawId:%s; OrderId:%s; JSXId:%s; CustId:%s; Alt:%s; Cmd:%d; Stock:%s.%s; Input:%s; ClientId:%s;\n",
								 orderToken, szWithdrawId, szOrderId, szJSXId, szCustId, szAlt, command, szStockCode, BOARD_TN, szInputUser, szClientId);
		return;
	}
	uint32_t stockflags = tnsi.GetStockFlags(szStockCode);
	if (0 == atoll(szJSXId))
	{
		uint16_t ecode = (stockflags & CStockInfo::SI_WATCH_CALL) ? (
																		(lTodayScheduleState & TSES_CALL_AUCTION_SESSIONS) ? 0x01 : (callOutgoingQ.WithdrawOrderInQueue(szOrderId, false) ? 0 : 0x01))
																  : (
																		(lTodayScheduleState & TSES_SESSIONS) ? 0x01 : (outgoingQ.WithdrawOrderInQueue(szOrderId, false) ? 0 : 0x01));
		CTEOrderPackage *p = new CTEOrderPackage(102, PK_RT_WITHDRAW_BEFORE_PASSING);
		if (NULL == p)
			return;
		p->AddString(szWithdrawId);
		p->AddString(szOrderId);
		p->AddString(szJSXId);
		p->SetErrorCode(ecode);
		outgoing_te_packages.AddtoQueue(p);
	}
	else
	{
		COuchCancelOrder *o = new COuchCancelOrder(orderToken, szWithdrawId, szOrderId, szClientId);
		if (NULL == o)
		{
			journal.WriteLog("[ProcessWithdrawCashOrder] - Memory Exception.\n");
			withdrawJournal.WriteLog("[MEMORY EXCEPTION] - TokenId:%lu; WithdrawId:%s; OrderId:%s; JSXId:%s; CustId:%s; Alt:%s; Cmd:%d; Stock:%s.%s; Input:%s; ClientId:%s;\n",
									 orderToken, szWithdrawId, szOrderId, szJSXId, szCustId, szAlt, command, szStockCode, BOARD_TN, szInputUser, szClientId);
			return;
		}
		outgoingQ.AddImmediately(o);
	}
	withdrawJournal.WriteLog("TokenId:%lu; WithdrawId:%s; OrderId:%s; JSXId:%s; CustId:%s; Alt:%s; Cmd:%d; Stock:%s.%s; Input:%s; ClientId:%s;\n",
							 orderToken, szWithdrawId, szOrderId, szJSXId, szCustId, szAlt, command, szStockCode, BOARD_TN, szInputUser, szClientId);
}

void COuchEngine::ProcessWithdrawCashOrder(CTEOrderPackage *pkg)
{
	char szOrderId[32] = {}, szWithdrawId[32] = {}, szJSXId[32] = {}, szCustId[32] = {}, szAlt[32] = {}, szStockCode[32] = {}, szInputUser[64] = {}, szClientId[32] = {}, szBrokerId[8] = {};
	uint8_t command = pkg->GetByte(),
			nationality = pkg->GetByte();
	uint32_t price = pkg->GetDWord();
	uint16_t slen = pkg->GetWord();
	pkg->GetString(slen, szWithdrawId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szOrderId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szJSXId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szAlt);
	slen = pkg->GetWord();
	pkg->GetString(slen, szCustId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szStockCode);
	slen = pkg->GetWord();
	pkg->GetString(slen, szInputUser);
	slen = pkg->GetWord();
	pkg->GetString(slen, szBrokerId);
	slen = pkg->GetWord();
	pkg->GetString(slen, szClientId);
	uint32_t orderToken = pkg->GetDWord();
	// if (isOuchUser(szClientId))
	ProcessWithdrawCashOrder(szWithdrawId, szOrderId, szJSXId, szCustId, szAlt, szStockCode, szInputUser, szBrokerId, szClientId, orderToken, command, nationality, price);
	// else
	//	jonecEngine.ProcessWithdrawCashOrder(szWithdrawId, szOrderId, szJSXId, szCustId, szAlt, szStockCode, szInputUser, szBrokerId, szClientId, command, nationality, price);
}

void COuchEngine::ProcessIncomingTEPackages(void *param)
{
	CTEOrderPackage *pkg = NULL;
	COuchEngine *p = (COuchEngine *)param;
	p->isTEThreadRunning = true;
	uint32_t nId = __sync_add_and_fetch(&p->nZMQFrontEndThreadsRunning, 1);
	p->journal.WriteLog("[COuchEngine::ProcessIncomingTEPackages] - Starting ProcessIncomingTEPackages, nId : %lu;\n", nId);
	while (p->isTEThreadRunning)
	{
		if (NULL == (pkg = p->incoming_te_packages.GetfromQueue()))
		{
			usleep(USLEEP_DELAY);
			continue;
		}
		switch (pkg->GetPackageId())
		{
		case PK_RT_NEW_REGULAR_ORDER:
			p->ProcessNewRegularOrder(pkg);
			break;
		case PK_RT_NEW_CASH_ORDER:
			p->ProcessNewCashOrder(pkg);
			break;
		case PK_RT_AMEND_REGULAR_ORDER:
			p->ProcessAmendRegularOrder(pkg);
			break;
		case PK_RT_AMEND_CASH_ORDER:
			p->ProcessAmendCashOrder(pkg);
			break;
		case PK_RT_WITHDRAW_REGULAR_ORDER:
			p->ProcessWithdrawRegularOrder(pkg);
			break;
		case PK_RT_WITHDRAW_CASH_ORDER:
			p->ProcessWithdrawCashOrder(pkg);
			break;
		}
	}
	p->journal.WriteLog("[COuchEngine::ProcessIncomingTEPackages] - ProcessIncomingTEPackages Ended, nId : %lu;\n", nId);
	__sync_sub_and_fetch(&p->nZMQFrontEndThreadsRunning, 1);
}

void COuchEngine::ZMQAOProxyFrontendThread(void *param)
{
	int hwm = 0;
	COuchEngine *p = (COuchEngine *)param;
	uint32_t nId = __sync_add_and_fetch(&p->nZMQFrontEndThreadsRunning, 1);
	p->journal.WriteLog("[COuchEngine::ZMQAOProxyFrontendThread] - Starting ZMQAOProxyFrontendThread, nId : %lu;\n", nId);
	void *context = zmq_ctx_new();
	if (NULL == context)
	{
		p->journal.WriteLog("Can not create ZMQ Context, ZMQAOProxyFrontendThread Ended.\n");
		return;
	}
	zmq_ctx_set(context, ZMQ_IO_THREADS, p->oConfig.zmqConfig.zmq_thread_count);
	void *zsock = zmq_socket(context, ZMQ_DEALER);
	if (NULL == zsock)
	{
		p->journal.WriteLog("[COuchEngine::ZMQAOProxyFrontendThread] - Can not create ZMQ Socket, ZMQAOProxyFrontendThread Ended.\n");
		return;
	}
	p->journal.WriteLog("[COuchEngine::ZMQAOProxyFrontendThread] - Set ZMQ Identity : %s.\n", ZMQ_AOPROXY_IDENTITY);
	zmq_setsockopt(zsock, ZMQ_IDENTITY, ZMQ_AOPROXY_IDENTITY, strlen(ZMQ_AOPROXY_IDENTITY));
	zmq_setsockopt(zsock, ZMQ_RCVHWM, &hwm, sizeof(hwm));
	zmq_setsockopt(zsock, ZMQ_SNDHWM, &hwm, sizeof(hwm));
	zmq_setsockopt(zsock, ZMQ_LINGER, &(hwm = 100), sizeof(hwm)); //	Set Linger 100 mseconds
	zmq_setsockopt(zsock, ZMQ_RCVTIMEO, &hwm, sizeof(hwm));		  //	Receive time out 100 mseconds
	p->journal.WriteLog("[COuchEngine::ZMQAOProxyFrontendThread] - Connect ZMQ Frontend : %s.\n", p->oConfig.zmqConfig.zmq_AOProxy_Frontend);
	if (-1 == zmq_connect(zsock, p->oConfig.zmqConfig.zmq_AOProxy_Frontend))
	{
		p->isTEThreadRunning = false;
		p->journal.WriteLog("[COuchEngine::ZMQAOProxyFrontendThread][Id:%s] - Failed to connect to ZMQ frontend %s, ZMQAOProxyFrontendThread Ended.\n", ZMQ_AOPROXY_IDENTITY, p->oConfig.zmqConfig.zmq_AOProxy_Frontend);
	}
	int size = 0;
	zmq_msg_t msg;
	time_t lastTime = time(NULL);
	p->isTEThreadRunning = true;
	// for (uint16_t i = 0; i < p->oConfig.zmqConfig.frontend_process_thread_count; i++)

	while (p->isTEThreadRunning)
	{
		if (-1 != size)
			zmq_msg_init(&msg);
		size = zmq_msg_recv(&msg, zsock, 0);
		if (-1 == size)
		{
			uint32_t nSent = 0;
			if (difftime(time(NULL), lastTime) >= 1)
			{
				lastTime = time(NULL);
				CTEOrderPackage o(1, PK_HTS_HEART_BEAT);
				o.AddByte(p->isOuchInstanceAlive() ? 1 : 0);
				zmq_send(zsock, (const char *)o, o.GetPackageLength(), 0);
			}
			CTEOrderPackage *pkg = NULL;
			while (NULL != (pkg = p->outgoing_te_packages.GetfromQueue()))
			{
				zmq_send(zsock, (const char *)*pkg, pkg->GetPackageLength(), 0);
				if (!p->isTEThreadRunning || ++nSent > 100)
					break;
			}
			continue;
		}
		if (size >= (int)TE_HEADER_LENGTH)
		{
			char *psz = new char[size];
			memcpy(psz, zmq_msg_data(&msg), size);
			CTEOrderPackage *pkg = new CTEOrderPackage();
			pkg->SetRawData(psz, size);
			p->incoming_te_packages.AddtoQueue(pkg);
		}
		zmq_msg_close(&msg);
	}
	zmq_close(zsock);
	zmq_ctx_destroy(context);
	p->journal.WriteLog("[COuchEngine::ZMQAOProxyFrontendThread] - ZMQAOProxyFrontendThread Ended, nId : %lu;\n", nId);
	__sync_sub_and_fetch(&p->nZMQFrontEndThreadsRunning, 1);
}
