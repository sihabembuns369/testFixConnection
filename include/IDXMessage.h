
#ifndef IDX_MESSAGE_H
#define IDX_MESSAGE_H

#include <iostream>

namespace idx
{
namespace msg
{

class IDXMessage
{



protected:
    std::string _Version;
    int64_t _SeqNum;
    int _CheckSum;
    std::string _MsgType;
    std::string _SessionName;
    std::string _PossDup;
    std::string _Data;
    std::string _RawString;

public:
    IDXMessage(const std::string Version);
    ~IDXMessage();

    std::string GetVersion();
    void SetVersion(const std::string Version);
    std::string GetMsgType();
    void SetMsgType(const std::string MsgType);
    int64_t GetSeqNum();
    void SetSeqNum(const int64_t SeqNum);
    std::string GetSessionName();
    void SetSessionName(const std::string SessionName);
    std::string GetPossDup();
    void SetPossDup(const std::string PossDup);
    int GetCheckSum();
    void SetCheckSumm(const int CheckSumm);
    std::string GetData();
    void SetData(const std::string FixMsgData);
    std::string GetRawString();
    void SetRawString(const std::string RawString);

};

}
}


#endif // IDX_MESSAGE_H
