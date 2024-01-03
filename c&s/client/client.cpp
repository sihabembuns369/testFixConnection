#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "../../inc/WriteLog.hpp"
#include "../../inc/netstat.hpp"

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"

class FixSession
{
public:
    FixSession(boost::asio::io_service &ioService, const std::string &targetIP, const std::string &targetPort, const std::string &username, const std::string &password)
        : ioService_(ioService),
          socket_(ioService),
          targetIP_(targetIP),
          targetPort_(targetPort),
          username_(username),
          password_(password) {}

    void start()
    {
        // Resolve the target IP and port
        boost::asio::ip::tcp::resolver resolver(ioService_);
        boost::asio::ip::tcp::resolver::query query(targetIP_, targetPort_);
        // auto endpointIterator = resolver.resolve(query);
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
        connect_start(iterator);
        _thread_service = new boost::thread(boost::bind(&FixSession::runIOService, this, boost::ref(ioService_)));
    }

    void connect_start(boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
    {
        boost::asio::ip::tcp::tcp::endpoint endpoint = *endpoint_iterator;
        boost::asio::async_connect(socket_, endpoint_iterator,
                                   boost::bind(&FixSession::handleConnect, this,
                                               boost::asio::placeholders::error));
    }

    std::string currentTime(){
         WriteLog time;
         return time.timecurrent();
    }

    void logWrite(std::string log, std::string color)
    {
        WriteLog logWrite;
        netstat net;
        // std::cout << logWrite.timecurrent() << std::endl;
        std::string netprint = net.exec("netstat -an | grep 9876");
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

private:
    void handleConnect(const boost::system::error_code &ec)
    {
        if (!ec)
        {
            std::cout << "Connected to the FIX server: " + targetIP_ + " on port: " + targetPort_ << std::endl;
            
            logWrite("connected to server", GREEN);
            // sendFixMessage("8=FIX.1.1|35=A|49=" + username_ + "|56=IDX|34=1|98=0|108=30|141=Y|554=" + password_ + "|");

            // Example: Read messages from the FIX server
            readFixMessage();
            //  "8=FIXT.1.1|35=A|49=SenderCompID|56=TargetCompID|34=1|98=0|108=30|141=Y|553=Username|554=Password|10=231|";
              std::string times = currentTime();
            std::string message = "8=FIXT.1.1|9=0|35=A|34=0|49=VP|56=IDX|52"+ times +"|98=0|108=30|141=Y|553"+username_+"|554=" + password_ + "|";
            do_writestr(message);
        }
        else
        {
            std::cerr << "Error connecting to the FIX server: " << ec.message() << std::endl;
            logWrite("error connecting to the fix server: " + ec.message(), RED);
        }
    }

     void do_writestr(const std::string& message) {
        // Post the write operation to the io_service
        ioService_.post(boost::bind(&FixSession::sendFixMessage, this, message));
    }

    void sendFixMessage(const std::string &message)
    {

        logWrite("message sent to server: "+message, BLUE);

        boost::asio::async_write(socket_, boost::asio::buffer(message), boost::bind(&FixSession::writestr_complete, this, boost::asio::placeholders::error));
    }

    void writestr_complete(const boost::system::error_code &error)
    {

        if (!error)
        {
            std::cout << "reading message from server" << std::endl;
            // logWrite("send message to server success: " , GREEN);
            readFixMessage();            
            std::string times = currentTime();
            std::string message = "8=FIXT.1.1|9=0|35=A|34=0|49=VP|56=IDX|52"+ times +"|98=0|108=30|141=Y|553"+username_+"|554=" + password_ + "|";
          boost::asio::async_write(socket_, boost::asio::buffer(message), boost::bind(&FixSession::writestr_complete, this, boost::asio::placeholders::error));
        }else{
             logWrite(" write error : " + error.message(), RED);
            std::cerr << "Write error: " << error.message() << std::endl;
        }
    }


    void readFixMessage()
    {
        // TODO: Implement logic to read FIX message
        // For example: boost::asio::async_read_until(socket_, buffer_, '\x01', ...);
        socket_.async_read_some(boost::asio::buffer(_read_msg, max_read_length),
                                boost::bind(&FixSession::read_complete,
                                            this,
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
            std::cout << "reading message from server" + receivemsg << std::endl;

            // handle_receive(receivemsg);
            // cout << "Enter read_start... "  << endl;
            readFixMessage(); // start waiting for another asynchronous read again
        }
        else
        {
            logWrite("    error read complete: " + error.value(), RED);
        }
    }

    boost::thread *_thread_service;
    boost::asio::io_service &ioService_;
    boost::asio::ip::tcp::socket socket_;
    std::string targetIP_;
    std::string targetPort_;
    std::string username_;
    std::string password_;
    static const int max_read_length = 1024;
    char _read_msg[max_read_length];
};

int main()
{
    boost::asio::io_service ioService;

    // Replace "TARGET_IP" and "TARGET_PORT" with your FIX server details
    FixSession fixSession(ioService,"127.0.0.1", "9876", "vpfc1001", "jakarta123");
    try
    {

        fixSession.start();

        ioService.run();
    }
    catch (std::exception &e)
    {
        std::string error = e.what();
        fixSession.logWrite("exception: " + error, RED);
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
