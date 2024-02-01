#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <sstream>

#include <thread>
#include <chrono>
#include <iomanip>

#include <deque>
#include <pthread.h>
#include <signal.h>

#include <boost/atomic.hpp>
#include <tbb/concurrent_queue.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/mem_fn.hpp>
#include <boost/function.hpp>
#include <unistd.h>

#include "inc/WriteLog.hpp"
#include "inc/netstat.hpp"
#include "inc/FileCustom.hpp"
#include "inc/sendOncsv.hpp"

// #include "inc/GuiConsole.hpp"

#include "include/IDXConstants.h"
#include "include/IDXMessage.h"
#include "include/Message.h"
#include <tbb/concurrent_queue.h>
#include "include/DataDictionary.h"
#include "include/FixFieldNumbers.h"
#include "include/IDXUtil.h"

#include "include/md5.hpp"

// #include "OuchEngine.h"
#include "ouchEngine/header/OuchEngine.h"
#include <zmq.h>
#include "ouchEngine/header/TEOrderPackage.h"
#include "ouchEngine/header/miscfunc.h"
#include "ouchEngine/header/ClientConverter.h"
#include "ouchEngine/header/GQueue.h"

#define ZMQ_AOPROXY_IDENTITY "ORDER_ENGINE"
// typedef CGQueue<CTEOrderPackage> __TEORDER_QUEUE__;

typedef const std::string CSTRING;
typedef boost::asio::io_service IOSERVICE;

class FixSession
{
public:
    FixSession(boost::asio::io_service &ioService, FIX::DataDictionary &dd, const std::string &targetIP, const std::string &targetPort, const std::string &username, const std::string &password, const float sendsleep, const float recvsleep)
        : ioService_(ioService),
          _dd(dd),
          targetIP_(targetIP),
          targetPort_(targetPort),
          username_(username),
          password_(password),
          _connect_timer(ioService),
          _connection_timeout(boost::posix_time::seconds(100)),
          _sendsleep(sendsleep),
          _recvsleep(recvsleep)
    {

        try
        {
            brecvthreadrun = true;
            _thread_receive = new boost::thread(boost::bind(&FixSession::handle_receive_thread, this));
            // std::cout << "brecvthreadrun run." << std::endl;
            // std::cout << "mencoba menyambung ulang..." << std::endl;
        }
        catch (std::exception &e)
        {
            // std::cout << "gagal menyambung..." << std::endl;
            // OnErrorSystem(e);
            cerr << e.what() << endl;
        }

        try
        {
            bsendthreadrun = true;
            _thread_send = new boost::thread(boost::bind(&FixSession::handle_send_thread, this));
            // std::cout << "halo halo halo..." << std::endl;
        }
        catch (std::exception &e)
        {
            // OnErrorSystem(e);
            cerr << e.what() << endl;
        }

        try
        {
            bsendTypeAn = true;
            _thread_TypeAN = new boost::thread(boost::bind(&FixSession::TypeAN, this));
            // std::cout << "TypeAN..." << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        try
        {
            conCheck = true;
            _thread_conCheck = new boost::thread(boost::bind(&FixSession::CheckConnection, this));
            std::cout << "TypeAcheckN..." << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        try
        {
            zmq_thread = true;
            _thread_zmq = new boost::thread(boost::bind(&FixSession::ZMQAOProxyFrontendThread, this));
            std::cout << "ZMQAOProxyFrontendThread..." << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        try
        {
            incomingpackage_thread = true;
            _thread_incomingpackage = new boost::thread(boost::bind(&FixSession::ProcessIncomingTEPackages, this));
            std::cout << "ProcessIncomingTEPackagesThread..." << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        try
        {
            // incomingpackage_thread = true;
            _thread_itchsubscrption = new boost::thread(boost::bind(&FixSession::ZMQProcessItchSubscription, this));
            std::cout << "ZMQProcessItchSubscription..." << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
        // }
    }

    ~FixSession() // destructor
    {
        if (_thread_service != NULL)
        {
            brecvthreadrun = false;
            bsendthreadrun = false;
            bsendTypeAn = false;
            conCheck = false;
            incomingpackage_thread = false;

            std::cout << "threadjoind() function has been running: " << std::endl;
            // logWrite("threadjoind() function has be running: ", MAGENTA);

            _thread_service->join();

            _thread_receive->join();
            _thread_send->join();
            _thread_TypeAN->join();
            _thread_conCheck->join();
            _thread_zmq->join();
            _thread_incomingpackage->join();
            _thread_itchsubscrption->join();

            delete _thread_itchsubscrption;
            delete _thread_incomingpackage;
            delete _thread_zmq;
            delete _thread_conCheck;
            delete _thread_TypeAN;
            delete _thread_service;

            delete _thread_receive;
            delete _thread_send;
        }
        else
        {
            std::cout << "threadjoind() not running: " << std::endl;
        }
    }

    void NewOrder(const std::string clorid, const std::string AccountType, const std::string orderQty, const std::string orderType, const std::string price, const std::string instrument)
    {

        if (FixSession::_loggedOn2)
        {
            FixSession::MsgNum++;
            std::cout << "new order" << std::endl;
            std::string xs(msg::sendCSVOrderSingle("vpft1001", MsgNum, clorid, AccountType, orderQty, orderType, price, instrument));
            logWrite("New Order: " + Replace_Soh(xs), YELLOW, outp);
            SendMessage(xs);
        }
    }

    void MarketData()
    {
        if (FixSession::_loggedOn2)
        {

            std::string xs(msg::SendCSVMarketDataSnapshot("vpft1001", MsgNum));
            logWrite("New Order: " + Replace_Soh(xs), YELLOW, outp);
            SendMessage(xs);
        }
    }

    void MassQuote()
    {
        if (FixSession::_loggedOn2)
        {

            std::string xs(msg::sendCSVMassQuote("./conf/mass_quote.csv", "vpft1001", MsgNum));
            logWrite("New Order: " + Replace_Soh(xs), YELLOW, outp);
            SendMessage(xs);
        }
    }

    void MassQuoteActionRequest()
    {
        if (FixSession::_loggedOn2)
        {

            std::string xs(msg::sendCSVOrderMassActionRequest("./conf/mass_action.csv", "vpft1001", MsgNum));
            logWrite("New Order: " + Replace_Soh(xs), YELLOW, outp);
            SendMessage(xs);
        }
    }
    void TypeAE()
    {
        if (FixSession::_loggedOn2)
        {
            std::cout << "type AE" << std::endl;
            std::string xs(msg::sendCSVCancelNegoDealInternalCrossingInisiator("./conf/cancel_negotwoside_inisiator.csv", "vpft1001", MsgNum));
            logWrite("New Order: " + Replace_Soh(xs), YELLOW, outp);
            SendMessage(xs);
        }
    }
    void TypeAN()
    {
        try
        {

            while (bsendTypeAn)
            {
                if (FixSession::_loggedOn2)
                {
                    std::cout << YELLOW << "_loggedOn: " << _loggedOn << RESET << std::endl;
                    MsgNum++;
                    FixSession::_TypeAn = true;

                    std::string xs(msg::sendCSVRequestForPositions("./conf/request_position.csv", "vpft1001", MsgNum));
                    std::cout << YELLOW << "AN: " << Replace_Soh(xs) << RESET << std::endl;
                    logWrite("AN: " + Replace_Soh(xs), YELLOW, outp);
                    SendMessage(xs);
                }
                boost::posix_time::seconds idletime(20);
                boost::this_thread::sleep(idletime);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    void CheckConnection()
    {
        std::string reconect = "menghubungkan ulang yang ke ";
        int count = 0;
        while (conCheck)
        {
            if (!FixSession::IsActive()) // tidak dalam kondisi konek
            {
                // reconect += ".";
                count++;
                isTEThreadRunning = false;
                std::cout << YELLOW << reconect << std::to_string(count) << " dalam 700(ms)" << RESET << std::endl;
                FixSession::start();
            }
            else // sudah konek
            {
                if (!_loggedOn2)
                {
                    /* code */
                }

                reconect = "reconnection ";

                // std::cout << "aktif" << std::endl;
            }
            boost::posix_time::milliseconds idletime(700);
            boost::this_thread::sleep(idletime);
        }
    }

    void start()
    {
        // Resolve the target IP and port
        logWrite("\t\t\t\tconnection starts...", YELLOW, outp);
        std::cout << "connection start" << std::endl;
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::resolver resolver(ioService_);
        boost::asio::ip::tcp::resolver::query query(targetIP_, targetPort_);
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
        connect_start(iterator);
        _thread_service = new boost::thread(boost::bind(&FixSession::runIOService, this, boost::ref(ioService_)));
    }

    std::string currentTime()
    {
        WriteLog time;
        return time.gmt_Time();
    }

    void ProcessIncomingTEPackages()
    {
        CTEOrderPackage *pkg = NULL;
        // COuchEngine *p = (COuchEngine *)param;
        isTEThreadRunning = true;
        std::cout << GREEN << "void ProcessIncomingTEPackages() run " << RESET << std::endl;
        // uint32_t nId = __sync_add_and_fetch(&p->nZMQFrontEndThreadsRunning, 1);
        // p->journal.WriteLog("[COuchEngine::ProcessIncomingTEPackages] - Starting ProcessIncomingTEPackages, nId : %lu;\n", nId);
        while (isTEThreadRunning)
        {
            if (NULL == (pkg = incoming_te_packages.GetfromQueue()))
            {
                // std::cout << YELLOW << "sleep" << RESET << std::endl;
                usleep(100000);
                continue;
            }

            // std::cout << BLUE << "pkg->GetPackageId(): " << pkg->GetPackageId() << RESET << std::endl;
            switch (pkg->GetPackageId())
            {
            case PK_RT_NEW_REGULAR_ORDER:
                std::cout << RED << "PK_RT_NEW_REGULAR_ORDER" << RESET << std::endl;
                ProcessNewRegularOrder(pkg);
                break;
            case PK_RT_NEW_CASH_ORDER:
                std::cout << RED << "PK_RT_NEW_CASH_ORDER" << RESET << std::endl;
                // p->ProcessNewCashOrder(pkg);
                break;
            case PK_RT_AMEND_REGULAR_ORDER:
                std::cout << RED << "PK_RT_AMEND_REGULAR_ORDER" << RESET << std::endl;
                // p->ProcessAmendRegularOrder(pkg);
                break;
            case PK_RT_AMEND_CASH_ORDER:
                std::cout << RED << "PK_RT_AMEND_CASH_ORDER" << RESET << std::endl;
                // p->ProcessAmendCashOrder(pkg);
                break;
            case PK_RT_WITHDRAW_REGULAR_ORDER:
                std::cout << RED << "PK_RT_WITHDRAW_REGULAR_ORDER" << RESET << std::endl;
                // p->ProcessWithdrawRegularOrder(pkg);
                break;
            case PK_RT_WITHDRAW_CASH_ORDER:
                std::cout << RED << "PK_RT_WITHDRAW_CASH_ORDER" << RESET << std::endl;
                // p->ProcessWithdrawCashOrder(pkg);
                break;
            }
        }
        std::cout << RED << "[COuchEngine::ProcessIncomingTEPackages] - ProcessIncomingTEPackages Ended, nId " << RESET << std::endl;

        // p->journal.WriteLog("[COuchEngine::ProcessIncomingTEPackages] - ProcessIncomingTEPackages Ended, nId : %lu;\n", nId);
        // __sync_sub_and_fetch(&pnZMQFrontEndThreadsRunning, 1);
    }
    void ProcessNewRegularOrder(CTEOrderPackage *pkg)
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
        slen = pkg->GetWord();
        pkg->GetString(slen, szOrderSource);

        uint32_t orderToken = pkg->GetDWord(), orderBookId = 0;

        std::cout << "szOrderId: " << szOrderId << " szAlt: " << szAlt << " szCustId: " << szCustId << " szStockCode: " << szStockCode << " szInputUser: " << szInputUser << " szComplianceId: " << szComplianceId << " szOrderSource: " << szOrderSource << " price: " << price << " volume: " << volume << " (lembar sham)" << std::endl;

        NewOrder(szOrderId, "1", std::to_string(volume), "1", std::to_string(price), szStockCode);
    }

    void SendStartSendingOrderPackage()
    {
        CTEOrderPackage *o = new CTEOrderPackage(0, PK_RT_START_SENDING_ORDERS);
        // o->SetErrorCode(bReqResendOrders ? 1 : 0);
        outgoing_te_packages.AddtoQueue(o, __TEORDER_QUEUE__::PRIOR_TAIL);
        // outgoing_te_packages.AddtoQueue(o, __TEORDER_QUEUE__::PRIOR_HEAD);
    }

    void ZMQProcessItchSubscription()
    {
        std::cout << "ZMQProcessItchSubscription run" << std::endl;
        // p->journal.WriteLog("[COuchEngine::ZMQProcessItchSubscription] - ZMQProcessItchSubscription Thread started.\n");
        void *context = zmq_ctx_new();
        if (NULL == context)
        {
            std::cout << "ZMQProcessItchSubscription] - Can not create ZMQ Context." << RESET << std::endl;

            // p->journal.WriteLog("[COuchEngine::ZMQProcessItchSubscription] - Can not create ZMQ Context.\n");
            isItchSubscRunning = false;
            return;
        }
        zmq_ctx_set(context, ZMQ_IO_THREADS, 2);
        void *zsock = zmq_socket(context, ZMQ_SUB);
        if (NULL == zsock)
        {
            std::cout << "ZMQProcessItchSubscription] - Can not create zmq socket." << RESET << std::endl;

            // p->journal.WriteLog("[COuchEngine::ZMQProcessItchSubscription] - Can not create zmq socket.\n");
            isItchSubscRunning = false;
            return;
        }
        int nRead = 1000;
        zmq_setsockopt(zsock, ZMQ_LINGER, &nRead, sizeof(nRead));   //	Linger Period of Socket Shutdown
        zmq_setsockopt(zsock, ZMQ_RCVTIMEO, &nRead, sizeof(nRead)); //	Receive time out 1 second
        if (0 != zmq_connect(zsock, "tcp://10.249.250.138:5050"))
        {
            std::cout << "ZMQProcessItchSubscription] - Can not connect to zmq :" << RESET << std::endl;

            // p->journal.WriteLog("[COuchEngine::ZMQProcessItchSubscription] - Can not connect to zmq : %s\n", p->oConfig.zmqConfig.zmq_ITCH_Publisher_Address);
            isItchSubscRunning = false;
            return;
        }
        isItchSubscRunning = true;
        zmq_setsockopt(zsock, ZMQ_SUBSCRIBE, "", 0);
        nRead = 0;
        zmq_msg_t msg;
        pthread_t tid = 0;
        // pthread_create(&tid, NULL, (void *(*)(void *))ProcessItchMessages, p);
        while (isItchSubscRunning)
        {
            if (-1 != nRead)
                zmq_msg_init(&msg);
            if (-1 == (nRead = zmq_msg_recv(&msg, zsock, 0)))
            {
                // std::cout << "ZMQProcessItchSubscription] sleep" << RESET << std::endl;

                usleep(100000);
                continue;
            }
            itchQ.AddtoQueue(new CItch((char *)zmq_msg_data(&msg), nRead, true));
            zmq_msg_close(&msg);
            nRead = 0;
        }
        isItchSubscRunning = false;
        if (nRead > 0)
            zmq_msg_close(&msg);
        // _thread_itchsubscrption->join();
        // pthread_join(tid, NULL);
        zmq_close(zsock);
        zmq_ctx_destroy(context);
        // p->journal.WriteLog("[COuchEngine::ZMQProcessItchSubscription] - ZMQProcessItchSubscription Thread ended.\n");
    }

    void ZMQAOProxyFrontendThread()
    {
        int hwm = 0;
        // char host[128] = "tcp://localhost:5580";
        const char *message = "Hello from sender!";
        // COuchEngine *p = (COuchEngine *)this;
        // uint32_t nId = __sync_add_and_fetch(&p->nZMQFrontEndThreadsRunning, 1);
        // p->journal.WriteLog("[COuchEngine::ZMQAOProxyFrontendThread] - Starting ZMQAOProxyFrontendThread, nId : %lu;\n", nId);
        void *context = zmq_ctx_new();
        if (NULL == context)
        {
            std::cout << RED << " Can not create ZMQ Context, ZMQAOProxyFrontendThread Ended." << RESET << std::endl;
            // p->journal.WriteLog("Can not create ZMQ Context, ZMQAOProxyFrontendThread Ended.\n");
            return;
        }
        zmq_ctx_set(context, ZMQ_IO_THREADS, 2);
        void *zsock = zmq_socket(context, ZMQ_DEALER);
        if (NULL == zsock)
        {
            // p->journal.WriteLog("[COuchEngine::ZMQAOProxyFrontendThread] - Can not create ZMQ Socket, ZMQAOProxyFrontendThread Ended.\n");
            std::cout << RED << " Can not create ZMQ Socket, ZMQAOProxyFrontendThread Ended" << RESET << std::endl;

            return;
        }
        // p->journal.WriteLog("[COuchEngine::ZMQAOProxyFrontendThread] - Set ZMQ Identity : %s.\n", ZMQ_AOPROXY_IDENTITY);
        zmq_setsockopt(zsock, ZMQ_IDENTITY, ZMQ_AOPROXY_IDENTITY, strlen(ZMQ_AOPROXY_IDENTITY));
        zmq_setsockopt(zsock, ZMQ_RCVHWM, &hwm, sizeof(hwm));
        zmq_setsockopt(zsock, ZMQ_SNDHWM, &hwm, sizeof(hwm));
        zmq_setsockopt(zsock, ZMQ_LINGER, &(hwm = 100), sizeof(hwm)); //	Set Linger 100 mseconds
        zmq_setsockopt(zsock, ZMQ_RCVTIMEO, &hwm, sizeof(hwm));       //	Receive time out 100 mseconds
        std::cout << RED << "  Connect ZMQ Frontend :" << RESET << std::endl;

        // p->journal.WriteLog("[COuchEngine::ZMQAOProxyFrontendThread] - Connect ZMQ Frontend : %s.\n", p->oConfig.zmqConfig.zmq_AOProxy_Frontend);
        int result = zmq_connect(zsock, "tcp://10.249.250.138:5580");
        if (-1 == zmq_connect(zsock, "tcp://10.249.250.138:5580"))
        {
            std::cout << RED << " Failed to connect to ZMQ frontend" << RESET << std::endl;
        }
        else
        {
            std::cout << GREEN << " koneksi sukses" << RESET << std::endl;
        }

        int size = 0;
        zmq_msg_t msg;
        time_t lastTime = time(NULL);
        isTEThreadRunning = true;
        // for (uint16_t i = 0; i < p->oConfig.zmqConfig.frontend_process_thread_count; i++)

        while (isTEThreadRunning)
        {
            // std::cout << "tes isTEThreadRunning" << std::endl;
            if (-1 != size)
                zmq_msg_init(&msg);
            size = zmq_msg_recv(&msg, zsock, 0);
            if (-1 == size)
            {
                // Menangani kasus ketika tidak ada pesan yang diterima dalam jangka waktu tertentu
                uint32_t nSent = 0;
                if (difftime(time(NULL), lastTime) >= 1)
                {
                    lastTime = time(NULL);
                    CTEOrderPackage o(1, PK_HTS_HEART_BEAT);
                    o.AddByte(1);
                    // std::cout << "mengirim message di zmq1" << std::endl;

                    zmq_send(zsock, (const char *)o, o.GetPackageLength(), 0);
                    // std::cout << "msg: " << (const char *)o << " length: " << o.GetPackageLength() << std::endl;
                    // zmq_send(zsock, message, strlen(message), 0);
                }
                CTEOrderPackage *pkg = NULL;
                while (NULL != (pkg = outgoing_te_packages.GetfromQueue()))
                {
                    std::cout << "mengirim message di zmq2" << std::endl;
                    zmq_send(zsock, (const char *)*pkg, pkg->GetPackageLength(), 0);
                    if (!isTEThreadRunning || ++nSent > 100)
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
                incoming_te_packages.AddtoQueue(pkg);
            }
            zmq_msg_close(&msg);
        }
        zmq_close(zsock);
        zmq_ctx_destroy(context);
        std::cout << RED << "  ZMQAOProxyFrontendThread Ended" << RESET << std::endl;

        // p->journal.WriteLog("[COuchEngine::ZMQAOProxyFrontendThread] - ZMQAOProxyFrontendThread Ended, nId : %lu;\n", nId);
        // __sync_sub_and_fetch(&p->nZMQFrontEndThreadsRunning, 1);
    }

    void logWrite(std::string log, std::string color, std::string mode)
    {
        // WriteLog *logWrite = new WriteLog();

        netstat net;
        // std::cout << logWrite.timecurrent() << std::endl;
        std::string netprint = net.exec("netstat -an | grep 59881");
        if (netprint.size() <= 0)
        {
            LogWrite->logMessage(log + " => " + "no connection occurs", color, mode, 1);
        }
        else
        {
            LogWrite->logMessage(log, color, mode, 1);
        }
        //  logWrite.logMessage(log);
    }

    void runIOService(boost::asio::io_service &io_service)
    {
        // std::cout << "runIOService Running: " << std::endl;
        io_service.run();
        io_service.reset();
    }

    bool checkSchedule()
    {

        if (1 == 1)
        {
            /* code */
        }
    }

    std::string sendTestRequest()
    {
        std::string MsgSeqNum = std::to_string(MsgNum);
        FIX::Message message;
        message.getHeader().setField(8, "FIXT.1.1");   // BeginString
        message.getHeader().setField(35, "1");         // MsgType
        message.getHeader().setField(49, _senderid);   // SenderCompID VP
        message.getHeader().setField(56, _targetComp); // TargetCompID, with enumeration, FIX::FIELD::TargetCompID IDX
        message.getHeader().setField(34, MsgSeqNum);
        message.getHeader().setField(43, "N");
        message.getHeader().setField(52, currentTime());
        // message.setField(98, "0");
        // message.setField(108, "45");
        message.setField(112, currentTime());
        // message.setField(1137, "8");

        FIX::Message checkMsg(message.toString(), _dd, true);
        _fix_raw_logon_string = message.toString();
        queue_send.push(_fix_raw_logon_string);
        logWrite("sendTestRequest: " + message.toString(), RED, outp);

        return message.toString();
    }

    std::string ResendRequest(int SeqSeverSekarang)
    {
        std::string MsgSeqNum = std::to_string(MsgNum);
        FIX::Message message;
        message.getHeader().setField(8, "FIXT.1.1");   // BeginString
        message.getHeader().setField(35, "2");         // MsgType
        message.getHeader().setField(49, _senderid);   // SenderCompID VP
        message.getHeader().setField(56, _targetComp); // TargetCompID, with enumeration, FIX::FIELD::TargetCompID IDX
        message.getHeader().setField(34, MsgSeqNum);
        message.getHeader().setField(43, "N");
        message.getHeader().setField(52, currentTime());
        message.setField(7, std::to_string(SeqSeverSekarang));
        message.setField(16, "");

        FIX::Message checkMsg(message.toString(), _dd, true);
        _fix_raw_logon_string = message.toString();
        queue_send.push(_fix_raw_logon_string);
        logWrite("sendTestRequest: " + message.toString(), RED, outp);

        return message.toString();
    }

    void logon(const std::string user, const std::string pass)
    {
        Logon(createLogon(user, pass));
    }

    void logout()
    {
        Logout(CreateLogout("User Request Logout"));
    }

    // void printpritTest()
    // {
    //     try
    //     {
    //         SequenceReset();
    //     }
    //     catch (const std::exception &e)
    //     {
    //         std::cerr << e.what() << '\n';
    //     }
    // }

    // void reset4()
    // {
    //     start();
    //     SequenceReset();
    // }

    bool is_connect() { return _connect; }

    void val_conf(std::string targetComp, std::string senderId)
    {
        _targetComp = targetComp;
        _senderid = senderId;
    }

    void printht()
    {
        sendHeartBtInt();
    }

    void printReplaceSoh()
    {
        std::cout << GREEN << Replace_Soh(sendHeartBtInt()) << RESET << std::endl;
        // Replace_Soh(sendHeartBtInt());
    };

public:
    WriteLog *LogWrite = new WriteLog();

    int MsgNum;
    int MsgNumServer;
    bool _active;
    bool _logoutInProgress;
    typedef CGQueue<CTEOrderPackage> __TEORDER_QUEUE__;
    __TEORDER_QUEUE__ outgoing_te_packages, incoming_te_packages;

    typedef CGQueue<CItch> _ITCHQ_;
    _ITCHQ_ itchQ;

    // bool _HouseKeeping;

private:
    std::string Replace_Soh(std::string _fix_raw_logon_string)
    {
        // FIXT.1.19=007135=A34=1049=IDX56=VP52=20240119-01:42:50.43250698=0108=451137=810=066
        std::string soh;
        size_t pos = 0;
        while ((pos = _fix_raw_logon_string.find(FIXMSG_DELIMITER)) != string::npos)
        {
            soh += _fix_raw_logon_string.substr(0, pos) + REPLACE_SOH;
            _fix_raw_logon_string.erase(0, pos + FIXMSG_DELIMITER.length());
        }
        return soh;
    }
    ///////////////////////////block to connect to the server [BEGIN]/////////////////////////////////////
    void connect_start(boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
    {
        boost::asio::ip::tcp::tcp::endpoint endpoint = *endpoint_iterator;
        socket_.reset(new boost::asio::ip::tcp::socket(ioService_));
        socket_->async_connect(endpoint, boost::bind(&FixSession::handleConnect, this, boost::asio::placeholders::error));
        // _connect_timer.expires_from_now(_connection_timeout);
        // _connect_timer.async_wait(boost::bind(&FixSession::do_close, this, boost::asio::placeholders::error));
    }

    bool IsActive() { return _active; }

    void handleConnect(const boost::system::error_code &ec)
    {
        if (!ec) // not error
        {
            std::cout << "connection to the server was successful" << std::endl;
            logWrite("connection to the server was successful", GREEN, outp);
            _connect = true;
            _active = true;
            isTEThreadRunning = true;
            // _connect_timer.cancel();
            handle_connected(ec);
            readFixMessage();
        }
        else
        {
            _active = false;
            logWrite("error connecting to the fix server: " + ec.message(), RED, outp);
            do_close(ec);
            std::cerr << "Error connecting to the FIX server: " << RED << ec.message() << RESET << std::endl;
        }
    }

    void handle_connected(const boost::system::error_code &err)
    {
        try
        {

            _loggedOn = false;
            _loggedOnJATS = false;
            _logonInProgress = false;
            _logoutInProgress = false;
            _initiateLogon = false;
            _initiateLogout = false;
            _SendHtbt = false;
            _SendTestRequest = false;

            _initiateLogon = true;
            _logonInProgress = true;

            // logon(FileCustom::myVector[4], FileCustom::myVector[6]);

            if (!_writestring_msgs.empty())
            {
                std::cout << "tidak kosong di handle conect " << Replace_Soh(_writestring_msgs.front()) << std::endl;
                this->do_writestr(_writestring_msgs.front());
            }

            // std::cout << "message on handle_connected(): " + _fix_raw_logon_string << std::endl;
            logWrite("message on handle_connected(): " + Replace_Soh(_fix_raw_logon_string), BLUE, outp);
            // delete oMsg;
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << '\n';
            std::string error = e.what();
            logWrite("error on handle_connected()" + error, RED, outp);
        }
    }
    ///////////////////////////block to connect to the server [END]/////////////////////////////////////////

    ///////////////////////// to send to the server [BGEGIN]////////////////////////////////////////////////

    void do_writestr(const std::string &message)
    {
        // std::cout << "on do_writestr()" << std::endl;
        ioService_.post(boost::bind(&FixSession::sendFixMessage, this, message));
    }

    void sendFixMessage(std::string &message)
    {

        bool write_in_progress = !_writestring_msgs.empty();
        _writestring_msgs.push_back(message);

        std::cout << "_writestring_msgs on senFixMessage(): " << Replace_Soh(_writestring_msgs.front()) << std::endl;
        FileCustom::saveSeqNum(FixSession::MsgNum, FixSession::MsgNumServer);
        LogWrite->SaveToDb(_writestring_msgs.front());

        logWrite(Replace_Soh(_writestring_msgs.front()), BLUE, outp);

        if (!write_in_progress)
        {
            boost::asio::async_write(*socket_, boost::asio::buffer(_writestring_msgs.front(), _writestring_msgs.front().size()), boost::bind(&FixSession::writestr_complete, this, boost::asio::placeholders::error));
        }
        else
        {
            std::cout << "_writestring_msgs: " << _writestring_msgs.front() << std::endl;
        }
    }

    void writestr_complete(const boost::system::error_code &error)
    {

        if (!error)
        {
            _writestring_msgs.pop_front();
            if (!_writestring_msgs.empty())
            {
                std::cout << RED << "masih ada pesan yang belum terkirim di writestr_complete()" << RESET << std::endl;
                // logWrite("masih ada pesan yang belum terkirim di writestr_complete()", GREEN, outp);
                boost::asio::async_write(*socket_, boost::asio::buffer(_writestring_msgs.front(), _writestring_msgs.front().size()), boost::bind(&FixSession::writestr_complete, this, boost::asio::placeholders::error));
            }
            else
            {
                // logWrite("send message to server success, write complete: ", GREEN, outp);
            }
        }
        else
        {
            logWrite(" write error : " + error.message(), RED, outp);
            std::cerr << "Write error: " << error.message() << std::endl;
            // do_close(error);
        }
    }

    void handle_send_thread()
    {
        try
        {
            // logWrite("handle send thread() try: ", MAGENTA,outp);

            while (bsendthreadrun)
            {
                // logWrite("handle send thread() while (bsendthreadrun): ", MAGENTA,outp);

                if (IsActive())
                {
                    // logWrite("handle send thread() if(IsActive()): ", MAGENTA,outp);
                    std::string senddata;
                    while (queue_send.try_pop(senddata))
                    {
                        // logWrite("handle send thread()  while (queue_send.try_pop(senddata)): ", MAGENTA, outp);

                        do_writestr(senddata);
                    }
                }
                // // ------------------
                boost::posix_time::milliseconds idletime(_sendsleep);
                boost::this_thread::sleep(idletime);
            }
        }
        catch (std::exception &e)
        {
            std::cout << "error: " << e.what() << std::endl;
        }
    }
    ////////////////////// to send to the server [END]//////////////////////////////////////////////////

    ///////////////////////////////////// to read server message [BEGIN]/////////////////////////////////
    void readFixMessage()
    {
        // logWrite("di fungsi readFixMessage()", MAGENTA, inp);
        socket_->async_read_some(boost::asio::buffer(_read_msg, max_read_length),
                                 boost::bind(&FixSession::read_complete, this,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    }

    void read_complete(const boost::system::error_code &error, size_t bytes_transferred)
    {
        // the asynchronous read operation has now completed or failed and returned an error
        if (error)
        {
            std::cout << RED << "error read complete: " << error.message() << RESET << std::endl;
            logWrite("   error read on read_complete(): " + error.message(), RED, inp);
            _active = false;
            // FixSession::start();
            do_close(error);
        }
        else
        {
            // read completed, so process the data
            std::string receivemsg(_read_msg, bytes_transferred);
            logWrite(Replace_Soh(receivemsg), GREEN, inp);
            // std::cout << "reading message from server: " + receivemsg << std::endl;
            handle_receive(receivemsg);
            readFixMessage();
        }
    }

    void handle_receive(const std::string recv)
    {

        try
        {
            if (!recv.empty())
            {
                std::string res(recv);
                queue_receive.push(res);
                // std::cout << GREEN << "receive message on handle_receive(): " + res << RESET << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::string error = e.what();
            std::cout << RED << "error handle_receive(): " + error << RESET << std::endl;
            std::cerr << e.what() << '\n';
        }
    }

    void handle_receive_thread()
    {
        try
        {
            while (brecvthreadrun)
            {
                std::string receivedata;
                std::string sraw;
                std::string getChecksum;
                std::string posEnd;
                std::string find = FIXMSG_DELIMITER + "10=";

                if (queue_receive.try_pop(receivedata))
                {
                    std::cout << MAGENTA << "receive message on handle_receive_thread(): " << Replace_Soh(receivedata) << RESET << std::endl;
                    sraw = _incomplete_raw + receivedata;
                    _incomplete_raw = "";
                    bool bLoop = true;

                    while (bLoop)
                    {
                        // idx::msg::IDXMessage *oMsg = new idx::msg::IDXMessage(_protocol_version);
                        // oMsg->SetData(receivedata);
                        // getChecksum = oMsg->GetCheckSum();
                        size_t posEnd, posStart;
                        size_t found = sraw.find(find);

                        // if (found != std::string::npos)
                        // {
                        //     std::string smsg;
                        //     idx::GetMsgFix(sraw, smsg, FIXMSG_DELIMITER + "10=");
                        //     std::cout << BLUE << "di temukan di handle_receive_thread(): " << Replace_Soh(smsg) << RESET << std ::endl;
                        //     // std::cout << BLUE << "getCheckSum: " << getChecksum << RESET << std ::endl;
                        //     // Replace_Soh(smsg);
                        //     // internal_parsing_data(smsg);
                        // }

                        posStart = sraw.find(find);
                        if (posStart != std::string::npos)
                        {
                            std::string smsg;
                            posEnd = sraw.find(FIXMSG_DELIMITER, posStart + 4);
                            if (posEnd != std::string::npos)
                            { // 74 + 3 = 77, 83 - 79 = 74
                                std::string bginseq = sraw.substr(0, posEnd + 1);
                                // std::cout << YELLOW << "ori get: " << receivedata << RESET << std ::endl;
                                // std::cout << BLUE << "setelah di parse get: " << Replace_Soh(bginseq) << RESET << std ::endl;
                                internal_parsing_data(bginseq);
                                sraw.erase(0, posStart + find.length());
                                // std::cout << "sqnum: " << bginseq << " pos " << posStart << " posend: " << posEnd << std::endl;
                            }
                        }
                        else
                        {
                            if (!receivedata.empty())
                            {
                                // internal_parsing_data(receivedata);
                            }
                            std::cout << RED << "bLoop false " << RESET << std ::endl;
                            _incomplete_raw = sraw;
                            bLoop = false;
                            // delete oMsg;
                        }
                    }
                }
                // ------------------
                boost::posix_time::milliseconds idletime(_recvsleep);
                boost::this_thread::sleep(idletime);
            }
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    ///////////////////////////////////// to read server message [END]//////////////////////////////////
    bool isGapDetected(int gapSeq) const
    {
        return gapSeq < FileCustom::getmsgSeqNumServer();
    }
    void internal_parsing_data(const std::string data)
    {
        try
        {
            if (!data.empty())
            {
                std::string sMsgType;
                std::string beginSeqNum;
                std::string getChecksum;

                // size_t posStart, posEnd;
                idx::msg::IDXMessage *oMsg = new idx::msg::IDXMessage(_protocol_version);
                oMsg->SetData(data);
                // oMsg->SetRawString(data);
                oMsg->SetSessionName(_connection_name);
                std::string fixmsg(oMsg->GetData());
                sMsgType = oMsg->GetMsgType();
                beginSeqNum = oMsg->GetBeginSeqNo();
                getChecksum = oMsg->GetCheckSum();
                std::string s(oMsg->GetRawString());
                MsgNumServer = std::stoi(oMsg->GetSeqNum());

                if (oMsg->getIsTag58() && oMsg->isSeqnumLow())
                {
                    MsgNum = std::stoi(oMsg->getSeqNumLow());
                    logon(FileCustom::myVector[4], FileCustom::myVector[6]);
                }

                // cek nomor urut server (seqnum)
                if (isGapDetected(MsgNumServer))
                {
                    FileCustom::saveSeqNum(MsgNum, MsgNumServer + 1);

                    std::cout << "nomor urut terdeteksi, server" << std::stoi(oMsg->GetSeqNum()) << " file " << FileCustom::getmsgSeqNumServer()
                              << std::endl;
                    // FixSession::ResendRequest(FileCustom::getmsgSeqNumServer());
                }

                std::cout << YELLOW << "parsing data beginSeqNum: " << RESET << GREEN << beginSeqNum << RESET << YELLOW << " sMsgType: " << RESET << GREEN << sMsgType << RESET << YELLOW << " checksum: " << RESET << GREEN << getChecksum << RESET << std::endl;
                if (sMsgType == "A") // Logon
                {
                    if (!_loggedOn && _logonInProgress)
                    {
                        std::cout << GREEN << "LOGON SUKSES" << RESET << std::endl;
                        // MsgNum++;
                        // SequenceReset();
                        _loggedOn = true;
                        _loggedOn2 = true;
                    }
                }
                else if (sMsgType == "0")
                {
                    std::cout << BLUE << "menerima pesan heartbeat dari server:" << RESET << std::endl;
                    if (_logoutInProgress)
                    {
                        std::cout << BLUE << "User Meminta Logout" << RESET << std::endl;

                        MsgNum++;
                        CreateLogout("User Request Logout");
                        _active = false;
                    }
                    else
                    {
                        // _loggedOn = false;
                        if (!FixSession::_TypeAn)
                        {
                            std::cout << GREEN << "mengirim pesan heartbeat ke server:" << RESET << std::endl;
                            MsgNum++;
                            sendHeartBtInt();
                        }
                        _logoutInProgress = false;
                    }
                }
                else if (sMsgType == "2")
                {
                    // MsgNum++;
                    SequenceReset(beginSeqNum);
                    _SendHtbt = true;
                }

                else if (sMsgType == "5")
                {
                    if (_loggedOn)
                    {
                        if (_initiateLogout || !_loggedOnJATS)
                        {
                            std::cout << BLUE << "sedang logout  dari server:" << RESET << std::endl;
                            MsgNum++;
                            CreateLogout("confirming logout");
                            _loggedOn = false;
                            Close();
                        }
                    }
                    else
                    {
                        if (_initiateLogon)
                        {
                            _loggedOn = false;
                            Close();
                        }
                    }
                    std::cout << "logout" << std::endl;
                    Close();
                }
                else if (sMsgType == "e") // Error
                {
                    std::cout << RED << "ERROR" << RESET << std::endl;
                }
                else if (sMsgType == "d") // Disconnect
                {
                    _loggedOn = false;
                    std::cout << RED << "DISCONNECT" << RESET << std::endl;
                }
                // std::cout << BLUE << "delete oMsg" << RESET << std::endl;
                delete oMsg;
                FileCustom::saveSeqNum(MsgNum, MsgNumServer);
            }
        }
        catch (std::exception &e)
        {
            std::cout << "error: " << e.what() << std::endl;
        }
    }

    std::string SequenceReset(std::string beginSeqNum)
    {

        std::string MsgSeqNum = std::to_string(MsgNum);
        std::string gapfill = std::to_string(MsgNum + 5);

        FIX::Message message;
        message.getHeader().setField(8, "FIXT.1.1");   // BeginString
        message.getHeader().setField(35, "4");         // MsgType
        message.getHeader().setField(49, _senderid);   // SenderCompID VP
        message.getHeader().setField(56, _targetComp); // TargetCompID, with enumeration, FIX::FIELD::TargetCompID IDX
        message.getHeader().setField(34, beginSeqNum);
        message.getHeader().setField(43, "Y");
        message.getHeader().setField(52, currentTime());
        message.getHeader().setField(122, currentTime());
        message.getHeader().setField(123, "Y");
        message.setField(36, gapfill);
        // message.setField(1137, "8");
        MsgNum = MsgNum + 4;
        FIX::Message checkMsg(message.toString(), _dd, true);
        // std::cout << GREEN << "tag 36: " << gapfill << " " << MsgNum << RESET << std::endl;

        std::cout << BLUE << "message  SequenceReset(): " << Replace_Soh(message.toString()) << RESET << std::endl;

        _fix_raw_logon_string = message.toString();
        queue_send.push(_fix_raw_logon_string);
        return message.toString();
    }

    std::string sendHeartBtInt()
    {

        std::string MsgSeqNum = std::to_string(MsgNum);
        FIX::Message message;

        message.getHeader().setField(8, "FIXT.1.1");   // BeginString
        message.getHeader().setField(35, "0");         // MsgType
        message.getHeader().setField(49, _senderid);   // SenderCompID VP
        message.getHeader().setField(56, _targetComp); // TargetCompID, with enumeration, FIX::FIELD::TargetCompID IDX
        message.getHeader().setField(34, MsgSeqNum);
        message.getHeader().setField(43, "N");
        message.getHeader().setField(52, currentTime());
        FIX::Message checkMsg(message.toString(), _dd, true);
        _fix_raw_logon_string = message.toString();
        queue_send.push(_fix_raw_logon_string);

        // std::cout << BLUE << "message heartbeart: " << message.toString() << RESET << std::endl;

        return message.toString();
    }

    void Logon(CSTRING fix_raw_logon)
    {
        // std::cout << "logon function has been called:..." << std::endl;
        _fix_raw_logon_string = fix_raw_logon;
        queue_send.push(fix_raw_logon);
        // FixSession::start();
    }

    std::string createLogon(const std::string user, const std::string pass)
    {
        std::string MsgSeqNum = std::to_string(MsgNum);
        FIX::Message message;

        message.getHeader().setField(8, "FIXT.1.1");   // BeginString
        message.getHeader().setField(35, "A");         // MsgType
        message.getHeader().setField(49, _senderid);   // SenderCompID VP
        message.getHeader().setField(56, _targetComp); // TargetCompID, with enumeration, FIX::FIELD::TargetCompID IDX
        message.getHeader().setField(34, MsgSeqNum);
        message.getHeader().setField(43, "N");

        message.getHeader().setField(52, currentTime());
        message.setField(98, "0");
        message.setField(108, "15");
        message.setField(553, user); // vpfc1001
        message.setField(554, pass); // jakarta123
        message.setField(1137, "8");

        FIX::Message checkMsg(message.toString(), _dd, true);
        logWrite("message from create logon: " + message.toString(), GREEN, outp);
        // std::cout << "message from create logon: " + message.toString() << std::endl;
        // std::cout << "checksum: " + checksum << std::endl;

        return message.toString();
    }

    void Logout(CSTRING fix_raw_logout)
    {
        _logoutInProgress = true;
        _initiateLogout = true;

        _fix_raw_logon_string = fix_raw_logout;
        // FixSession::start();
    }

    std::string CreateLogout(std::string reason)
    {
        std::cout << RED << "LOGOUT LOGOUT" << RESET << std::endl;

        std::string MsgSeqNum = std::to_string(MsgNum);
        FIX::Message message;
        message.getHeader().setField(8, "FIXT.1.1");   // BeginString
        message.getHeader().setField(49, _senderid);   // SenderCompID
        message.getHeader().setField(56, _targetComp); // TargetCompID, with enumeration
        message.getHeader().setField(35, "5");         // MsgType
        message.getHeader().setField(34, MsgSeqNum);   // seqnum asal aja soalnya sampe di server nanti di recreate lagi kok!
        message.getHeader().setField(52, currentTime());

        message.setField(58, reason); // userid

        FIX::Message checkMsg(message.toString(), _dd, true);
        _fix_raw_logon_string = message.toString();
        queue_send.push(_fix_raw_logon_string);
        return message.toString();
    }

    void SendMessage(CSTRING fix_raw_message)
    {
        std::string fix_raw(fix_raw_message);
        idx::msg::IDXMessage *oMsg = new idx::msg::IDXMessage(_protocol_version);
        oMsg->SetData(fix_raw);
        std::string s(oMsg->GetRawString());
        queue_send.push(s);
        delete oMsg;
    }

    void do_close(const boost::system::error_code &error)
    {
        // something has gone wrong, so close the socket & make this object inactive
        if (error == boost::asio::error::operation_aborted) // if this call is the result of a timer cancel()
            logWrite("connection closed: operation aborted", RED, outp);
        return; // ignore it because the connection cancelled the timer
        if (error)
        {
            std::string s = error.message();
            logWrite("connection closed: " + s, RED, outp);
            handle_error(error.message(), error);
        }
        socket_->close();
        _connect = false;
        _active = false;
        try
        {
            FixSession::start();
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    void handle_error(const std::string errmsg, const boost::system::error_code &err)
    {
        try
        {
            _connect = false;
            _loggedOn = false;
            _active = false;
            // _loggedOn = true;
            std::string errs = "koneksi Error bro: " + errmsg;
        }
        catch (std::exception &e)
        {
            std::cout << "error: " << e.what() << std::endl;
        }
    }

    void Close()
    {
        ioService_.post(boost::bind(&FixSession::do_close, this, boost::system::error_code()));
    }

private:
    // declaration variable
    boost::thread *_thread_service;
    boost::asio::io_service &ioService_;
    FIX::DataDictionary &_dd;
    boost::atomic<float> _sendsleep;
    boost::atomic<float> _recvsleep;
    // boost::asio::ip::tcp::socket socket_;

    std::string targetIP_;
    std::string targetPort_;
    std::string username_;
    std::string password_;

    std::string _senderid;
    std::string _targetComp;
    static const int max_read_length = 1024;
    char _read_msg[max_read_length];

    boost::asio::deadline_timer _connect_timer;
    boost::posix_time::time_duration _connection_timeout;

    std::string _MsgType;
    std::string _SessionName;
    std::string _fix_raw_logon_string;
    std::string _connection_name = "VP";
    std::string _protocol_version = "IDXEQ";
    tbb::concurrent_queue<std::string> queue_receive;
    tbb::concurrent_queue<std::string> queue_send;
    std::deque<std::string> _writestring_msgs;

    boost::atomic<bool> bsendthreadrun;
    boost::atomic<bool> brecvthreadrun;
    boost::atomic<bool> bsendTypeAn;
    boost::atomic<bool> conCheck;
    boost::atomic<bool> zmq_thread;
    boost::atomic<bool> incomingpackage_thread;
    // boost::atomic<uint32_t> nZMQFrontEndThreadsRunning;

    boost::thread *_thread_send;
    boost::thread *_thread_receive;
    boost::thread *_thread_TypeAN;
    boost::thread *_thread_conCheck;
    boost::thread *_thread_zmq;
    boost::thread *_thread_incomingpackage;
    boost::thread *_thread_itchsubscrption;

    std::string _incomplete_raw;
    boost::shared_ptr<boost::asio::ip::tcp::socket> socket_;

    bool _connect;
    bool _loggedOn = false;
    bool _loggedOn2 = false;
    bool _SendHtbt;
    bool _SendTestRequest;
    bool _TypeAn;
    bool isTEThreadRunning;
    bool isItchSubscRunning;

    bool _loggedOnJATS;
    bool _logonInProgress;
    bool _initiateLogon;
    bool _initiateLogout;

    uint32_t lCurDate, //   yyyymmdd
                       // lCurTime, //   hhmmssxxx
        weekDay,       //   0 : Sunday; 1 : Monday; 6 : Saturday
        tokenId, SHARES_LOT;
    char szCurDate[8];
};

void displayRunningText(std::string text)
{
    while (true)
    {
        int consoleWidth = 100; // Set to your desired console width

        // Calculate the number of spaces needed to center the text
        int spaces = (consoleWidth - text.length()) / 2;
        // Tampilkan teks di console
        std::cout << std::setw(spaces + text.length()) << text << " " << std::flush; // Add a space to clear any remaining characters

        // Geser teks ke kiri
        char temp = text[0];
        std::string newText = text.substr(1) + temp;

        // Tunggu sejenak (dalam milidetik)
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Kembali ke awal baris dengan carriage return
        std::cout << "\r" << std::flush;

        text = newText;
    }
}

int main(int argc, char *argv[])
{
    std::string filesconfig = argv[1];
    std::string fileDatadictionary = argv[2];
    boost::asio::io_service ioService;
    std::ifstream is(fileDatadictionary, std::ios::in);
    std::string userinput;
    // FileCustom conf;
    // FixSession._HouseKeeping = conf.HouseKeeping();
    std::cout << YELLOW << "reading configuration file at a specified path: " << RESET << "" MAGENTA << filesconfig << RESET << std::endl;
    if (!is.is_open())
    {
        std::cout << RED << "The file " << fileDatadictionary << " cannot be opened" << RESET << std::endl;
        return 1;
    }
    FIX::DataDictionary dd(is);

    if (FileCustom::ReadConFile(filesconfig))
    {

        //  conf.printFile();
        // conf.myVector[2] = "test change";
        // conf.writeToXml();
        // conf.printFile();
        try
        {
            FixSession fixSession(ioService, dd, FileCustom::myVector[0], FileCustom::myVector[1], FileCustom::myVector[4], FileCustom::myVector[6], 100, 100); ////test to jats server
            fixSession.MsgNum = FileCustom::getmsgSeqNum();
            // fixSession.start();
            fixSession.val_conf(FileCustom::myVector[3], FileCustom::myVector[2]);
            // fixSession.aktif();

            // fixSession.printCreateLoogn();
            while (true)
            {
                std::cout << "input your choice: i for login, o for logout, d new order, v marketdata c for exit" << std::endl;
                std::cout << "active? " << fixSession._active << std::endl;

                // std::cout << "\n";
                // std::thread textThread(displayRunningText, "input your choice: i for login, o for logout, c for exit");
                if (fixSession.is_connect())
                {
                    std::cin >> userinput;
                    // textThread.detach();
                    if (userinput == "i" or userinput == "I")
                    {
                        fixSession.MsgNum++;
                        fixSession.logon(FileCustom::myVector[4], FileCustom::myVector[6]);

                        // fixSession.reset4();
                        // fixSession.printReplaceSoh();
                        // fixSession.printpritTest();
                        // fixSession.sendHeartBeat();
                        // fixSession.printCreateLoogn();
                    }
                    else if (userinput == "o" or userinput == "O")
                    {
                        fixSession.MsgNum++;
                        fixSession._logoutInProgress = true;
                        fixSession.sendTestRequest();
                    }
                    else if (userinput == "d" or userinput == "D")
                    {
                        // fixSession.MsgNum++;
                        // fixSession.NewOrder();
                    }
                    else if (userinput == "v" or userinput == "V")
                    {
                        fixSession.MsgNum++;
                        fixSession.MarketData();
                    }
                    else if (userinput == "m" or userinput == "M")
                    {
                        fixSession.MsgNum++;
                        fixSession.MassQuote();
                    }
                    else if (userinput == "a" or userinput == "A")
                    {
                        fixSession.MsgNum++;

                        fixSession.TypeAE();
                        // fixSession.aktif();
                    }
                    else if (userinput == "ca" or userinput == "CA")
                    {
                        fixSession.MsgNum++;

                        fixSession.MassQuoteActionRequest();
                        // fixSession.aktif();
                    }
                    else if (userinput == "c" or userinput == "C")
                    {
                        std::cout << RED << "exit the application" << RESET << std::endl;
                        // return EXIT_FAILURE;
                        std::exit(EXIT_SUCCESS);
                    }
                }
                FileCustom::saveSeqNum(fixSession.MsgNum, fixSession.MsgNumServer);
            }
            // ioService.run();
        }
        catch (std::exception &e)
        {
            // std::fclose(file);
            std::string error = e.what();
            // fixSession.logWrite("exception: " + error, RED);
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }
    return 0;
}
