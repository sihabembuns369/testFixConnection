
#include <stdlib.h>
#include "../include/IDXUtil.h"
#include "../include/IDXConstants.h"
#include "../include/IDXMessage.h"

namespace idx
{
namespace msg
{

    IDXMessage::IDXMessage(const std::string Version):
        _Version(Version),
        _SeqNum(0),
        _CheckSum(0)/*, 
        body_length_(0)*/
    {

    }

    IDXMessage::~IDXMessage()
    {

    }

    std::string IDXMessage::GetVersion()
    {
        return _Version;
    }

    void IDXMessage::SetVersion(const std::string Version)
    {
        _Version = Version;
    }

    std::string IDXMessage::GetMsgType()
    {
        return _MsgType;
    }

    void IDXMessage::SetMsgType(const std::string MsgType)
    {
        _MsgType = MsgType;
    }

    int64_t IDXMessage::GetSeqNum()
    {
        return _SeqNum;
    }

    void IDXMessage::SetSeqNum(const int64_t SeqNum)
    {
        _SeqNum = SeqNum;
    }

    std::string IDXMessage::GetSessionName()
    {
        return _SessionName;
    }

    void IDXMessage::SetSessionName(const std::string SessionName)
    {
        _SessionName = SessionName;
    }

    std::string IDXMessage::GetPossDup()
    {
        return _PossDup;
    }

    void IDXMessage::SetPossDup(const std::string PossDup)
    {
        _PossDup = PossDup;
    }

    int IDXMessage::GetCheckSum()
    {
        return _CheckSum;
    }

    void IDXMessage::SetCheckSumm(const int CheckSumm)
    {
        _CheckSum = CheckSumm;
    }

    std::string IDXMessage::GetData()
    {
        return _Data;
    }

    void IDXMessage::SetData(const std::string FixMsgData)
    {
        std::string tag35 = FIXMSG_DELIMITER + "35=";
        std::string tag34 = FIXMSG_DELIMITER + "34=";
        std::string tag10 = FIXMSG_DELIMITER + "10=";

        size_t posStart, posEnd;


        posStart = FixMsgData.find(tag35);
        if (posStart!=std::string::npos)
        {
            posEnd = FixMsgData.find(FIXMSG_DELIMITER, posStart+4);
            if (posEnd!=std::string::npos)
                _MsgType = FixMsgData.substr(posStart+4, posEnd-(posStart+4));
        }

        posStart = FixMsgData.find(tag34);
        if (posStart!=std::string::npos)
        {
            posEnd = FixMsgData.find(FIXMSG_DELIMITER, posStart+4);
            if (posEnd!=std::string::npos)
            {
                std::string seq_num = FixMsgData.substr(posStart+4, posEnd-(posStart+4));
                idx::StringToInt64(seq_num, _SeqNum, 0L);
            }
        }

        posStart = FixMsgData.find(tag10);
        if (posStart!=std::string::npos)
        {
            posEnd = FixMsgData.find(FIXMSG_DELIMITER, posStart+4);
            if (posEnd!=std::string::npos)
            {
                std::string schecksum = FixMsgData.substr(posStart+4, posEnd-(posStart+4));
                try{
                    _CheckSum = atoi(schecksum.data());
                }catch(std::exception &e){
                    _CheckSum = 0;
                }
            }
        }

        _Data = FixMsgData;

    }

    void IDXMessage::SetRawString(const std::string RawString)
    {
        std::string sRaw = RawString;
        std::string temp;
        using namespace idx;
        GetStringNextToken(sRaw, _Version, IDX_SEPARATOR);
        GetStringNextToken(sRaw, _MsgType, IDX_SEPARATOR);
        GetStringNextToken(sRaw, _SessionName, IDX_SEPARATOR);

        GetStringNextToken(sRaw, temp, IDX_SEPARATOR);
        StringToInt64(temp, _SeqNum, 0L);

        GetStringNextToken(sRaw, _PossDup, IDX_SEPARATOR);
        GetStringNextToken(sRaw, _Data, IDX_SEPARATOR);

        GetStringNextToken(sRaw, temp, IDX_SEPARATOR);
        StringToInt(temp, _CheckSum, 0);
    }

    std::string IDXMessage::GetRawString()
    {
        std::string res = "", sSeqNum, sCheckSum;

        using namespace idx;

        Int64ToString(_SeqNum, sSeqNum, "0");
        IntToString(_CheckSum, sCheckSum,"0");

        res += _Version + IDX_SEPARATOR + _MsgType + IDX_SEPARATOR + _SessionName + IDX_SEPARATOR;
        res += sSeqNum + IDX_SEPARATOR + _PossDup + IDX_SEPARATOR;
        if (!_Data.empty())
            res += _Data + IDX_SEPARATOR;
        res += sCheckSum + IDX_SEPARATOR;
        return res;
    }


}
}
