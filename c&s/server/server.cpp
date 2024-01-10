#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <map>

using namespace boost::asio;
using ip::tcp;

class FixServer {
public:
    FixServer(io_service& io_service, short port)
        : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
          socket_(io_service) {
        startAccept();
    }

    // Fungsi untuk mencetak pesan FIX
void printFixMessage(const std::map<int, std::string>& fixMessage) {
    std::cout << "Pesan FIX: ";
    for (const auto& entry : fixMessage) {
        std::cout << "[" << entry.first << "=" << entry.second << "] ";
    }
    std::cout << std::endl;
}

private:
    void startAccept() {
        acceptor_.async_accept(socket_,
            [this](boost::system::error_code ec) {
                if (!ec) {
                    std::cout << "Client connected." << std::endl;
                    handleRead();
                }
                startAccept();
            });
    }
    

    void handleRead() {
    socket_.async_read_some(boost::asio::buffer(_read_msg, max_read_length),
                                boost::bind(&FixServer::read_complete,
                                            this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

    void read_complete(const boost::system::error_code &error, size_t bytes_transferred){
        if (!error)
        {
            // read completed, so process the data
            std::string receivemsg(_read_msg, bytes_transferred);
            std::cout << "read message: " + receivemsg << std::endl;
            // handleRead(); // start waiting for another asynchronous read again
            sendFixMessage("8=FIXT.1.19=6335=A49=IDX56=VP34=152=20240109-00:55:1498=0108=451137=810=083");
        }
        else
        {
            std::cout << "" << std::endl;
        }
    }

    void sendFixMessage(const std::string &message)
    {

        // logWrite("message sent to server: "+message, BLUE);

        boost::asio::async_write(socket_, boost::asio::buffer(message), boost::bind(&FixServer::writestr_complete, this, boost::asio::placeholders::error));
    }

    void writestr_complete(const boost::system::error_code &error)
    {

        if (!error)
        {
            // logWrite("send message to server success: " , GREEN);
            handleRead();            
        //     std::string times = currentTime();
        //     std::string message = "8=FIXT.1.1|9=0|35=A|34=0|49=VP|56=IDX|52"+ times +"|98=0|108=30|141=Y|553"+username_+"|554=" + password_ + "|";
        //   boost::asio::async_write(socket_, boost::asio::buffer(message), boost::bind(&FixSession::writestr_complete, this, boost::asio::placeholders::error));
        }else{
            //  logWrite(" write error : " + error.message(), RED);
            std::cerr << "Write error: " << error.message() << std::endl;
        }
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    
    std::string data_;
    static const int max_read_length = 1024;
    char _read_msg[max_read_length];
};

int main() {
    try {
        boost::asio::io_service io_service;
        FixServer server(io_service, 12345);
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
