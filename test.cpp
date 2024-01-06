#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <sstream>

#include <deque>
#include <pthread.h>
#include <signal.h>

#include <boost/atomic.hpp>
#include <tbb/concurrent_queue.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "inc/WriteLog.hpp"
#include "inc/netstat.hpp"
#include "include/IDXConstants.h"
#include "include/IDXMessage.h"
#include "include/Message.h"
#include <tbb/concurrent_queue.h>
#include "include/DataDictionary.h"
#include "include/FixFieldNumbers.h"
#include "include/IDXUtil.h"

#include "include/md5.hpp"

// Warna ANSI Escape Codes
#define RESET     "\033[0m"
#define BLACK     "\033[30m"
#define RED       "\033[31m"
#define GREEN     "\033[32m"
#define YELLOW    "\033[33m"
#define BLUE      "\033[34m"
#define MAGENTA   "\033[35m"
#define CYAN      "\033[36m"
#define WHITE     "\033[37m"
#define BOLDBLACK     "\033[1m\033[30m"
#define BOLDRED       "\033[1m\033[31m"
#define BOLDGREEN     "\033[1m\033[32m"
#define BOLDYELLOW    "\033[1m\033[33m"
#define BOLDBLUE      "\033[1m\033[34m"
#define BOLDMAGENTA   "\033[1m\033[35m"
#define BOLDCYAN      "\033[1m\033[36m"
#define BOLDWHITE     "\033[1m\033[37m"


typedef const std::string CSTRING;
typedef boost::asio::io_service IOSERVICE;
class FixSession
{
public:
    // FixSession(boost::asio::io_service &ioService, const std::string &targetIP, const std::string &targetPort, const std::string &username, const std::string &password, FIX::DataDictionary& dd)
    //     : ioService_(ioService),
    //       socket_(ioService),
    //       targetIP_(targetIP),
    //       targetPort_(targetPort),
    //       username_(username),
    //       password_(password),
    //       _dd(dd) {}

    FixSession(boost::asio::io_service &ioService, const std::string &targetIP, const std::string &targetPort, const std::string &username, const std::string &password, FIX::DataDictionary &dd)
        : ioService_(ioService),
          socket_(ioService),
          targetIP_(targetIP),
          targetPort_(targetPort),
          username_(username),
          password_(password),
          _connect_timer(ioService),
          _connection_timeout(boost::posix_time::seconds(3)),
          _dd(dd) {}

    void start()
    {
        // Resolve the target IP and port
        boost::asio::io_service io_service;
        std::cout << "function start() " << std::endl;
        boost::asio::ip::tcp::resolver resolver(ioService_);
        boost::asio::ip::tcp::resolver::query query(targetIP_, targetPort_);
        // auto endpointIterator = resolver.resolve(query);
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
        connect_start(iterator);
        _thread_service = new boost::thread(boost::bind(&FixSession::runIOService, this, boost::ref(ioService_)));
        bsendthreadrun = true;
        brecvthreadrun = true;
        _thread_send = new boost::thread(boost::bind(&FixSession::handle_send_thread, this));
        _thread_receive = new boost::thread(boost::bind(&FixSession::handle_receive_thread, this));

        // threadjoin();
    }

    std::string currentTime()
    {
        WriteLog time;
        return time.timecurrent();
    }

    void logWrite(std::string log, std::string color)
    {
        WriteLog logWrite;
        netstat net;
        // std::cout << logWrite.timecurrent() << std::endl;
        std::string netprint = net.exec("netstat -an | grep 59881");
        if (netprint.size() <= 0)
        {
            logWrite.logMessage(log + " => " + "no connection occurs", color);
        }
        else
        {
            logWrite.logMessage(log + " => " + netprint, color);
        }
        //  logWrite.logMessage(log);
    }
    void runIOService(boost::asio::io_service &io_service)
    {
        std::cout << "runIOService Running: " << std::endl;
        io_service.run();
        io_service.reset();
    }

    void logon(const std::string user, const std::string pass)
    {
        Logon(createLogon(user, pass));
    }

    bool is_connect() { return _connect; }

    void threadjoin()
    {
        if (_thread_service != NULL)
        {
            _thread_service->join();
            delete _thread_service;
        }
    }

private:
    ///////////////////////////block to connect to the server [BEGIN]/////////////////////////////////////
    void connect_start(boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
    {
        boost::asio::ip::tcp::tcp::endpoint endpoint = *endpoint_iterator;
        boost::asio::async_connect(socket_, endpoint_iterator,
                                   boost::bind(&FixSession::handleConnect, this,
                                               boost::asio::placeholders::error));
        _connect_timer.expires_from_now(_connection_timeout);
    }

    bool IsActive() { return _active; }

    void handleConnect(const boost::system::error_code &ec)
    {
        if (!ec) // not error
        {
            std::cout << "connection to the server was successful" << std::endl;
            logWrite("connection to the server was successful", GREEN);
            _connect = true;
            _connect_timer.cancel();
            handle_connected(ec);
            readFixMessage();
        }
        else
        {
            logWrite("error connecting to the fix server: " + ec.message(), RED);
            do_close(ec);
            std::cerr << "Error connecting to the FIX server: " << ec.message() << std::endl;
        }
    }

    void handle_connected(const boost::system::error_code &err)
    {
        try
        {

            idx::msg::IDXMessage *oMsg = new idx::msg::IDXMessage(_protocol_version);
            oMsg->SetData(_fix_raw_logon_string);
            oMsg->SetSessionName(_connection_name);

            std::string s(oMsg->GetRawString());
            s += IDX_END;
            this->do_writestr(s);
            delete oMsg;
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << '\n';
            std::string error = e.what();
            logWrite("error on handle_connected()" + error, RED);
        }
    }
    ///////////////////////////block to connect to the server [END]/////////////////////////////////////////





    ///////////////////////// to send to the server [BGEGIN]////////////////////////////////////////////////

    void do_writestr(const std::string &message)
    {
        ioService_.post(boost::bind(&FixSession::sendFixMessage, this, message));
    }

    void sendFixMessage(std::string &message)
    {
        bool write_in_progress = !_writestring_msgs.empty();
        _writestring_msgs.push_back(message);

        // std::cout << "_writestring_msgs: " << _writestring_msgs.front() << std::endl;
        // logWrite("message: " + message, BLUE);

        if (!write_in_progress)
        {
            boost::asio::async_write(socket_, boost::asio::buffer(_writestring_msgs.front(), _writestring_msgs.front().size()), boost::bind(&FixSession::writestr_complete, this, boost::asio::placeholders::error));
        }else{
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
                std::cout << "masih ada pesan yang belum terkirim di writestr_complete()" << std::endl;
                logWrite("masih ada pesan yang belum terkirim di writestr_complete()", GREEN);
                boost::asio::async_write(socket_, boost::asio::buffer(_writestring_msgs.front(), _writestring_msgs.front().size()), boost::bind(&FixSession::writestr_complete, this, boost::asio::placeholders::error));
            }
            logWrite("send message to server success, write complete: ", GREEN);

        }
        else
        {
            logWrite(" write error : " + error.message(), RED);
            std::cerr << "Write error: " << error.message() << std::endl;
            do_close(error);
        }
    }
    ////////////////////// to send to the server [END]//////////////////////////////////////////////

    

    ///////////////////////////////////// to read server message [BEGIN]/////////////////////////////////
    void readFixMessage()
    {
        logWrite("di fungsi readFixMessage()", MAGENTA);
        socket_.async_read_some(boost::asio::buffer(_read_msg, max_read_length),
                                boost::bind(&FixSession::read_complete, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

    void read_complete(const boost::system::error_code &error, size_t bytes_transferred)
    {
        // the asynchronous read operation has now completed or failed and returned an error
        if (!error)
        {
            // read completed, so process the data
            std::string receivemsg(_read_msg, bytes_transferred);
            logWrite("reading message from server" + receivemsg, GREEN);
            // std::cout << "reading message from server: " + receivemsg << std::endl;
            handle_receive(receivemsg);
            readFixMessage();
        }
        else
        {
            std::cout << "error read complete: " << error.message() << std::endl;
            logWrite("   error read complete: " + error.message(), RED);
            do_close(error);
        }
    }
    ///////////////////////////////////// to read server message [END]//////////////////////////////////

    void handle_receive(const std::string recv)
    {

        try
        {
            if (!recv.empty())
            {
                std::string res(recv);
                queue_receive.push(res);
                std::cout << "res: " + res << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::string error = e.what();
            std::cout << "error" + error << std::endl;
            std::cerr << e.what() << '\n';
        }
    }

    void internal_parsing_data(const std::string data)
    {
        try
        {
            if (!data.empty())
            {
                std::string sMsgType;
                // size_t posStart, posEnd;
                idx::msg::IDXMessage *oMsg = new idx::msg::IDXMessage(_protocol_version);
                oMsg->SetRawString(data);
                oMsg->SetSessionName(_connection_name);
                std::string fixmsg(oMsg->GetData());
                sMsgType = oMsg->GetMsgType();
                if (sMsgType == "A") // Logon
                {
                    if (!_loggedOn && _logonInProgress)
                    {
                        std::cout << "logon successfuly" << std::endl;
                        std::cout << "logon successfuly" << std::endl;
                        _loggedOn = true;

                        // OnReceiveLogon(fixmsg);
                        // OnConnectedToJONES(fixmsg);
                    }
                }
                delete oMsg;
            }
        }
        catch (std::exception &e)
        {
            std::cout << "error: " << e.what() << std::endl;
        }

        // delete oMsg;
    }

    void handle_receive_thread()
    {
        try
        {
            while (brecvthreadrun)
            {
                std::string receivedata;
                std::string sraw;

                if (queue_receive.try_pop(receivedata))
                {
                        std::cout << "run handle_reveive_thread() queue_receive.try_pop(receivedata)" << std::endl;

                    sraw = _incomplete_raw + receivedata;
                    _incomplete_raw = "";

                    bool bLoop = true;
                    while (bLoop)
                    {
                        std::cout << "run handle_reveive_thread()   while (bLoop)" << std::endl;
                        
                        size_t found = sraw.find(IDX_END);
                        if (found != std::string::npos)
                        {
                        std::cout << "run handle_reveive_thread()      if (found != std::string::npos)" << std::endl;

                            std::string smsg;
                            std::cout << "fixmsg =====> " << smsg << std::endl;
                            idx::GetStringNextToken(sraw, smsg, IDX_END);
                            if (!smsg.empty())
                                internal_parsing_data(smsg);
                        }
                        else
                        {
                            _incomplete_raw = sraw;
                            bLoop = false;
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

    void do_close(const boost::system::error_code &error)
    {
        // something has gone wrong, so close the socket & make this object inactive
        if (error == boost::asio::error::operation_aborted) // if this call is the result of a timer cancel()
            return;                                         // ignore it because the connection cancelled the timer
        if (error)
        {

            std::string s = error.message();
            logWrite("connection closed: " + s, RED);
            // handle_error(error.message(), error);
        }

        socket_.close();
        _connect = false;
        // _active = false;
    }

    std::string createLogon(const std::string user, const std::string pass)
    {
        /// construct fix message logon
        FIX::Message message;

        message.getHeader().setField(8, "FIXT.1.1");  // BeginString
        message.getHeader().setField(49, "vpfc1001"); // SenderCompID
        message.getHeader().setField(56, "IDX");      // TargetCompID, with enumeration, FIX::FIELD::TargetCompID
        message.getHeader().setField(35, "A");        // MsgType
        message.setField(98, "0");
        message.setField(108, "0");
        message.setField(553, user);
        message.setField(554, pass);
        message.setField(1137, "8");
        // message.getField(98);

        // int i = message.toString().length();
        // std::cout << "message.toString().length(): " << i << std::endl;
        // std::cout << "datadictionary: " << _dd << std::endl;

        FIX::Message checkMsg(message.toString(), _dd, true);
        // logWrite("message from create logon: " + message.toString() , BLUE);
        std::cout << "message: " + message.toString() << std::endl;
        return message.toString();
    }

    void SendMessage(CSTRING fix_raw_message)
    {
        std::string fix_raw(fix_raw_message);
        idx::msg::IDXMessage *oMsg = new idx::msg::IDXMessage(_protocol_version);
        oMsg->SetData(fix_raw);
        oMsg->SetSessionName(_connection_name);

        std::string s(oMsg->GetRawString());
        s += IDX_END;

        queue_send.push(s);
        delete oMsg;
    }

    void Logon(CSTRING fix_raw_logon)
    {
        _fix_raw_logon_string = fix_raw_logon;
        FixSession::start();
    }

    void handle_send_thread()
    {
        try
        {
            while (bsendthreadrun)
            {

                if (IsActive())
                {
                    std::string senddata;
                    while (queue_send.try_pop(senddata))
                    {
                        do_writestr(senddata);
                    }
                }
                // ------------------
                boost::posix_time::milliseconds idletime(_sendsleep);
                boost::this_thread::sleep(idletime);
            }
        }
        catch (std::exception &e)
        {
            std::cout << "error: " << e.what() << std::endl;
        }
    }

    void handle_error(const std::string errmsg, const boost::system::error_code &err)
    {
        try
        {
            _connect = true;
            // _loggedOn = true;

            std::string errs = "koneksi Error bro: " + errmsg;
        }
        catch (std::exception &e)
        {
            std::cout << "error: " << e.what() << std::endl;
        }
    }
    boost::thread *_thread_service;
    boost::asio::io_service &ioService_;
    boost::asio::ip::tcp::socket socket_;
    std::string targetIP_;
    std::string targetPort_;
    std::string username_;
    std::string password_;
    bool _active;
    static const int max_read_length = 1024;
    char _read_msg[max_read_length];

    boost::asio::deadline_timer _connect_timer;
    boost::posix_time::time_duration _connection_timeout;

    std::string _MsgType;
    std::string _SessionName;
    std::string _fix_raw_logon_string;
    std::string _connection_name = "VP_01";
    std::string _protocol_version = "IDXEQ";
    tbb::concurrent_queue<std::string> queue_receive;
    tbb::concurrent_queue<std::string> queue_send;
    std::deque<std::string> _writestring_msgs;

    boost::atomic<bool> bsendthreadrun;
    boost::atomic<bool> brecvthreadrun;
    boost::atomic<float> _sendsleep;
    boost::atomic<float> _recvsleep;
    boost::thread *_thread_send;
    boost::thread *_thread_receive;
    std::string _incomplete_raw;

    FIX::DataDictionary &_dd;

    // boost::atomic<bool> bsendthreadrun;
    // boost::atomic<bool> brecvthreadrun;
    // std::string _incomplete_raw;
    // boost::atomic<float> _recvsleep;

    bool _connect;
    bool _loggedOn;
    bool _logonInProgress;
};

int main()
{
    boost::asio::io_service ioService;
    std::ifstream is("conf/FIX50SP1-IDX.xml", std::ios::in);
    FIX::DataDictionary dd(is);
    std::string userinput;
    FixSession fixSession(ioService, "172.18.2.213", "59881", "vpfc1001", "jakarta123", dd);

    // std::FILE *file = std::fopen("conf/FIX50SP1-IDX.xml", "rb");
    // std::string MD5FIXML = "d36cb8f0fccd193867eb70d75d57d7a3";
    // if (!file)
    // {
    //     std::cout << "ggal membuka file" << std::endl;
    //     return 1; // Menghentikan program dengan status error
    // }

    // membaca isi xml
    //     int character;
    //      while ((character = std::fgetc(file)) != EOF) {
    //          std::putchar(character);
    //      }

    // fixSession.threadjoin();
    // if (MD5FIXML.compare(md5file(file).c_str()) != 0)
    // {
    //     std::cout << "md5 matched" << std::endl;
    // }
    // else
    // {
    //     std::cout << "md5 has not matched" << std::endl;
    // }

    std::cout << " status: " << (fixSession.is_connect() ? GREEN "connected" RESET : RED "not connect" RESET) << std::endl;
    // FIX::DataDictionary& _dd = dd;

    // Replace "TARGET_IP" and "TARGET_PORT" with your FIX server details
    // "172.18.2.213", "59881"

    try
    {
        // fixSession.start();
        fixSession.logon("vpfc1001", "jakarta123");

        // if (fixSession.is_connect())
        // {
        //     std::cout << "do you wwant to logon? y/n:  ";
        //     std::cin >> userinput;
        //     if (userinput == "y" or userinput == "Y")
        //     {
        //         fixSession.logon("vpfc1001", "jakarta123");
        //     }
        //     else
        //     {
        //         return 1;
        //     }
        // }
        // else
        // {
        //     // reconnect?
        //     fixSession.logWrite("Reconnect.", YELLOW);

        //     fixSession.start();
        // }

        ioService.run();
    }
    catch (std::exception &e)
    {
        // std::fclose(file);
        std::string error = e.what();
        fixSession.logWrite("exception: " + error, RED);
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
