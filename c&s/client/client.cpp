#include <iostream>
#include <string>
#include <boost/asio.hpp>

class TcpClient {
public:
    TcpClient(boost::asio::io_service& io_service, const std::string& server_address, short port, const std::string& username, const std::string& password)
        : socket_(io_service) {
        connectToServer(server_address, port);

        // Send username and password to the server
        boost::asio::write(socket_, boost::asio::buffer(username + " " + password + "\n"));

        // Read the response from the server
        boost::asio::streambuf response;
        boost::asio::read_until(socket_, response, '\n');

        // Process the response
        std::istream is(&response);
        std::string serverResponse;
        std::getline(is, serverResponse);

        std::cout << "Received response from server: " << serverResponse << std::endl;
    }

private:
    void connectToServer(const std::string& server_address, short port) {
        boost::asio::ip::tcp::resolver resolver(socket_.get_io_service());
        boost::asio::ip::tcp::resolver::query query(server_address, std::to_string(port));
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        boost::asio::connect(socket_, endpoint_iterator);
    }

private:
    boost::asio::ip::tcp::socket socket_;
};

int main() {
    try {
        boost::asio::io_service io_service;
        TcpClient client(io_service,"172.18.2.213", 59881, "vpfc1001", "jakarta123");
        io_service.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
