#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <iostream>
#include <deque>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <unistd.h>

#include <tbb/concurrent_queue.h>

#include "DataDictionary.h"
#include "Message.h"
#include "IDXMessage.h"
#include "IDXConstants.h"


namespace idx
{
namespace tcp
{

typedef boost::asio::io_service IOSERVICE;
typedef const boost::system::error_code ERROR;
typedef const std::string CSTRING;

using namespace std;

using boost::asio::ip::tcp;

class TcpClient
{
public: // ctor & dtor
    TcpClient(boost::asio::io_service& io_service, const std::string address, const std::string port);
    virtual ~TcpClient();

private:
    static const int max_read_length = 1024; // maximum amount of data to read in one operation
    bool _active; // remains true while this object is still operating
    std::string _address;
    std::string _port;    

    boost::asio::io_service& _io_service; // the main IO service that runs this connection
    boost::asio::deadline_timer _connect_timer;
    boost::posix_time::time_duration _connection_timeout; // time to wait for the connection to succeed

    boost::shared_ptr<tcp::socket> socketP;
    boost::thread *_thread_service;

    char _read_msg[max_read_length]; // data read from the socket
    deque<std::string> _writestring_msgs;
    void connect_start(tcp::resolver::iterator endpoint_iterator);
    void connect_complete(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator);

    void read_start(void);
    void read_complete(const boost::system::error_code& error, size_t bytes_transferred);

    void writestr_complete(const boost::system::error_code& error);
    void do_writestr(std::string msg);

    void do_close(const boost::system::error_code& error);

    void runIOService(boost::asio::io_service& io_service);

protected:
    virtual void handle_receive(const std::string receivemsg) =0;
    virtual void handle_error(const std::string errmsg, const boost::system::error_code& err) = 0;
    virtual void handle_connected(const boost::system::error_code& err) =0;
    
public:
    virtual void Connect();
    void WriteStr(const std::string msg);
    void Close();
    bool IsActive();


    std::string GetAddress()
    {
        return _address;
    };

    std::string GetPort()
    {
        return _port;
    };

};

}
}

#endif // TCPCLIENT_H





