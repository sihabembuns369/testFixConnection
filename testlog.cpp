#include <iostream>
#include <string>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

// Struktur data untuk merepresentasikan pesan Logon FIX 5.0
struct LogonMessage {
    std::string beginString;
    std::string msgType;
    std::string senderCompID;
    std::string targetCompID;
};

// Adaptasi struktur data ke dalam Fusion untuk digunakan dengan Boost.Spirit
BOOST_FUSION_ADAPT_STRUCT(
    LogonMessage,
    (std::string, beginString)
    (std::string, msgType)
    (std::string, senderCompID)
    (std::string, targetCompID)
)

namespace qi = boost::spirit::qi;

int main() {
    // Contoh pesan Logon FIX 5.0
    std::string logonMessage = "8=FIXT.1.1|35=A|49=SenderCompID|56=TargetCompID|";

    // Definisi grammar untuk parsing pesan Logon FIX 5.0
    qi::rule<std::string::iterator, std::string(), qi::space_type> string_rule =
        qi::lexeme[+(qi::char_ - '|')]; // Mengonsumsi karakter hingga bertemu '|'

    qi::rule<std::string::iterator, LogonMessage(), qi::space_type> logonMessageParser =
        qi::lit("8=") >> qi::as_string[string_rule] >> '|' >>  // BeginString
        qi::lit("35=") >> qi::as_string[string_rule] >> '|' >> // MsgType
        qi::lit("49=") >> qi::as_string[string_rule] >> '|' >> // SenderCompID
        qi::lit("56=") >> qi::as_string[string_rule];           // TargetCompID

    LogonMessage parsedLogonMessage;

    // Parsing pesan Logon
    std::string::iterator iter = logonMessage.begin();
    std::string::iterator end = logonMessage.end();
    bool parseSuccess = qi::phrase_parse(iter, end, logonMessageParser, qi::space, parsedLogonMessage);

    // Memeriksa apakah parsing berhasil
    if (parseSuccess && iter == end) {
        // Menampilkan hasil parsing
        std::cout << "Parsed Logon Message:" << std::endl;
        std::cout << "BeginString: " << parsedLogonMessage.beginString << std::endl;
        std::cout << "MsgType: " << parsedLogonMessage.msgType << std::endl;
        std::cout << "SenderCompID: " << parsedLogonMessage.senderCompID << std::endl;
        std::cout << "TargetCompID: " << parsedLogonMessage.targetCompID << std::endl;
    } else {
        std::cerr << "Parsing failed!" << std::endl;
    }

    return 0;
}
