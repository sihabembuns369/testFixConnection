#include <iostream>
#include <boost/asio.hpp>

int main()
{
    std::string host = "172.18.2.213";
    int port = 59881;
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket client_socket(io_service);
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);


    // Mencoba melakukan koneksi

    boost::system::error_code ec;
    client_socket.connect(endpoint, ec);
    io_service.run();

    if (ec)
    {
        // Kesalahan koneksi
        std::cerr << "Error connecting to the server: " << ec.message() << std::endl;
        return 1; // Keluar dari program dengan status error
    }

    try
    {
        std::cout << "\nstarting...." << std::endl;
        if (client_socket.is_open())
        {
            std::cout << "connected to server " << host << " on port " << port << std::endl;
            std::string logonMessage = "8=FIX.1.1|35=A|34=1|49=VP_01|56=IDX|98=0|108=30|141=Y|";
            boost::asio::write(client_socket, boost::asio::buffer(logonMessage));

            // Membaca respons dari server FIX menggunakan Boost.Asio
            std::vector<char> data(1024);
            size_t bytesRead = boost::asio::read(client_socket, boost::asio::buffer(data));
            // Memproses data FIX yang dibaca
            std::cout << "Received FIX message: " << std::string(data.begin(), data.begin() + bytesRead) << std::endl;

            // Menutup socket
            // client_socket.close();
        }
        else
        {
            std::cerr << "Failed to connect to the server." << std::endl;
        }
    }
    catch (const boost::system::system_error &e)
    {
        if (e.code() == boost::asio::error::eof)
        {
            // Koneksi ditutup karena EOF
            std::cerr << "Connection closed by the peer (EOF)." << std::endl;
        }
        else
        {
            // Tangani exception lainnya
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}
