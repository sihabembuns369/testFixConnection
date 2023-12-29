#include <iostream>
#include <deque>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "inc/WriteLog.hpp"
// #include <fstream>
// #include <ctime>
using boost::asio::ip::tcp;

class FixClient
{
public:
    // FixClient(boost::asio::io_service &ioService, const std::string &host, const std::string &port)
    //     : socket_(ioService), resolver_(ioService), endpoint_(resolver_.resolve({host, port})) {}
    FixClient(boost::asio::io_service &ioService, const std::string& host, const std::string& port,const std::string& username, const std::string& password)
        : socket_(ioService), resolver_(ioService), endpoint_(resolver_.resolve({host, port})),username_(username), password_(password){}


    void connect()
    {
        boost::asio::io_service ioService;
        socketP.reset(new boost::asio::ip::tcp::socket(ioService));
        boost::asio::async_connect(socket_, endpoint_, std::bind(&FixClient::handleConnect, this, std::placeholders::_1));
        //  _connect_timer.async_wait(boost::bind(&FixClient::do_close, this,std::placeholders::_1, std::placeholders::_2));
    }
   
   void logWrite(std::string log){
    WriteLog logWrite;
    logWrite.logMessage(log);

   }

private:
    bool _active;
    boost::thread *_thread_service;
    boost::shared_ptr<tcp::socket> socketP;
    std::deque<std::string> _writestring_msgs;
    std::array<char, 1024> receive_buffer_;
    std::string username_;
    std::string password_;
    

    // boost::asio::deadline_timer _connect_timer;

    void handleConnect(const boost::system::error_code &err)
    {
        if (!err)
        {
            sendFixMessage();
            // sendMessage("8=FIX.1.1|35=A|34=1|49=VP_01|56=IDX|98=0|108=30|141=Y|");
             startRead();
        }
        else {
            std::cerr << "Connection error: " << err.message() << std::endl;
        }
    }

    void sendFixMessage()
    {
        std::cout << "sending a message" << std::endl;
        // bool write_in_progress = !_writestring_msgs.empty();
        // _writestring_msgs.push_back(msg);
        std::string fixMessage ="8=FIX.1.1|35=A|49=" + username_ + "|56=IDX|34=1|98=0|108=30|141=Y|554=" + password_ + "|";
        std::cout << "send fix message to server "<< fixMessage << std::endl;

        boost::asio::async_write(socket_, boost::asio::buffer(fixMessage), std::bind(&FixClient::handleWrite, this, std::placeholders::_1, std::placeholders::_2));
    }

    void handleWrite(const boost::system::error_code &error, std::size_t)
    {
        if (!error){
             std::string fixMessage ="8=FIX.1.1|35=A|49=" + username_ + "|56=IDX|34=1|98=0|108=30|141=Y|554=" + password_ + "|";
            boost::asio::async_write(socket_, boost::asio::buffer(fixMessage), std::bind(&FixClient::handleWrite, this, std::placeholders::_1, std::placeholders::_2));
        }
        else {
            // std::cout << "closed" << std::endl;
            do_close(error);
        }
    }

    void do_close(const boost::system::error_code &error)
    {
        if (error == boost::asio::error::operation_aborted)
            return;
        if (error) {
            // std::cout << "connection error " << std::endl;
            logWrite("connection error");
        }
        _active = false;
    }

void startRead() {
     socket_.async_read_some(boost::asio::buffer(_read_msg, 1024),boost::bind(&FixClient::read_complete, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void read_complete(const boost::system::error_code &error, size_t bytes_transferred)
    {

        if (!error)
        {
            std::istream response_stream(&response_);
            std::string received_message;
            std::getline(response_stream, received_message);

            std::cout << "Received: " << received_message << std::endl;
            startRead();
        }
        else
        {
            do_close(error);
        }
    }

// void read_complete(const boost::system::error_code& error, size_t bytes_transferred)
// {
//     // the asynchronous read operation has now completed or failed and returned an error
//     if (!error)
//     {
//         // read completed, so process the data
//         std::string receivemsg(_read_msg, bytes_transferred);
//         handle_receive(receivemsg);
//         //cout << "Enter read_start... "  << endl;	
//         read_start(); // start waiting for another asynchronous read again
//     }
//     else
//         do_close(error);
// }


private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::resolver::iterator endpoint_;
      boost::asio::streambuf request_;
    boost::asio::streambuf response_;
     char _read_msg[1024];
    
    // boost::asio::deadline_timer _connect_timer;
};

int main()
{

    try
    {
        boost::asio::io_service ioService;
        FixClient fixClient(ioService, "172.18.2.213", "59881", "vpfc1001", "jakarta123");
        fixClient.connect();
        ioService.run();
        // fixClient.logWrite("tes");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
