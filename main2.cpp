#include <iostream>
#include <deque>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "inc/WriteLog.hpp"
#include "inc/netstat.hpp"
#include <boost/bind.hpp>

// #include <fstream>
// #include <ctime>
using boost::asio::ip::tcp;

// ANSI escape codes for text colors
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"


class FixClient
{
public: 
    // FixClient(boost::asio::io_service &ioService, const std::string &host, const std::string &port)
    //     : socket_(ioService), resolver_(ioService), endpoint_(resolver_.resolve({host, port})) {}

    FixClient(boost::asio::io_service &ioService, const std::string& host, const std::string& port, const std::string& username, const std::string& password)
        : socket_(ioService),
        resolver_(ioService),
        endpoint_(resolver_.resolve({host, port})),
        username_(username),
        password_(password){}

    void start(){}

    void connect()
    {
        std::cout << "connect to server" << std::endl;
        boost::asio::io_service ioService;
        socketP.reset(new boost::asio::ip::tcp::socket(ioService));
        boost::asio::async_connect(socket_, endpoint_, std::bind(&FixClient::handleConnect, this, std::placeholders::_1));
        //  _connect_timer.async_wait(boost::bind(&FixClient::do_close, this,std::placeholders::_1, std::placeholders::_2));
    }
   
   void logWrite(std::string log, std::string color){
    WriteLog logWrite;
     netstat net;
    std::string netprint = net.exec("netstat -an | grep 59881");
    if (netprint.size()<=0){
     logWrite.logMessage(log + " => " + "tidak ada koneksi terjadi", color);
    }else{
    logWrite.logMessage(log + " => " + netprint, color);
    }
    //  logWrite.logMessage(log);

   }



    

    // boost::asio::deadline_timer _connect_timer;

    void handleConnect(const boost::system::error_code &err)
    {
        if (!err)
        {
            std::cout << "ini handle koneksi" <<std::endl;
             logWrite("succesful conection in handleConnect()" + err.message(), GREEN);
            sendFixMessage();
            // sendMessage("8=FIX.1.1|35=A|34=1|49=VP_01|56=IDX|98=0|108=30|141=Y|");
            //  startRead();
        }
        else {
            std::cerr << "Connection error: " << err.message() << std::endl;
            logWrite("connection error in handleConnect()" + err.message(), RED);
        }
    }

    void sendFixMessage()
    {
        // std::cout << "sending a message" << std::endl;
        // bool write_in_progress = !_writestring_msgs.empty();
        // _writestring_msgs.push_back(msg);
        std::string fixMessage ="8=FIX.1.1|35=A|49=" + username_ + "|56=IDX|34=1|98=0|108=30|141=Y|554=" + password_ + "|";
        std::cout << "send fix message to server "<< fixMessage << std::endl;
        logWrite("send to server", BLUE);

        // boost::asio::async_write(socket_, boost::asio::buffer(fixMessage, sizeof(fixMessage)), std::bind(&FixClient::handleWrite, this, std::placeholders::_1, std::placeholders::_2));

        async_write(socket_, boost::asio::buffer(fixMessage, sizeof(fixMessage)),boost::bind(&FixClient::handleWrite,this,boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void handleWrite(const boost::system::error_code &error, std::size_t bytes_transferred)
    {
        if (!error){
            //  logWrite("handlewrite() sukses", BLUE);
             std::string fixMessage ="8=FIX.1.1|35=A|49=" + username_ + "|56=IDX|34=1|98=0|108=30|141=Y|554=" + password_ + "|";
          async_write(socket_, boost::asio::buffer(fixMessage, sizeof(fixMessage)),boost::bind(&FixClient::handleWrite,this,boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        }
        else {
            // std::cout << "closed" << std::endl;
             logWrite( "handlewrite() error write", RED);
            do_close(error);
            // connect();
            // sendFixMessage();
        }
    }

    void do_close(const boost::system::error_code &error)
    {
        if (error == boost::asio::error::operation_aborted)
         logWrite("connection operation aborted on do_close()", RED);
            return;
        if (error) {
            // std::cout << "connection error " << std::endl;
            logWrite("connection error on do_close()", RED);
        }
        // _active = false;
    }

      void startRead() {
        boost::asio::async_read(socket_, boost::asio::buffer(receive_buffer_),
            [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                if (!ec) {
                    std::cout << "Received data from server: " << std::string(receive_buffer_.data(), bytes_transferred) << std::endl;
                    startRead();
                } else {
                    logWrite( "Read error: " + ec.message(), RED);
                    std::cerr << "Read error: " << ec.message() << std::endl;
                }
            });
    }
    // void startRead() {
    //  socket_.async_read_some(boost::asio::buffer(_read_msg, 1024),boost::bind(&FixClient::read_complete, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    // }

    // void read_complete(const boost::system::error_code &error, size_t bytes_transferred)
    // {

    //     if (!error)
    //     {
    //         std::istream response_stream(&response_);
    //         std::string received_message;
    //         std::getline(response_stream, received_message);

    //         std::cout << "Received: " << received_message << std::endl;
            
    //         startRead();
    //     }
    //     else
    //     {
    //         do_close(error);
    //     }
    // }




private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::resolver::iterator endpoint_;
    boost::asio::streambuf response_;


     char _read_msg[1024];
         bool _active;
    boost::thread *_thread_service;
    boost::shared_ptr<tcp::socket> socketP;
    std::deque<std::string> _writestring_msgs;
    std::array<char, 1024> receive_buffer_;
    std::string username_;
    std::string password_;

    // boost::asio::deadline_timer _connect_timer;

     
    
    // boost::asio::deadline_timer _connect_timer;
};

int main()
{       boost::asio::io_service ioService;
        FixClient fixClient(ioService, "172.18.2.213", "59881", "vpfc1001", "jakarta123" );
    try
    {
        

        fixClient.connect();
        ioService.run();
        // fixClient.logWrite("tes", GREEN);
    }
    catch (std::exception &e)
    {
        std::string error = e.what();
        fixClient.logWrite("Exception: " + error, RED);
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
