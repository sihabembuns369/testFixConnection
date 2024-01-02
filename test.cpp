#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "inc/WriteLog.hpp"
#include "inc/netstat.hpp"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"


class FixSession {
public:
    FixSession(boost::asio::io_service& ioService, const std::string& targetIP, const std::string& targetPort, const std::string& username, const std::string& password)
        : ioService_(ioService),
          socket_(ioService),
          targetIP_(targetIP),
          targetPort_(targetPort),
          username_(username),
        password_(password) {}

    void start() {
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

    void logWrite(std::string log, std::string color){
    WriteLog logWrite;
     netstat net;
    std::string netprint = net.exec("netstat -an | grep 59881");
    if (netprint.size()<=0){
     logWrite.logMessage(log + " => " + "no connection occurs", color);
    }else{
    logWrite.logMessage(log + " => " + netprint, color);
    }
    //  logWrite.logMessage(log);

   }
 void runIOService(boost::asio::io_service& io_service)
{
    io_service.run();

    io_service.reset();
}


private:

    void handleConnect(const boost::system::error_code& ec) {
        if (!ec) {
            std::cout << "Connected to the FIX server." << std::endl;
            logWrite("connected to server", GREEN);

            // TODO: Implement FIX session logic here

            // Example: Send a FIX Logon message
            sendFixMessage("8=FIX.1.1|35=A|49=" + username_ + "|56=IDX|34=1|98=0|108=30|141=Y|554=" + password_ + "|");

            // Example: Read messages from the FIX server
            readFixMessage();
        } else {
            std::cerr << "Error connecting to the FIX server: " << ec.message() << std::endl;
            logWrite("error connecting to the fix server: " + ec.message(), RED);
        }
    }

    void sendFixMessage(const std::string& message) {
        // TODO: Implement logic to send FIX message
        // For example: boost::asio::async_write(socket_, boost::asio::buffer(message), ...);
    }

    void readFixMessage() {
        // TODO: Implement logic to read FIX message
        // For example: boost::asio::async_read_until(socket_, buffer_, '\x01', ...);
    }

    boost::thread *_thread_service;
    boost::asio::io_service& ioService_;
    boost::asio::ip::tcp::socket socket_;
    std::string targetIP_;
    std::string targetPort_;
    std::string username_;
    std::string password_;
};

int main() {
      boost::asio::io_service ioService;

        // Replace "TARGET_IP" and "TARGET_PORT" with your FIX server details
        FixSession fixSession(ioService,  "172.18.2.213", "59881", "vpfc1001", "jakarta123");
    try {
      
        fixSession.start();

        ioService.run();
    } catch (std::exception& e) {
        std::string error = e.what();
        fixSession.logWrite("exception: " + error, RED);
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
