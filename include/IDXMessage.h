
// #ifndef IDX_MESSAGE_H
// #define IDX_MESSAGE_H

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
            int64_t _BeginSeqNo;

            std::string sqnum;
            int _CheckSum;
            std::string _MsgType;
            std::string _SessionName;
            std::string _PossDup;
            std::string _Data;
            std::string _RawString;
            std::string seqnumlow;
            bool isSeqnumlow;
            bool isTag58;

        public:
            IDXMessage(const std::string Version);
            ~IDXMessage();

            std::string GetVersion();
            void SetVersion(const std::string Version);
            std::string GetMsgType();
            void SetMsgType(const std::string MsgType);
            std::string GetSeqNum();
            void SetSeqNum(const int64_t SeqNum);

            std::string GetBeginSeqNo();
            void SetBeginSeqNo(const int64_t val);

            std::string GetSessionName();
            void SetSessionName(const std::string SessionName);
            std::string GetPossDup();
            void SetPossDup(const std::string PossDup);
            std::string GetCheckSum();
            void SetCheckSumm(const int CheckSumm);
            std::string GetData();
            void SetData(const std::string FixMsgData);
            std::string GetRawString();
            void SetRawString(const std::string RawString);

            std::string getSeqNumLow();
            bool getIsTag58();

            bool isSeqnumLow();
        };

    }
}

// #endif // IDX_MESSAGE_H
