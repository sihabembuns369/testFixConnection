
#ifndef IDX_CONSTANTS_H
#define IDX_CONSTANTS_H

#include <iostream>


static const std::string FIXMSG_DELIMITER = "\x01";
static const std::string IDX_SEPARATOR = "\x03";
static const std::string IDX_TYPE_HEARTBEAT = "0";
static const std::string IDX_TYPE_LOGON = "A";
static const std::string IDX_TYPE_TESTREQUEST = "1";
static const std::string IDX_TYPE_REJECT = "3";
static const std::string IDX_TYPE_SEQUENCERESET = "4";
static const std::string IDX_TYPE_LOGOUT = "5";
static const std::string IDX_TYPE_APPMESSAGE = "M";
static const std::string IDX_TYPE_ADMMESSAGE = "N";
static const std::string IDX_PROTOCOLVER_EQUITY = "FIXT.1.1";
static const std::string IDX_PROTOCOLVER_DERIVATIVE = "IDXDR";
static const std::string IDX_END = "\r\n";



#endif // IDX_CONSTANTS_H
