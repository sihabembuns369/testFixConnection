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

#include "include/md5.hpp"

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
typedef const std::string CSTRING;

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

    FixSession(boost::asio::io_service &ioService, const std::string &targetIP, const std::string &targetPort, const std::string &username, const std::string &password,FIX::DataDictionary& dd)
        : ioService_(ioService),
          socket_(ioService),
          targetIP_(targetIP),
          targetPort_(targetPort),
          username_(username),
          password_(password),
        _connect_timer(ioService),
    _connection_timeout(boost::posix_time::seconds(3)),
    _dd(dd){}

    void start()
    {
        // Resolve the target IP and port
        std::cout << "function start() " << std::endl;
        boost::asio::ip::tcp::resolver resolver(ioService_);
        boost::asio::ip::tcp::resolver::query query(targetIP_, targetPort_);
        // auto endpointIterator = resolver.resolve(query);
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
        connect_start(iterator);
        _thread_service = new boost::thread(boost::bind(&FixSession::runIOService, this, boost::ref(ioService_)));
         bsendthreadrun = true;
        _thread_send = new boost::thread(boost::bind(&FixSession::handle_send_thread, this));
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
        io_service.run();

        io_service.reset();
    }
    
    void logon(const std::string user, const std::string pass){

        Logon(createLogon(user,pass));
    }
    // FIX::DataDictionary& _dd ;

    // void data_dic(FIX::DataDictionary& dd){

    //    FIX::DataDictionary& _dd = dd;
    // }


private:

   void connect_start(boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
    {
        boost::asio::ip::tcp::tcp::endpoint endpoint = *endpoint_iterator;
        boost::asio::async_connect(socket_, endpoint_iterator,
                                   boost::bind(&FixSession::handleConnect, this,
                                               boost::asio::placeholders::error));

        _connect_timer.expires_from_now(_connection_timeout); 
    }

    bool IsActive(){return _active;}

    void handleConnect(const boost::system::error_code &ec)
    {
        if (!ec)
        {
            std::cout << "Connected to the FIX server." << std::endl;

            logWrite("connected to server", GREEN);
            // sendFixMessage("8=FIX.1.1|35=A|49=" + username_ + "|56=IDX|34=1|98=0|108=30|141=Y|554=" + password_ + "|");

            // Example: Read messages from the FIX server
            //  "8=FIXT.1.1|35=A|49=SenderCompID|56=TargetCompID|34=1|98=0|108=30|141=Y|553=Username|554=Password|10=231|";
            //   std::string times = currentTime();
            // std::string message = "8=FIXT.1.1|9=0|35=A|34=0|49=VP|56=IDX|52"+ times +"|98=0|108=30|141=Y|553"+username_+"|554=" + password_ + "|";
            std::string logon = createLogon("vpfc1001", "jakarta123");
            handle_connected(ec);
            // do_writestr(ec);
            readFixMessage();
        }
        else
        {
            std::cerr << "Error connecting to the FIX server: " << ec.message() << std::endl;
            logWrite("error connecting to the fix server: " + ec.message(), RED);
        }
    }

    void do_writestr(const std::string &message)
    {
        // Post the write operation to the io_service
        ioService_.post(boost::bind(&FixSession::sendFixMessage, this, message));
    }

    void sendFixMessage(std::string &message)
    {

        logWrite("message: " + message, BLUE);
        bool write_in_progress = !_writestring_msgs.empty();
        _writestring_msgs.push_back(message);
        if (!write_in_progress)
        {
            std::cout << "sendFixMessage()" << std::endl;

            boost::asio::async_write(socket_, boost::asio::buffer(_writestring_msgs.front(), _writestring_msgs.front().size()), boost::bind(&FixSession::writestr_complete, this, boost::asio::placeholders::error));
        }
    }

    void writestr_complete(const boost::system::error_code &error)
    {

        if (!error)
        {
            logWrite("send message to server success: ", GREEN);
            // readFixMessage();
            // std::string times = currentTime();
            // std::string message = "8=FIXT.1.1|9=0|35=A|34=0|49=VP|56=IDX|52" + times + "|98=0|108=30|141=Y|553" + username_ + "|554=" + password_ + "|";
            _writestring_msgs.pop_front();
            if (!_writestring_msgs.empty())
            {
                std::cout << "writestr_complete()" << std::endl;
                boost::asio::async_write(socket_, boost::asio::buffer(_writestring_msgs.front(), _writestring_msgs.front().size()), boost::bind(&FixSession::writestr_complete, this, boost::asio::placeholders::error));
            }
            // readFixMessage();
            // std::string logon = createLogon("vpfc1001", "jakarta123");
            //   boost::asio::async_write(socket_, boost::asio::buffer(logon), boost::bind(&FixSession::writestr_complete, this, boost::asio::placeholders::error));
        }
        else
        {
            logWrite(" write error : " + error.message(), RED);
            std::cerr << "Write error: " << error.message() << std::endl;
            do_close(error);
        }
    }

    void readFixMessage()
    {
        // TODO: Implement logic to read FIX message
        // For example: boost::asio::async_read_until(socket_, buffer_, '\x01', ...);
        logWrite("di fungsi readFixMessage()", YELLOW);
        socket_.async_read_some(boost::asio::buffer(_read_msg, max_read_length),
                                boost::bind(&FixSession::read_complete,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
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
            // cout << "Enter read_start... "  << endl;
            // readFixMessage(); // start waiting for another asynchronous read again
        }
        else
        {
            logWrite("   error read complete: " + error.value(), RED);
            do_close(error);
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

    void do_close(const boost::system::error_code &error)
    {
        // something has gone wrong, so close the socket & make this object inactive
        if (error == boost::asio::error::operation_aborted) // if this call is the result of a timer cancel()
            return;                                         // ignore it because the connection cancelled the timer
        if (error)
        {
            std::string s = createLogon("vpfc1001", "jakarta123");
            logWrite("connection closed message: " + s, RED);
            // handle_error(error.message(), error);
        }

        socket_.close();
        // _active = false;
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
        }
    }

    std::string createLogon(const std::string user, const std::string pass)
    {
        /// construct fix message logon
        FIX::Message message;

        message.getHeader().setField(8, "FIXT.1.1"); // BeginString
        message.getHeader().setField(49, "vpfc1001");   // SenderCompID
        message.getHeader().setField(56, "IDX");     // TargetCompID, with enumeration, FIX::FIELD::TargetCompID
        message.getHeader().setField(35, "A");       // MsgType
        // message.setField(98, 0);
        // message.setField(108, 0);
        message.setField(553, user);
        message.setField(554, pass);
        message.setField(1137, "8");

        int i=message.toString().length();
        // std::cout << "message.toString().length(): " << i << std::endl;
        // std::cout << "datadictionary: " << _dd << std::endl;

        FIX::Message checkMsg(message.toString(), _dd, true);

        return message.toString();
    }

    void SendMessage(CSTRING fix_raw_message)
{
    std::string fix_raw(fix_raw_message);
    idx::msg::IDXMessage * oMsg = new idx::msg::IDXMessage(_protocol_version);
    oMsg->SetData(fix_raw);
    oMsg->SetSessionName(_connection_name);

    std::string s(oMsg->GetRawString());
    s += IDX_END;

    queue_send.push(s);
    delete oMsg;

}

void  handle_send_thread()
{
    try{
        while (bsendthreadrun)
        {

	        if (IsActive())
	        {
	        	std::string senddata;
	        	while(queue_send.try_pop(senddata))
			{
			    do_writestr(senddata);

			}
	                
	        }
            // ------------------
            boost::posix_time::milliseconds idletime(_sendsleep);
            boost::this_thread::sleep(idletime);
        }
    }catch (std::exception& e){
       std::cerr << e.what() << '\n';
    }
}

void Logon(CSTRING fix_raw_logon)
{
    _fix_raw_logon_string = fix_raw_logon;	
    FixSession::start();
}
    // 8=FIXT.1.1^9=116^35=A^49=BuySide^56=SellSide^34=1^52=20190605-11:49:18.979^1128=8^98=0^108=30^141=Y^553=Username^554=Password^1137=8^10=089^

    // std::string createLogon(const std::string &username, const std::string &password, const std::string &targetCompID, const std::string &senderCompID)
    // {
    //     std::ostringstream fixMessage;
    //     fixMessage << "8=FIX.1.1"           // BeginString
    //                << "\x01"                // SOH (Start of Header)
    //                << "49=" << targetCompID // TargetCompID
    //                << "\x01"                // SOH
    //                << "56=" << senderCompID // SenderCompID
    //                << "\x01"                // SOH
    //                << "35=A"                // MsgType (Logon)
    //                << "\x01"                // SOH
    //                << "553=" << username    // Username
    //                << "\x01"                // SOH
    //                << "554=" << password    // Password
    //                << "1137=8"
    //                << "\x01"; // SOH

    //     // ... (tambahkan field-field FIX logon yang diperlukan sesuai kebutuhan)

    //     return fixMessage.str();
    // }

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
    std::string _fix_raw_logon_string ;
    std::string _connection_name = "VP_01";
    std::string _protocol_version = "IDXEQ";
    tbb::concurrent_queue<std::string> queue_receive;
     tbb::concurrent_queue<std::string> queue_send;
    std::deque<std::string> _writestring_msgs;

    boost::atomic<bool> bsendthreadrun;
    boost::atomic<float> _sendsleep;
    boost::thread *_thread_send;
    FIX::DataDictionary& _dd;
};

int main()
{

    std::FILE *file = std::fopen("conf/FIX50SP1-IDX.xml", "rb");
    std::string MD5FIXML = "d36cb8f0fccd193867eb70d75d57d7a3";
    if (!file) {
       std::cout << "ggal membuka file" << std::endl;
        return 1; // Menghentikan program dengan status error
    }

   int character;
    while ((character = std::fgetc(file)) != EOF) {
        std::putchar(character);
    }

    if ( MD5FIXML.compare(md5file(file).c_str()) != 0)
    {
     std::cout << "md5 matched" << std::endl;
    }else{
        std::cout << "md5 has not matched" << std::endl;
    }

    boost::asio::io_service ioService;
    std::ifstream is("conf/FIX50SP1-IDX.xml", std::ios::in);
    FIX::DataDictionary dd(is);
    // FIX::DataDictionary& _dd = dd;


    // Replace "TARGET_IP" and "TARGET_PORT" with your FIX server details
    // "172.18.2.213", "59881"
   
    FixSession fixSession(ioService, "172.18.2.213", "59881" , "vpfc1001", "jakarta123", dd);
    try
    {


        // fixSession.logon("vpfc1001", "jakarta123");
        // fixSession.data_dic(dd);

        // fixSession.start();

        // ioService.run();
    }
    catch (std::exception &e)
    {
        std::fclose(file);
        std::string error = e.what();
        fixSession.logWrite("exception: " + error, RED);
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
