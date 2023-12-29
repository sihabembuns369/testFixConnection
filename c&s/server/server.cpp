#include <iostream>
#include <string>
#include <boost/asio.hpp>

class TcpServer {
public:
    TcpServer(boost::asio::io_service& io_service, short port)
        : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
          socket_(io_service) {
        acceptConnection();
    }

private:
    void acceptConnection() {
        acceptor_.accept(socket_);

        // Read username and password from the client
        boost::asio::streambuf request;
        boost::asio::read_until(socket_, request, '\n');

        std::istream is(&request);
        std::string username, password;
        is >> username >> password;

        // Authenticate the client
        bool isAuthenticated = authenticate(username, password);

        // Send response to the client
        std::string response = isAuthenticated ? "Authentication successful\n" : "Authentication failed\n";
        boost::asio::write(socket_, boost::asio::buffer(response + "\n"));

        // Close the socket (optional)
        socket_.close();

        // Continue accepting connections
        acceptConnection();
    }

    bool authenticate(const std::string& username, const std::string& password) {
        // Simple authentication logic (replace with your own logic)
        return (username == "user" && password == "pass");
    }

private:
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
};

int main() {
    try {
        boost::asio::io_service io_service;
        TcpServer server(io_service, 12345);
        io_service.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
