#include <iostream>
#include <boost/asio.hpp>

class TcpClient {
public:
    TcpClient(const std::string& host, int port) : host_(host), port_(port), io_service_(), client_socket_(io_service_) {
        endpoint_ = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host_), port_);
    }

    void connectToServer() {
        boost::system::error_code ec;
        client_socket_.connect(endpoint_, ec);
        if (!ec) {
            std::cout << "Connected to the server!" << std::endl;
            std::string message ="8=FIXT.1.1|35=A|49=VP_01|56=IDX|34=1|";
            sendMessage(message);
  
        } else {
            std::cerr << "Failed to connect: " << ec.message() << std::endl;
        }
    }

    void runService() {
        io_service_.run();
    }

    void sendMessage(const std::string& message) {
        boost::system::error_code ec;
        boost::asio::write(client_socket_, boost::asio::buffer(message), ec);

        if (!ec) {
            std::cout << "Message sent: " << message << std::endl;
        } else {
            std::cerr << "Failed to send message: " << ec.message() << std::endl;
        }
    }

   
private:
    std::string host_;
    int port_;
    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::socket client_socket_;
    boost::asio::ip::tcp::endpoint endpoint_;
};

int main() {
    std::string host = "172.18.2.213";
    int port = 59881;

    TcpClient tcpClient(host, port);
    tcpClient.connectToServer();


    // std::string message ="8=FIXT.1.1|35=A|49=VP_01|56=IDX|34=1|";
    // tcpClient.sendMessage(message);

    tcpClient.runService();

    return 0;
}
