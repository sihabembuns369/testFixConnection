#include <iostream>
#include <string>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <map>

// Struktur data untuk merepresentasikan pesan FIX
struct FixMessage {
    std::string beginString;
    char messageType;
    int msgSeqNum;
    std::string senderCompID;
    std::string targetCompID;
    // Tambahkan field-field lain yang dibutuhkan
};

// Struktur data untuk merepresentasikan pesan eksekusi
struct ExecutionReport {
    std::string orderID;
    char execType;
    double price;
    int quantity;
    // Tambahkan field-field lain yang dibutuhkan
};

// Adaptasi struktur data ke dalam Fusion untuk digunakan dengan Boost.Spirit
BOOST_FUSION_ADAPT_STRUCT(
    FixMessage,
    (std::string, beginString)
    (char, messageType)
    (int, msgSeqNum)
    (std::string, senderCompID)
    (std::string, targetCompID)
)

BOOST_FUSION_ADAPT_STRUCT(
    ExecutionReport,
    (std::string, orderID)
    (char, execType)
    (double, price)
    (int, quantity)
)

namespace qi = boost::spirit::qi;

// Data Dictionary (contoh sederhana)
const std::map<std::string, int> tagToType = {
    {"8", 1},  // BeginString
    {"35", 2}, // MsgType
    {"34", 3}, // MsgSeqNum
    {"49", 4}, // SenderCompID
    {"56", 5},  // TargetCompID
    {"11", 6},  // ClOrdID
    {"39", 7},  // OrdStatus
    {"44", 8},  // Price
    {"38", 9}   // OrderQty
};

// Definisi grammar untuk parsing pesan FIX dengan Data Dictionary
template <typename Iterator>
struct FixMessageParser : qi::grammar<Iterator, FixMessage(), qi::space_type> {
    FixMessageParser() : FixMessageParser::base_type(start) {
        using namespace qi;

        // Definisi aturan parsing dengan Data Dictionary
        start %= "8=" >> string("FIX.4.2") >> '|' >> // BeginString
                 "35=" >> char_ >> '|' >>          // MsgType
                 "34=" >> int_ >> '|' >>          // MsgSeqNum
                 "49=" >> string >> '|' >>        // SenderCompID
                 "56=" >> string;                  // TargetCompID
    }

    qi::rule<Iterator, FixMessage(), qi::space_type> start;
};

// Definisi grammar untuk parsing pesan Execution Report
template <typename Iterator>
struct ExecutionReportParser : qi::grammar<Iterator, ExecutionReport(), qi::space_type> {
    ExecutionReportParser() : ExecutionReportParser::base_type(start) {
        using namespace qi;

        // Definisi aturan parsing pesan Execution Report
        start %= "8=" >> string("FIX.4.2") >> '|' >>       // BeginString
                 "35=" >> char_("8") >> '|' >>            // MsgType (Execution Report)
                 "11=" >> string >> '|' >>               // ClOrdID
                 "39=" >> char_ >> '|' >>                 // OrdStatus
                 "44=" >> double_ >> '|' >>               // Price
                 "38=" >> int_;                            // OrderQty
    }

    qi::rule<Iterator, ExecutionReport(), qi::space_type> start;
};