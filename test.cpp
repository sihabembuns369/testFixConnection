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
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <unistd.h>

#include "inc/WriteLog.hpp"
#include "inc/netstat.hpp"
#include "inc/FileCustom.hpp"
#include "include/IDXConstants.h"
#include "include/IDXMessage.h"
#include "include/Message.h"
#include <tbb/concurrent_queue.h>
#include "include/DataDictionary.h"
#include "include/FixFieldNumbers.h"
#include "include/IDXUtil.h"

#include "include/md5.hpp"
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

        if (IsActive() == true)
        {

            try
            {
                brecvthreadrun = true;
                _thread_receive = new boost::thread(boost::bind(&FixSession::handle_receive_thread, this));
                std::cout << "brecvthreadrun run." << std::endl;
                // std::cout << "mencoba menyambung ulang..." << std::endl;
            }
            catch (std::exception &e)
            {
                std::cout << "gagal menyambung..." << std::endl;
                // OnErrorSystem(e);
                cerr << e.what() << endl;
            }

            try
            {
                bsendthreadrun = true;
                _thread_send = new boost::thread(boost::bind(&FixSession::handle_send_thread, this));
                std::cout << "halo halo halo..." << std::endl;
            }
            catch (std::exception &e)
            {
                // OnErrorSystem(e);
                cerr << e.what() << endl;
            }
        }
    }

    ~FixSession() // destructor
    {
        if (_thread_service != NULL)
        {
            brecvthreadrun = false;
            bsendthreadrun = false;

            std::cout << "threadjoind() function has been running: " << std::endl;
            // logWrite("threadjoind() function has be running: ", MAGENTA);

            _thread_service->join();

            _thread_receive->join();
            _thread_send->join();

            delete _thread_service;

            delete _thread_receive;
            delete _thread_send;
        }
        else
        {
            std::cout << "threadjoind()not running: " << std::endl;
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
        // auto endpointIterator = resolver.resolve(query);
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
        connect_start(iterator);
        _thread_service = new boost::thread(boost::bind(&FixSession::runIOService, this, boost::ref(ioService_)));
    }

    std::string currentTime()
    {
        WriteLog time;
        return time.timecurrent();
    }

    void logWrite(std::string log, std::string color, std::string mode)
    {
        WriteLog logWrite;
        netstat net;
        // std::cout << logWrite.timecurrent() << std::endl;
        std::string netprint = net.exec("netstat -an | grep 59881");
        if (netprint.size() <= 0)
        {
            logWrite.logMessage(log + " => " + "no connection occurs", color, mode, 1);
        }
        else
        {
            logWrite.logMessage(log + " => " + netprint, color, mode, 1);
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

    void printCreateLoogn()
    {
        try
        {

            idx::msg::IDXMessage *oMsg = new idx::msg::IDXMessage(_protocol_version);
            oMsg->SetData(_fix_raw_logon_string);
            oMsg->SetSessionName(_connection_name);

            std::string s(oMsg->GetRawString());
            s += IDX_END;
            this->do_writestr(s);

            std::cout << "message on printCreateLoogn(): " + _fix_raw_logon_string << std::endl;
            logWrite("message on  printCreateLoogn(): " + _fix_raw_logon_string, BLUE, outp);
            delete oMsg;
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << '\n';
            std::string error = e.what();
            logWrite("error on handle_connected()" + error, RED, outp);
        }
    }

    bool is_connect() { return _connect; }

    void val_conf(std::string targetComp, std::string senderId)
    {
        _targetComp = targetComp;
        _senderid = senderId;
    }
    int MsgNum;
    // bool _HouseKeeping;

private:
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
            // _connect = true;
            _active = true;

            // _connect_timer.cancel();
            handle_connected(ec);
            readFixMessage();
        }
        else
        {
            logWrite("error connecting to the fix server: " + ec.message(), RED, outp);
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
            this->do_writestr(_fix_raw_logon_string);

            std::cout << "message on handle_connected(): " + _fix_raw_logon_string << std::endl;
            logWrite("message on handle_connected(): " + _fix_raw_logon_string, BLUE, outp);
            delete oMsg;
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
        std::cout << "on do_writestr()" << std::endl;
        ioService_.post(boost::bind(&FixSession::sendFixMessage, this, message));
    }

    void sendFixMessage(std::string &message)
    {
        bool write_in_progress = !_writestring_msgs.empty();
        _writestring_msgs.push_back(message);

        std::cout << "_writestring_msgs on senFixMessage(): " << _writestring_msgs.front() << std::endl;

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
                std::cout << "masih ada pesan yang belum terkirim di writestr_complete()" << std::endl;
                logWrite("masih ada pesan yang belum terkirim di writestr_complete()", GREEN, outp);
                boost::asio::async_write(*socket_, boost::asio::buffer(_writestring_msgs.front(), _writestring_msgs.front().size()), boost::bind(&FixSession::writestr_complete, this, boost::asio::placeholders::error));
            }
            else
            {
                logWrite("send message to server success, write complete: ", GREEN, outp);
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
            logWrite("handle send thread() try: ", MAGENTA, outp);

            while (bsendthreadrun)
            {
                // logWrite("handle send thread() while (bsendthreadrun): ", MAGENTA, outp);

                if (IsActive())
                {
                    // logWrite("handle send thread() if(IsActive()): ", MAGENTA, outp);
                    std::string senddata;
                    while (queue_send.try_pop(senddata))
                    {
                        logWrite("handle send thread()  while (queue_send.try_pop(senddata)): ", MAGENTA, outp);

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
        logWrite("di fungsi readFixMessage()", MAGENTA, inp);
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
            std::cout << "error read complete: " << error.message() << std::endl;
            logWrite("   error read on read_complete(): " + error.message(), RED, inp);
            do_close(error);
        }
        else
        {
            // read completed, so process the data
            std::string receivemsg(_read_msg, bytes_transferred);
            logWrite("reading message from server" + receivemsg, GREEN, inp);
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
                std::cout << "receive message on handle_receive(): " + res << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::string error = e.what();
            std::cout << "error handle_receive(): " + error << std::endl;
            std::cerr << e.what() << '\n';
        }
    }

    void handle_receive_thread()
    {
        try
        {
            logWrite("run handle_reveive_thread() try", MAGENTA, inp);

            while (brecvthreadrun)
            {
                //    logWrite( "run handle_reveive_thread()   while (brecvthreadrun)",MAGENTA);

                std::string receivedata;
                std::string sraw;

                if (queue_receive.try_pop(receivedata))
                {
                    logWrite("run handle_reveive_thread() queue_receive.try_pop(receivedata)", MAGENTA, inp);
                    std::cout << "receive message: " << receivedata << std::endl;

                    sraw = _incomplete_raw + receivedata;
                    _incomplete_raw = "";

                    bool bLoop = true;
                    while (bLoop)
                    {
                        logWrite("run handle_reveive_thread()   while (bLoop)", MAGENTA, inp);

                        size_t found = sraw.find(IDX_END);
                        if (found != std::string::npos)
                        {
                            logWrite("run handle_reveive_thread()      if (found != std::string::npos)", MAGENTA, inp);

                            std::string smsg;
                            std::cout << "fixmsg =====> " << smsg << std::endl;
                            idx::GetStringNextToken(sraw, smsg, IDX_END);
                            if (!smsg.empty())
                                internal_parsing_data(smsg);
                            std::cout << "run handle_reveive_thread(): " << smsg << std::endl;
                            logWrite("run handle_reveive_thread() internal_parsing_data(smsg): " + smsg, BLUE, inp);
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
    ///////////////////////////////////// to read server message [END]//////////////////////////////////

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
                    std::cout << "logon successfuly" << std::endl;
                    if (!_loggedOn && _logonInProgress)
                    {
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

    void Logon(CSTRING fix_raw_logon)
    {
        std::cout << "logon function has been called:..." << std::endl;
        _fix_raw_logon_string = fix_raw_logon;
        FixSession::start();
    }

    std::string createLogon(const std::string user, const std::string pass)
    {
        std::string MsgSeqNum = std::to_string(MsgNum);
        /// construct fix message logon
        FIX::Message message;

        message.getHeader().setField(8, "FIXT.1.1");   // BeginString
        message.getHeader().setField(35, "A");         // MsgType
        message.getHeader().setField(49, _senderid);   // SenderCompID VP
        message.getHeader().setField(56, _targetComp); // TargetCompID, with enumeration, FIX::FIELD::TargetCompID IDX
        message.getHeader().setField(34, MsgSeqNum);
        message.getHeader().setField(52, currentTime());

        // message.getHeader().setField(8, "FIXT.1.1");  // BeginString
        // message.getHeader().setField(49, "VP"); // SenderCompID
        // message.getHeader().setField(56, "IDX");      // TargetCompID, with enumeration, FIX::FIELD::TargetCompID
        // message.getHeader().setField(35, "A");        // MsgType
        // message.getHeader().setField(34, "1");
        // message.getHeader().setField(52,currentTime());
        message.setField(98, "0");
        message.setField(108, "45");
        message.setField(553, user); // vpfc1001
        message.setField(554, pass); // jakarta123
        message.setField(1137, "8");
        FIX::Message checkMsg(message.toString(), _dd, true);
        logWrite("message from create logon: " + message.toString(), GREEN, outp);
        std::cout << "message from create logon: " + message.toString() << std::endl;
        // std::cout << "checksum: " + checksum << std::endl;

        return message.toString();
    }

    // void SendMessage(CSTRING fix_raw_message)
    // {
    //     std::string fix_raw(fix_raw_message);
    //     idx::msg::IDXMessage *oMsg = new idx::msg::IDXMessage(_protocol_version);
    //     oMsg->SetData(fix_raw);
    //     oMsg->SetSessionName(_connection_name);

    //     std::string s(oMsg->GetRawString());
    //     s += IDX_END;

    //     queue_send.push(s);
    //     delete oMsg;
    // }

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

    void Close()
    {
        ioService_.post(boost::bind(&FixSession::do_close, this, boost::system::error_code()));
    }

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
    bool _active;
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

    boost::thread *_thread_send;
    boost::thread *_thread_receive;
    std::string _incomplete_raw;
    boost::shared_ptr<boost::asio::ip::tcp::socket> socket_;

    bool _connect;
    bool _loggedOn;
    bool _logonInProgress;
};

int main(int argc, char *argv[])
{
    std::string filesconfig = argv[1];
    std::string fileDatadictionary = argv[2];
    boost::asio::io_service ioService;
    std::ifstream is(fileDatadictionary, std::ios::in);
    std::string userinput;
    FileCustom conf;
    // FixSession._HouseKeeping = conf.HouseKeeping();

    std::cout << YELLOW << "reading configuration file at a specified path: " << RESET << "" MAGENTA << filesconfig << RESET << std::endl;
    if (!is.is_open())
    {
        std::cout << RED << "The file " << fileDatadictionary << " cannot be opened" << RESET << std::endl;
        return 1;
    }
    FIX::DataDictionary dd(is);

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

    // std::cout << " status: " << (fixSession.is_connect() ? GREEN "connected" RESET : RED "not connect" RESET) << std::endl;
    // FIX::DataDictionary& _dd = dd;

    if (conf.ReadConFile(filesconfig))
    {

        //  conf.printFile();
        // conf.myVector[2] = "test change";
        // conf.writeToXml();

        // conf.printFile();
        try
        {
            FixSession fixSession(ioService, dd, conf.myVector[0], conf.myVector[1], conf.myVector[4], conf.myVector[6], 10, 10); ////test to jats server
            fixSession.MsgNum = conf.msgSeqNum();

            // fixSession.start();
            fixSession.val_conf(conf.myVector[3], conf.myVector[2]);

            // fixSession.printCreateLoogn();
            while (true)
            {
                /* code */

                if (fixSession.is_connect())
                {
                    std::cout << "do you wwant to logon? y/n:  ";
                    std::cin >> userinput;
                    if (userinput == "y" or userinput == "Y")
                    {
                        fixSession.MsgNum++;
                        conf.saveSeqNum(fixSession.MsgNum);
                        std::cout << "msgNum increment: " << fixSession.MsgNum << std::endl;
                        fixSession.logon(conf.myVector[4], conf.myVector[6]);
                    }
                    else
                    {
                        return 1;
                    }
                }
                else
                {
                    // reconnect?
                    fixSession.logWrite("Reconnect.", YELLOW, outp);

                    fixSession.start();
                }
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
