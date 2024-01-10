
#include "../include/TcpClient.h"
#include <iostream>

namespace idx
{
namespace tcp
{

/** @brief TcpClient
  *
  * @todo: document this function
  */
TcpClient::TcpClient(boost::asio::io_service& io_service, std::string address, std::string port):
    _address(address),
    _port(port),
    _io_service(io_service),
    _connect_timer(io_service),
    _connection_timeout(boost::posix_time::seconds(3))
{
    if (_address.empty())
    {
        throw std::runtime_error("address empty!");
        return;
    }
    if (_port.empty())
    {
        throw std::runtime_error("port empty!");
        return;
    }

}

/** @brief ~TcpClient
  *
  * @todo: document this function
  */
TcpClient::~TcpClient()
{

    if (_thread_service != NULL)
    {
        _thread_service->join();
        delete _thread_service;
    }
    
}

/** @brief IsActive
  *
  * @todo: document this function
  */
bool TcpClient::IsActive()
{
    return _active;
}

/** @brief InternalConnect
 *
 * @todo: document this function
 */
void TcpClient::Connect()
{
   boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(_io_service);
    boost::asio::ip::tcp::resolver::query query(_address, _port);
    boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

    connect_start(iterator);

    _thread_service = new boost::thread(boost::bind(&TcpClient::runIOService, this, boost::ref(_io_service)));
    std::cout << "Enter connect_start... d "  << std::endl;	
        
}


/** @brief Close
  *
  * @todo: document this function
  */
void TcpClient::Close()
{
    _io_service.post(boost::bind(&TcpClient::do_close, this, boost::system::error_code()));
}


/** @brief WriteStr
  *
  * @todo: document this function
  */
void TcpClient::WriteStr(const std::string msg)
{

   _io_service.post(boost::bind(&TcpClient::do_writestr, this, msg));

}

void TcpClient::runIOService(boost::asio::io_service& io_service)
{
    io_service.run();

    io_service.reset();
}

/** @brief do_close
  *
  * @todo: document this function
  */
void TcpClient::do_close(const boost::system::error_code& error)
{
    // something has gone wrong, so close the socket & make this object inactive
    if (error == boost::asio::error::operation_aborted) // if this call is the result of a timer cancel()
        return; // ignore it because the connection cancelled the timer
    if (error)
    {
        handle_error(error.message(), error);
    }	

    socketP->close();
    _active = false;
}


void TcpClient::do_writestr(std::string msg)
  {
    bool write_in_progress = !_writestring_msgs.empty();
    _writestring_msgs.push_back(msg);
    //cout << "Enter write_in_progress ... "  << endl;	
    if (!write_in_progress)
    {
      boost::asio::async_write(*socketP,
          boost::asio::buffer(_writestring_msgs.front(),
            _writestring_msgs.front().size()),
          boost::bind(&TcpClient::writestr_complete, this,
            boost::asio::placeholders::error));
    }
  }

void TcpClient::writestr_complete(const boost::system::error_code& error)
  { 

    if (!error)
    {

      _writestring_msgs.pop_front();
      if (!_writestring_msgs.empty())
      {
        boost::asio::async_write(*socketP,
            boost::asio::buffer(_writestring_msgs.front(),
              _writestring_msgs.front().size()),
            boost::bind(&TcpClient::writestr_complete, this,
              boost::asio::placeholders::error));
      }
    }
    else
    {
      do_close(error);
    }
  }

/** @brief read_complete
  *
  * @todo: document this function
  */
void TcpClient::read_complete(const boost::system::error_code& error, size_t bytes_transferred)
{
    // the asynchronous read operation has now completed or failed and returned an error
    if (!error)
    {
        // read completed, so process the data
        std::string receivemsg(_read_msg, bytes_transferred);
        handle_receive(receivemsg);
        //cout << "Enter read_start... "  << endl;	
        read_start(); // start waiting for another asynchronous read again
    }
    else
        do_close(error);
}

/** @brief read_start
  *
  * @todo: document this function
  */
void TcpClient::read_start(void)
{
    // Start an asynchronous read and call read_complete when it completes or fails
    socketP->async_read_some(boost::asio::buffer(_read_msg, max_read_length),
                            boost::bind(&TcpClient::read_complete,
                                        this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

/** @brief connect_complete
  *
  * @todo: document this function
  */
void TcpClient::connect_complete(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator)
{

    // the connection to the server has now completed or failed and returned an error
    if (!error) // success, so start waiting for read data
    {
      std::cout << "Connected to server\n";
	_active = true;

        _connect_timer.cancel(); // the connection was successful, so cancel the timeout
        handle_connected(error);
        read_start();
    }
    else
    {
      std::cerr << "gagal konek ke server di karenakan : " << error.message() << std::endl;
        do_close(error); 
    }
}

/** @brief connect_start
  *
  * @todo: document this function
  */
void TcpClient::connect_start(tcp::resolver::iterator endpoint_iterator)
{

    std::cout << "tes koneksi" << std::endl;
    // asynchronously connect a socket to the specified remote endpoint and call connect_complete when it completes or fails
    tcp::endpoint endpoint = *endpoint_iterator;
    socketP.reset(new boost::asio::ip::tcp::socket(_io_service));
    socketP->async_connect(endpoint,
                          boost::bind(&TcpClient::connect_complete,
                                      this,
                                      boost::asio::placeholders::error,
                                      ++endpoint_iterator));

    // start a timer that will expire and close the connection if the connection cannot connect within a certain time
    _connect_timer.expires_from_now(_connection_timeout); //boost::posix_time::seconds(connection_timeout));

    //cout << "Enter connect_start... d "  << endl;	

    _connect_timer.async_wait(boost::bind(&TcpClient::do_close, this, boost::asio::placeholders::error));



}

}
}
