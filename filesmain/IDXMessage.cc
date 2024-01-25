
#include <stdlib.h>
#include <sstream>
#include "../include/IDXUtil.h"
#include "../include/IDXConstants.h"
#include "../include/IDXMessage.h"

namespace idx
{
    namespace msg
    {

        IDXMessage::IDXMessage(const std::string Version) : _Version(Version),
                                                            _SeqNum(0),
                                                            _CheckSum(0) /*,
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

        std::string IDXMessage::getSeqNumLow()
        {
            return IDXMessage::seqnumlow;
        }
        bool IDXMessage::isSeqnumLow()
        {
            return IDXMessage::isSeqnumlow;
        }

        bool IDXMessage::getIsTag58()
        {
            return IDXMessage::isTag58;
        }

        std::string IDXMessage::GetMsgType()
        {
            return _MsgType;
        }

        void IDXMessage::SetMsgType(const std::string MsgType)
        {
            _MsgType = MsgType;
        }

        std::string IDXMessage::GetSeqNum()
        {
            std::stringstream ss;
            ss << _SeqNum;
            std::string myString = ss.str();
            return myString;
        }

        void IDXMessage::SetSeqNum(const int64_t SeqNum)
        {
            _SeqNum = SeqNum;
        }

        //////////////////////////////////////////
        std::string IDXMessage::GetBeginSeqNo()
        {
            std::stringstream tg7;
            tg7 << _BeginSeqNo;
            std::string myString = tg7.str();

            if (_BeginSeqNo == 0 || _BeginSeqNo == 4294967295)
            {
                return "Resend Request tidak di temukan: Tag 7 = 0 ";
            }
            else
            {
                return myString;
            }
        }

        void IDXMessage::SetBeginSeqNo(const int64_t val)
        {
            _BeginSeqNo = val;
        }
        /////////////////////////////////////////////

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

        std::string IDXMessage::GetCheckSum()
        {
            std::stringstream ss;
            ss << _CheckSum;
            std::string myString = ss.str();
            return myString;
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
            std::string tag35 = FIXMSG_DELIMITER + "35="; // = \x0135=
            std::string tag34 = FIXMSG_DELIMITER + "34=";
            std::string tag10 = FIXMSG_DELIMITER + "10=";
            std::string tag7 = FIXMSG_DELIMITER + "7=";

            size_t posStart, posEnd;

            posStart = FixMsgData.find(tag35);
            if (posStart != std::string::npos)
            {
                posEnd = FixMsgData.find(FIXMSG_DELIMITER, posStart + 4);
                if (posEnd != std::string::npos)
                    _MsgType = FixMsgData.substr(posStart + 4, posEnd - (posStart + 4));
            }

            posStart = FixMsgData.find(tag34);
            if (posStart != std::string::npos)
            {
                posEnd = FixMsgData.find(FIXMSG_DELIMITER, posStart + 4);
                if (posEnd != std::string::npos)
                {
                    std::string seq_num = FixMsgData.substr(posStart + 4, posEnd - (posStart + 4));
                    idx::StringToInt64(seq_num, _SeqNum, 0L);
                }
            }

            posStart = FixMsgData.find(tag10);
            if (posStart != std::string::npos)
            {
                posEnd = FixMsgData.find(FIXMSG_DELIMITER, posStart + 4);
                if (posEnd != std::string::npos)
                {
                    std::string schecksum = FixMsgData.substr(posStart + 4, posEnd - (posStart + 4));
                    // std::cout << "ceksum: " << schecksum << " pos " << posStart << " posend: " << posEnd << std::endl;
                    try
                    {

                        _CheckSum = atoi(schecksum.data());
                        // std::cout << "ceksum: " << _CheckSum << " pos " << posStart << " posend: " << posEnd << std::endl;
                    }
                    catch (std::exception &e)
                    {
                        _CheckSum = 0;
                    }
                }
            }

            posStart = FixMsgData.find(tag7);
            if (posStart != std::string::npos)
            {
                posEnd = FixMsgData.find(FIXMSG_DELIMITER, posStart + 4);
                if (posEnd != std::string::npos)
                { // 74 + 3 = 77, 83 - 79 = 74

                    std::string bginseq = FixMsgData.substr(posStart + 3, (posStart + 3) - posEnd);
                    idx::StringToInt64(bginseq, _BeginSeqNo, 0L);
                    // std::cout << "sqnum: " << bginseq << " posStart " << posStart << " posend: " << posEnd << std::endl;
                }
            }

            posStart = FixMsgData.find("expecting ");
            if (posStart != std::string::npos)
            {
                isSeqnumlow = true;
                posEnd = FixMsgData.find(FIXMSG_DELIMITER, posStart + 4);
                if (posEnd != std::string::npos)
                { // 74 + 3 = 77, 83 - 79 = 74

                    seqnumlow = FixMsgData.substr(posStart + 10, posEnd - (posStart + 25));
                    // idx::StringToInt64(bginseq, _BeginSeqNo, 0L);
                    // std::cout << "sqnum: " << seqnumlow << " posStart " << posStart << " posend: " << posEnd << std::endl;
                }
            }
            else
            {
                isSeqnumlow = false;
            }

            posStart = FixMsgData.find(FIXMSG_DELIMITER + "58=");
            if (posStart != std::string::npos)
            {
                std::cout << "tag 58 ditemukan" << std::endl;
                isTag58 = true;
            }
            else
            {
                isTag58 = false;
                std::cout << "tag 58 tidak ditemukan" << std::endl;
            }

            _Data = FixMsgData;
            // std::cout << "_Data: " << _Data << std::endl;
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
            StringToInt64(temp, _BeginSeqNo, 0L);

            GetStringNextToken(sRaw, _PossDup, IDX_SEPARATOR);
            GetStringNextToken(sRaw, _Data, IDX_SEPARATOR);

            GetStringNextToken(sRaw, temp, IDX_SEPARATOR);
            StringToInt(temp, _CheckSum, 0);
        }

        std::string IDXMessage::GetRawString()
        {
            std::string res = "", sSeqNum, sCheckSum, BeginSeqNo;

            using namespace idx;

            Int64ToString(_SeqNum, sSeqNum, "0");
            Int64ToString(_BeginSeqNo, BeginSeqNo, "0");
            IntToString(_CheckSum, sCheckSum, "0");

            // res += _Version + IDX_SEPARATOR + _MsgType + IDX_SEPARATOR + _SessionName + IDX_SEPARATOR;
            // res += sSeqNum + IDX_SEPARATOR + _PossDup + IDX_SEPARATOR;

            res += _Data;
            // if (!_Data.empty())
            //     res += _Data;
            // res += sCheckSum + IDX_SEPARATOR;

            // ex IDXEQAVP188=FIXT.1.19=9235=A49=VP56=IDX34=1852=20240112-09:30:5898=0108=45553=vpfc1001554=jakarta1231137=810=04545

            // ex IDXEQAVP178=FIXT.1.19=9235=A49=VP56=IDX34=1752=20240112-09:26:5698=0108=45553=vpfc1001554=jakarta1231137=810=04747
            // std::cout << "res on IDXMessage: " << res << std::endl;

            return res;
        }

    }
}
