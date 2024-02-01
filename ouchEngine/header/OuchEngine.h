#pragma once
#include <vector>
#include <pthread.h>
#include "journal.h"
#include "GSocket.h"
#include "TSession.h"
#include "miscfunc.h"
#include "StockInfo.h"
#include "OrderCheck.h"
#include "Ouch.h"
#include "Itch.h"
#include "OuchQueue.h"
#include "GQueue.h"
#include "ClientConverter.h"
#include "OrderBookDirectory.h"

class COuchEngine
{
public:
    typedef CGQueue<CItch> _ITCHQ_;
    typedef CGQueue<COuch> __OUCH_INQ__;
    typedef COuchQueue<CSoupBin> __OUCH_OUTQ__;
    typedef CGQueue<CTEOrderPackage> __TEORDER_QUEUE__;
    
    class COuchNode
    {
    public:
        COuchNode(COuchEngine *parent);
        ~COuchNode();

        void ConnectToOuchServer();
        void DisconnectFromOuchServer();
        void WaitForThreadsEnded();

        bool isOuchUserId(const char *szId);
        void SetSoupBinCfgData(SOUPBIN_CFG_DATA *o);

        bool isAlive() { return isConnectionActive; };
        bool isOuchThreadsRunning() { return isThreadRunning; };

    private:
        uint16_t curServer;
        uint32_t nThreadsRunning;
        bool isThreadRunning,
            isConnectionActive;
        CGSocket ouchSocket;
        COuchEngine *pparent;
        SOUPBIN_CFG_DATA ouchCfg;
        pthread_mutex_t mutex;

        long SendOuchClientHeartBeat();
        void SubscribeOUCHTradingLimitMessage();
        uint16_t GetCurrentActiveServerAddr(char *szAddr);
        int SendIDXOUCHLoginPackage(const char *uid, const char *pwd, unsigned long lseq);

        void RequeueOuchMessage(CSoupBin *msg, bool isNonImmediate, bool isPreop, bool isCallAuction, __OUCH_OUTQ__::QUEUE_PRIOR prior = __OUCH_OUTQ__::PRIOR_TAIL);

        bool SendOuchEnterOrderMessage(COuchEnterOrder *o, bool isNonImmediate, bool ispreop, bool isCallAuction);
        bool SendOuchReplaceOrderMessage(COuchReplaceOrder *o, bool isNonImmediate, bool ispreop, bool isCallAuction);
        bool SendOuchCancelOrderMessage(COuchCancelOrder *o, bool isNonImmediate, bool ispreop);

        void StartOuchSendThreads();

        static void OuchSendThread(void *p);
        static void OuchClientThread(void *p);
    };

public:
    typedef struct _ZMQ_CFG_
    {
        uint16_t zmq_thread_count;
        uint16_t frontend_process_thread_count;
        char zmq_AOProxy_Frontend[128];
        char zmq_ITCH_Publisher_Address[128];
    } ZMQ_CFG;
    typedef std::vector<COuchNode *> OUCH_VNODE;
    typedef struct _OUCH_CFG_
    {
        bool logOuchMessages;
        uint16_t ouchInstances;
        uint16_t ouchProcess_thread_count;
        OUCH_VNODE vouch;
    } OUCH_CFG;
    typedef struct _CONFIG_
    {
        ZMQ_CFG zmqConfig;
        OUCH_CFG ouchConfig;
        SCHEDULE schedule[7]; // 0 : Minggu; 1 : Senin; 6 : Sabtu;
    } CONFIG;
    __TEORDER_QUEUE__ outgoing_te_packages,
        incoming_te_packages;

public:
    COuchEngine();
    ~COuchEngine();

    bool LoadConfigFile();
    bool ReloadSentOrderId();
    bool ReloadJournalFile();
    void ReleaseAllPackages();
    bool LoadOrderBookDirectoryFromFile();

    void StartSchedulerThread();
    void StopSchedulerThread();

    void StartItchSubscriber();
    void StopItchSubscriber();

    void StartOuchProcessThread();
    void StopOuchProcessThread();

    void StartTradeEngineThread();
    void StopTradeEngineThread();

    void SendStartSendingOrderPackage();

    bool isOuchInstanceAlive();
    bool IsAllOuchInstancesAlive();
    bool isAllOuchInstancesThreadRunning();
    void ConnectToOuchServers(int iSvr);      //  -1 : Connect All Ouch Instances
    void DisconnectFromOuchServers(int iSvr); //  -1 : Disconnect All Ouch Instances

    bool GetHoliday() const { return bHoliday; };
    CJournal *GetJournalPtr() { return &journal; };

    void StartServices();
    void StopServices();
    void WaitProcesses();

private:
    _ITCHQ_ itchQ;
    __OUCH_INQ__ ouchIncomingQ;
    __OUCH_OUTQ__ outgoingQ, callOutgoingQ;

    CONFIG oConfig;
    OUCH_SYSTEM_EVENT_MSG systemEventMsg;
    bool isSchedulerRunning, isItchSubscRunning, isOuchProcessRunning, isTEThreadRunning, isEndOfSession, bHoliday, bReqResendOrders;
    CStockInfo rgsi, tnsi;
    COrderCheck order_check;
    CTokenMap tokenMap;
    COrderBookDirectory orderBookDir;
    pthread_t tid_schedulerThread, tid_itchSubscriber;
    uint16_t lTodayScheduleState, lScheduleState;
    uint32_t lCurDate, //   yyyymmdd
                       // lCurTime, //   hhmmssxxx
        weekDay,       //   0 : Sunday; 1 : Monday; 6 : Saturday
        tokenId, SHARES_LOT;
    uint32_t nZMQFrontEndThreadsRunning, nOuchProcessThreadsRunning;
    CTSession psession, ssession, callsession;
    CJournal journal, orderJournal, amendJournal, withdrawJournal,
        ouchSentJournal, ouchOutJournal, ouchInJournal;
    char szCurDate[8]; // yymmdd   ( For Check Valid Incoming Orders )

private:
    uint32_t ConvertTIF(uint8_t expiry);
    uint8_t ConvertOrderVerb(uint8_t command);
    void PrintErrNoMessage(int e, const char *s);

    void ReleaseOuchInstances();

    void CheckOUCHClientRunning();
    COuchNode *GetOuchInstanceByClientId(const char *szClientId);

    static void SchedulerThread(void *param);
    static void ProcessItchMessages(void *param);
    static void ZMQProcessItchSubscription(void *param);

    static void ProcessOuchIncomingMessages(void *param);

    static void ProcessIncomingTEPackages(void *param);
    static void ZMQAOProxyFrontendThread(void *param);

    void ProcessItchSystemEventMsg(uint32_t timeStamp, ITCH_SYSTEM_EVENT_MSG &o);
    uint16_t SynchronizeTimeStateLite(uint32_t lcurTime, uint16_t &lScheduleState);

    void ProcessNewRegularOrder(CTEOrderPackage *pkg);
    void ProcessNewCashOrder(CTEOrderPackage *pkg);
    void ProcessAmendRegularOrder(CTEOrderPackage *pkg);
    void ProcessAmendCashOrder(CTEOrderPackage *pkg);
    void ProcessWithdrawRegularOrder(CTEOrderPackage *pkg);
    void ProcessWithdrawCashOrder(CTEOrderPackage *pkg);
    void ProcessAmendRegularOrder(const char *szAmendId, const char *szOrderId, const char *szJSXId, const char *szCustId, const char *szAlt, const char *szStockCode,
                                  const char *szInputUser, const char *szComplianceId, const char *szClientId, uint32_t oldTokenId, uint8_t command, uint8_t expire, uint8_t nationality,
                                  uint32_t newPrice, int64_t newVolume, int64_t tradedVolume, bool isFirstPriority);
    void ProcessAmendCashOrder(const char *szAmendId, const char *szOrderId, const char *szJSXId, const char *szCustId, const char *szAlt, const char *szStockCode,
                               const char *szInputUser, const char *szComplianceId, const char *szClientId, uint32_t oldTokenId, uint8_t command, uint8_t expire,
                               uint8_t nationality, uint32_t newPrice, int64_t newVolume, int64_t tradedVolume, bool isFirstPriority);
    void ProcessWithdrawRegularOrder(const char *szWithdrawId, const char *szOrderId, const char *szJSXId, const char *szCustId, const char *szAlt, const char *szStockCode,
                                     const char *szInputUser, const char *szBrokerId, const char *szClientId, uint32_t orderToken, uint8_t command, uint8_t nationality, uint32_t price);
    void ProcessWithdrawCashOrder(const char *szWithdrawId, const char *szOrderId, const char *szJSXId, const char *szCustId, const char *szAlt, const char *szStockCode,
                                  const char *szInputUser, const char *szBrokerId, const char *szClientId, uint32_t orderToken, uint8_t command, uint8_t nationality, uint32_t price);

    inline void ResetTokenId() { __sync_lock_test_and_set(&tokenId, 0); };
    inline void SetTokenId(uint32_t id) { __sync_lock_test_and_set(&tokenId, id); };
    inline uint32_t GetNextTokenId() { return __sync_add_and_fetch(&tokenId, 1); };
};
