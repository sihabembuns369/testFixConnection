#include "IDX.h"

namespace idx
{
namespace tcp
{
using namespace idx;
class TcpBase: public idx::Base
{
};

class TcpClient;
class TcpClientSession;

typedef std::function<void(idx::tcp::TcpClient, std::string)> CALLBACK_TCPRECEIVE;
typedef std::function<void(idx::tcp::TcpClient)> CALLBACK_TCPCONNECTION;
typedef std::function<void(std::string)> CALLBACK_TCPERROR;

}
}
