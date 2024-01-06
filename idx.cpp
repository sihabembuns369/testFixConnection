#include "include/TcpClient.h";


class idx: public idx::tcp::TcpClient{
    public:
    idx(boost::asio::io_service& io_service, FIX::DataDictionary& dd, 
    const std::string address, const std::string port,
    const std::string protocol_version,
    const std::string begin_header,
    const std::string sender_comp_id,
    const std::string target_comp_id,
    const std::string connection_name,
    const float sendsleep,
    const float recvsleep):idx::tcp::TcpClient(io_service, address, port),
	    _dd(dd),
        _connected(false),
        _loggedOn(false),
	    _loggedOnJATS(false),
        _logonInProgress(false),
        _logoutInProgress(false),
        _initiateLogon(false),
        _initiateLogout(false),
        _protocol_version(protocol_version),
        _begin_header(begin_header),
        _sender_comp_id(sender_comp_id),
        _target_comp_id(target_comp_id),
        _connection_name(connection_name),
        _server_address(address),
        _server_port(port),
        _sendsleep(sendsleep),
        _recvsleep(recvsleep){}

    private:
     FIX::DataDictionary& _dd;

    bool _connected;
    bool _loggedOn;
    bool _loggedOnJATS;
    bool _logonInProgress;
    bool _logoutInProgress;
    bool _initiateLogon;
    bool _initiateLogout;

    std::string _protocol_version;
    std::string _begin_header;
    std::string _sender_comp_id;
    std::string _target_comp_id;
    std::string _connection_name;
    std::string _server_address;
    std::string _server_port;
    std::string _fix_raw_logon_string;
    boost::atomic<float> _sendsleep;
    boost::atomic<float> _recvsleep;
    std::string _incomplete_raw;

    boost::atomic<bool> bsendthreadrun;
    boost::atomic<bool> brecvthreadrun;
    tbb::concurrent_queue<std::string> queue_receive;
    tbb::concurrent_queue<std::string> queue_send;
    tbb::concurrent_queue<idx::msg::IDXMessage*> queue_sendX;
    boost::thread *_thread_receive;
    boost::thread *_thread_send;

};

