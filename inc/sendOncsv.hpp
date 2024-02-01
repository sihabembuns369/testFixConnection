#ifndef SEND_ON_CSV_H
#define SEND_ON_CSV_H

#include <cstdio>
#include <ctime>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <memory>
#include <vector>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <chrono>
#include <cstdint>
#include <fstream>

#include <cctype>
#include <stdio.h>
#include <sys/types.h>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/wait.h>

#include <iomanip>
#include <ctime>

// shuffle algorithm
#include <algorithm> // std::shuffle
#include <array>     // std::array
#include <random>    // std::default_random_engine
#include <chrono>    // std::chrono::system_clock
#include <stdexcept>

#include <boost/regex.hpp>
// #include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/locale.hpp>
#include <boost/date_time.hpp>
// #include <boost/thread.hpp>
#include <boost/locale/format.hpp>
#include <boost/typeof/typeof.hpp>
// ini-boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

#include <cassert>

// #include "../include/DataDictionary.h"
#include "../include/Message.h"
#include "../include/Group.h"
// #include "../include/md5.hpp"

// #include "colors.h"
namespace bpt = boost::posix_time;
namespace bg = boost::gregorian;
using namespace std;
std::string BEGINSTRING = "FIXT.1.1";
std::ifstream is("./DataDictionary/FIX50SP1-IDX.xml", std::ios::in);
FIX::DataDictionary _dd(is);

typedef std::vector<std::string> Row;
const boost::regex linesregx("\\r\\n|\\n\\r|\\n|\\r");
const boost::regex fieldsregx(",(?=(?:[^\"]*\"[^\"]*\")*(?![^\"]*\"))");
std::vector<std::string> PartyRoles{"3", "5", "7", "17", "36", "37"};
std::string today = "Monday";

std::vector<std::string> AccountTypes{"1", "3", "100", "101"};
std::vector<std::string> OrderRestrictions{"q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};
std::vector<std::string> Sides{"1", "2", "5", "M", "P"};
std::vector<std::string> OrdTypes{"1", "2"};

namespace msg
{
    std::string utc_date_()
    {
        static char const *const fmt = "%Y%m%d";
        std::ostringstream ss;
        // assumes std::cout's locale has been set appropriately for the entire app
        ss.imbue(std::locale(std::cout.getloc(), new bg::date_facet(fmt)));
        ss << bg::day_clock::universal_day();

        return ss.str();
    }

    std::string generatorCLORID()
    {
        std::ostringstream oss;
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y%m%d%X", &tstruct);

        return buf;
    }
    std::string GetFixCurrTime()
    {
        time_t currtime;
        struct tm *timeinfo;
        char buffer[26];

        // 20170725 08:59:30
        // 01234567890123456
        time(&currtime);
        timeinfo = localtime(&currtime);

        strftime(buffer, 18, "%Y%m%d-%H:%M:%S\0", timeinfo);
        std::string sCurrTime(buffer, 18);
        return sCurrTime;
    }

    std::vector<Row> parse(const char *data, unsigned int length)
    {
        std::vector<Row> result;

        // iterator splits data to lines
        boost::cregex_token_iterator li(data, data + length, linesregx, -1);
        boost::cregex_token_iterator end;

        while (li != end)
        {
            std::string line = li->str();
            ++li;

            // Split line to tokens
            boost::sregex_token_iterator ti(line.begin(), line.end(), fieldsregx, -1);
            boost::sregex_token_iterator end2;

            std::vector<std::string> row;
            while (ti != end2)
            {
                std::string token = ti->str();
                ++ti;
                row.push_back(token);
            }
            if (line.back() == ',')
            {
                // last character was a separator
                row.push_back("");
            }
            result.push_back(row);
        }
        return result;
    }

    void gen_random(char *s, const int len)
    {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        for (int i = 0; i < len; ++i)
        {
            s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        s[len] = 0;
    }

    bool in_array(const std::string &value, const std::vector<string> &array)
    {
        int max = array.size();

        if (max == 0)
            return false;

        for (int i = 0; i < max; i++)
            if (array[i] == value)
                return true;
        return false;
    }
    std::string gmt_Time()
    {
        char outstr[80];
        time_t t;
        struct tm *tmp;
        const char *fmt = "%Y%m%d-%X";

        t = time(NULL);
        tmp = gmtime(&t);
        strftime(outstr, sizeof(outstr), fmt, tmp);

        return outstr;
    }

    std::string date_add(const std::string today, int dayAdd)
    {
        std::string dday(today.substr(0, 8));
        std::string dtime(today.substr(8));

        bg::date d(bg::from_undelimited_string(dday));
        bg::date_duration dd(dayAdd);
        d += dd;
        std::string result(to_iso_string(d) + dtime);
        // std::cout << result;

        return result;
    }
    int dayofweek(int d, int m, int y)
    {
        static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
        y -= m < 3;
        return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
    }

    std::string getSettleDate(std::string today)
    {
        std::vector<std::string> holidays{"20180101", "20180216", "20180330", "20180501", "20180510",
                                          "20180529", "20180601", "20180611", "20180612", "20180613",
                                          "20180614", "20180615", "20180618", "20180619", "20180817",
                                          "20180822", "20180911", "20181120", "20181224", "20181225", "20181231"};

        std::string nextDate = date_add(today, 2);
        //	std::string nextDate;
        //	if (!in_array(date_add(nextDate,2), holidays)) date_add(nextDate,1); else date_add(nextDate,2);

        std::string ydate = nextDate.substr(0, 4);
        std::string mdate = nextDate.substr(4, 2);
        std::string ddate = nextDate.substr(6, 2);
        // cout << "today : " << today << " nexDate: " << nextDate << " ddate: " << ddate << " mdate: " << mdate << " ydate: " << ydate << endl;

        int day = dayofweek(atoi(ddate.c_str()), atoi(mdate.c_str()), atoi(ydate.c_str()));
        // cout << "day: " <<day<<endl;
        // const std::string wDay=get_week_day(nextDate);
        int choliday = 1;

        while (choliday == 1)
        {
            if ((day == 6) && (!in_array(date_add(nextDate, 2), holidays)))
                return date_add(nextDate, 2);
            else if ((day == 0) && (!in_array(date_add(nextDate, 2), holidays)))
                return date_add(nextDate, 2);
            // else if ( (day==5)  && (in_array(nextDate, holidays) )) return date_add(nextDate,5);
            else
            {
                nextDate = date_add(nextDate, 1);
                day = dayofweek(atoi(nextDate.substr(6, 2).c_str()), atoi(nextDate.substr(4, 2).c_str()), atoi(nextDate.substr(0, 4).c_str()));
                // cout << "nextDate: " << nextDate <<" d: " << day << " ddate: "<< atoi(nextDate.substr(6,2).c_str()) << " mdate: " << atoi(nextDate.substr(4,2).c_str()) << " ydate: " << atoi(nextDate.substr(0,4).c_str()) << endl;
                if (((day >= 1) && (day <= 5)) && (!in_array(nextDate, holidays)))
                    choliday = 0;
            }
        }

        return nextDate;
    }

    std::string sendCSVRequestForPositions(const std::string dataFileCSVRequestForPositions, const std::string senderID, int msgnum)
    {
        FIX::Message message;
        std::string MsgSeqNum = std::to_string(msgnum);

        std::string tag_710;
        char pReqId[20];
        gen_random(pReqId, 20);

        std::string ReqId(pReqId);

        std::string tag_724, tag_263, tag_453, tag_448, tag_447, tag_452, tag_715, tag_60;

        std::ifstream infile;
        infile.open(dataFileCSVRequestForPositions);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileCSVRequestForPositions << std::endl;
        }
        char buffer[1024];
        infile.read(buffer, sizeof(buffer));
        buffer[infile.tellg()] = '\0';

        // parse file, returns vector of rows
        std::vector<Row> result = parse(buffer, strlen(buffer));

        // print out result
        for (size_t r = 0; r < result.size(); r++)
        {
            Row &row = result[r];
            if (r == 0)
            {
                for (size_t d = 0; d < row.size() - 1; d++)
                {
                    // std::cout << row[d] << "\t";
                }
                // std::cout << "\n";
            }
            if (r == 1)
            {
                for (size_t c = 0; c < row.size() - 1; c++)
                {
                    std::cout << row[c] << "\t";
                }

                if (row[0].empty())
                {
                    std::cout << " tag 710 can't be empty " << std::endl;
                    tag_710 = ReqId;
                }
                else
                    tag_710 = row[0];
                if (row[1].empty())
                {
                    std::cout << " tag 724 can't be empty " << std::endl;
                }
                else
                    tag_724 = row[1];
                tag_263 = row[2];

                if (row[3].empty())
                    std::cout << " tag 453 can't be empty " << std::endl;
                else
                    tag_453 = row[3];
                if (row[4].empty())
                    std::cout << " tag 448 can't be empty " << std::endl;
                else
                    tag_448 = row[4];
                if (row[5].empty())
                    std::cout << " tag 447 can't be empty " << std::endl;
                else
                    tag_447 = row[5];

                /* if (row[6].empty()) std::cout <<" tag 452 can't be empty " << std::endl;
                else tag_452=row[6];
                 */
                if (row[6].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[6], PartyRoles))
                {
                    tag_452 = row[6];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[7].empty())
                {
                    std::cout << " tag 715 can't be empty " << std::endl;
                    tag_715 = utc_date_();
                }
                else
                    tag_715 = row[7];

                if (row[8].empty())
                {
                    std::cout << " tag 60 can't be empty " << std::endl;
                    tag_60 = GetFixCurrTime();
                }
                else
                    tag_60 = row[8];
            }
        }
        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderID
        message.getHeader().setField(35, "AN");       // MsgType
        message.getHeader().setField(49, "VP");
        message.getHeader().setField(56, "IDX");
        message.getHeader().setField(34, MsgSeqNum);
        message.getHeader().setField(43, "N");
        message.getHeader().setField(52, gmt_Time());

        message.setField(710, tag_710);
        message.setField(724, tag_724);
        message.setField(263, tag_263);
        message.setField(715, utc_date_());
        message.setField(60, gmt_Time());

        FIX::Group NoPartyIDs(453, 448, FIX::message_order(448, 447, 452, 0));
        NoPartyIDs.setField(448, tag_448);
        NoPartyIDs.setField(447, tag_447);
        NoPartyIDs.setField(452, tag_452);
        message.addGroup(NoPartyIDs);
        FIX::Message checkMsg(message.toString(), _dd, false);
        return message.toString();
    }

    std::string sendCSVOrderSingle(const std::string senderID, int msgnum, const std::string tag__11, const std::string tag_581, const std::string tag_38, const std::string tag_40, const std::string tag_44, const std::string tag_55)
    {
        FIX::Message message;
        std::string MsgSeqNum = std::to_string(msgnum);

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderID
        message.getHeader().setField(35, "D");        // MsgType
        message.getHeader().setField(49, "VP");
        message.getHeader().setField(56, "IDX");
        message.getHeader().setField(34, MsgSeqNum);
        message.getHeader().setField(43, "N");
        message.getHeader().setField(52, gmt_Time());

        message.setField(11, generatorCLORID());
        // message.setField(22, "8");
        message.setField(581, tag_581);
        // message.setField(207, "V");
        // message.setField(529, tag_529);
        message.setField(38, tag_38);
        message.setField(40, tag_40);
        message.setField(44, "5500"); // order price
        message.setField(54, "1");
        message.setField(60, gmt_Time());
        // message.setField(58, tag_58);
        message.setField(59, "0");
        message.setField(55, "BBRI");
        message.setField(762, "RG");
        FIX::Group NoPartyIDs(453, 448, FIX::message_order(448, 447, 452, 0));
        NoPartyIDs.setField(448, "1000");
        NoPartyIDs.setField(447, "C");
        NoPartyIDs.setField(452, "5");
        message.addGroup(NoPartyIDs);
        FIX::Message checkMsg(message.toString(), _dd, false);
        // cout << "sendCSVOrderSingle message: " << message.toString() << std::endl;
        return message.toString();
    }

    std::string SendCSVMarketDataSnapshot(const std::string senderID, int msgnum)
    {
        FIX::Message message;
        std::string MsgSeqNum = std::to_string(msgnum);
        char clId[20];
        gen_random(clId, 20);
        std::string clOrdId(clId);

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderID
        message.getHeader().setField(35, "X");        // MsgType
        message.getHeader().setField(49, "VP");
        message.getHeader().setField(56, "IDX");
        message.getHeader().setField(34, MsgSeqNum);
        message.getHeader().setField(43, "N");
        message.getHeader().setField(52, gmt_Time());

        message.setField(262, clOrdId);
        message.setField(279, "0");
        message.setField(269, "1");
        message.setField(278, "0");
        // message.setField(266, "Y");
        // message.setField(286, "6");
        // message.setField(546, "2");
        // message.setField(547, "1");
        // message.setField(559, "0");
        // message.setField(146, "1");
        // message.setField(207, "test");

        // message.setField(55, "ANTM");
        // message.setField(916, utc_date_());

        // FIX::Group NoPartyIDs(267, 269, FIX::message_order(269, 0));
        // NoPartyIDs.setField(269, "0");
        // message.addGroup(NoPartyIDs);

        FIX::Group NoRelatedSym(146, 55, FIX::message_order(55, 65, 48, 22, 460, 461, 167, 762, 200, 541, 1079, 201, 966, 1049, 965, 224, 225, 239, 226, 227, 228, 255, 543, 470, 471, 472, 240, 202, 947, 967, 968, 206, 231, 969, 996, 997, 223, 207, 970, 971, 106, 348, 349, 107, 350, 351, 691, 667, 875, 876, 873, 874, 711, 555, 15, 537, 63, 64, 271, 0));

        message.setField(55, "ANTM");
        // NoRelatedSym.setField(65, "ANTM");
        // NoRelatedSym.setField(48, "ANTM");
        // NoRelatedSym.setField(22, "ANTM");
        message.addGroup(NoRelatedSym);

        FIX::Message checkMsg(message.toString(), _dd, false);
        return message.toString();
        ;
    }

    std::string sendCSVMassQuote(const std::string dataFileCSVMassQuote, const std::string senderID, int msgnum)
    {
        /// construct fix message order
        FIX::Message message;
        std::string MsgSeqNum = std::to_string(msgnum);
        std::string tag_117, tag_453, tag_a_448, tag_a_447, tag_a_452, tag_b_448, tag_b_447, tag_b_452, tag_581, tag_296, tag_302, tag_304, tag_295, tag_299, tag_55, tag_762, tag_132, tag_133, tag_134, tag_135;

        std::ifstream infile;
        infile.open(dataFileCSVMassQuote);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileCSVMassQuote << endl;
        }
        char buffer[1024];
        infile.read(buffer, sizeof(buffer));
        buffer[infile.tellg()] = '\0';

        // parse file, returns vector of rows
        std::vector<Row> result = parse(buffer, strlen(buffer));

        // print out result
        for (size_t r = 0; r < result.size(); r++)
        {
            Row &row = result[r];
            if (r == 1)
            {
                /* for(size_t c=0; c < row.size() - 1; c++) {
                    //std::cout << row[c] << "\t";
                } */

                if (row[0].empty())
                    std::cout << " tag 117 can't be empty " << std::endl;
                else
                    tag_117 = row[0];

                if (row[1].empty())
                    std::cout << " tag 453 can't be empty " << std::endl;
                else
                    tag_453 = row[1];

                if (row[2].empty())
                    std::cout << " tag 448 can't be empty " << std::endl;
                else
                    tag_a_448 = row[2];

                if (row[3].empty())
                    std::cout << " tag 447 can't be empty " << std::endl;
                else
                    tag_a_447 = row[3];

                if (row[4].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[4], PartyRoles))
                {
                    tag_a_452 = row[4];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[5].empty())
                    std::cout << " tag 448 can't be empty " << std::endl;
                else
                    tag_b_448 = row[5];

                if (row[6].empty())
                    std::cout << " tag 447 can't be empty " << std::endl;
                else
                    tag_b_447 = row[6];

                if (row[7].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[7], PartyRoles))
                {
                    tag_b_452 = row[7];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[8].empty())
                    std::cout << " tag 581 can't be empty " << std::endl;
                else
                    tag_581 = row[8];

                // tag_581  =row[8];

                if (row[9].empty())
                    std::cout << " tag 296 can't be empty " << std::endl;
                else
                    tag_296 = row[9];

                if (row[10].empty())
                    std::cout << " tag 302 can't be empty " << std::endl;
                else
                    tag_302 = row[10];

                if (row[11].empty())
                    std::cout << " tag 304 can't be empty " << std::endl;
                else
                    tag_304 = row[11];

                if (row[12].empty())
                    std::cout << " tag 295 can't be empty " << std::endl;
                else
                    tag_295 = row[12];

                if (row[13].empty())
                    std::cout << " tag 299 can't be empty " << std::endl;
                else
                    tag_299 = row[13];

                if (row[14].empty())
                    std::cout << " tag 55 can't be empty " << std::endl;
                else
                    tag_55 = row[14];

                tag_762 = row[15];
                tag_132 = row[16];
                tag_133 = row[17];
                tag_134 = row[18];
                tag_135 = row[19];
            }
        }
        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderID
        message.getHeader().setField(35, "i");        // MsgType
        message.getHeader().setField(49, "VP");
        message.getHeader().setField(56, "IDX");
        message.getHeader().setField(34, MsgSeqNum);
        message.getHeader().setField(43, "N");
        message.getHeader().setField(52, gmt_Time());

        message.setField(117, tag_117);
        message.setField(581, tag_581);

        FIX::Group NoQuoteSets(296, 302, FIX::message_order(302, 304, 0));
        FIX::Group NoQuoteEntries(295, 299, FIX::message_order(299, 55, 132, 133, 134, 135, 762, 0));

        NoQuoteSets.setField(302, tag_302);
        NoQuoteSets.setField(304, tag_304);

        NoQuoteEntries.setField(55, tag_55);
        NoQuoteEntries.setField(132, tag_132);
        NoQuoteEntries.setField(133, tag_133);
        NoQuoteEntries.setField(134, tag_134);
        NoQuoteEntries.setField(135, tag_135);
        NoQuoteEntries.setField(299, tag_299);
        NoQuoteEntries.setField(762, tag_762);
        NoQuoteSets.addGroup(NoQuoteEntries);

        message.addGroup(NoQuoteSets);

        FIX::Group NoPartyIDs(453, 448, FIX::message_order(448, 447, 452, 0));
        NoPartyIDs.setField(448, tag_a_448);
        NoPartyIDs.setField(447, tag_a_447);
        NoPartyIDs.setField(452, tag_a_452);
        message.addGroup(NoPartyIDs);

        NoPartyIDs.setField(448, tag_b_448);
        NoPartyIDs.setField(447, tag_b_447);
        NoPartyIDs.setField(452, tag_b_452);
        message.addGroup(NoPartyIDs);

        FIX::Message checkMsg(message.toString(), _dd, false);
        return message.toString();
    }

    std::string sendCSVCancelNegoDealInternalCrossingInisiator(const std::string dataFileCSVCancelNegoDealInternalCrossingInisiator, const std::string senderID, int msgnum)
    {
        // cancel_negointernalcros_inisiator.csv
        FIX::Message message;
        std::string MsgSeqNum = std::to_string(msgnum);

        std::string tag_571, tag_856, tag_572, tag_64, tag_919, tag_55, tag_762, tag_31, tag_32, tag_552, tag_a_54, tag_a_581, tag_a_453, tag_a1_448, tag_a1_447, tag_a1_452, tag_a2_448, tag_a2_447, tag_a2_452, tag_b_54, tag_b_581, tag_b_453, tag_b1_448, tag_b1_447, tag_b1_452, tag_b2_448, tag_b2_447, tag_b2_452;

        char trdRptId[20];
        gen_random(trdRptId, 20);
        std::string tradeReportID(trdRptId);

        std::ifstream infile;
        infile.open(dataFileCSVCancelNegoDealInternalCrossingInisiator);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileCSVCancelNegoDealInternalCrossingInisiator << endl;
        }
        char buffer[1024];
        infile.read(buffer, sizeof(buffer));
        buffer[infile.tellg()] = '\0';

        // parse file, returns vector of rows
        std::vector<Row> result = parse(buffer, strlen(buffer));

        // print out result
        for (size_t r = 0; r < result.size(); r++)
        {
            Row &row = result[r];
            if (r == 1)
            {
                /* for(size_t c=0; c < row.size() - 1; c++) {
                    std::cout << row[c] << "\t";
                } */

                if (row[0].empty())
                {
                    std::cout << " tag 571 can't be empty " << std::endl;
                    tag_571 = tradeReportID;
                }
                else
                    tag_571 = row[0];

                tag_856 = row[1]; // 2
                tag_572 = row[2]; // 0

                if (row[3].empty())
                    tag_64 = "getSettleDate(today)";
                else
                    tag_64 = row[3];

                tag_919 = row[4];

                if (row[5].empty())
                    std::cout << " tag 55 can't be empty " << std::endl;
                else
                    tag_55 = row[5];

                tag_762 = row[6];
                if (tag_762.compare("NG") != 0)
                    tag_762 = "NG";

                if (row[7].empty())
                    std::cout << " tag 31 can't be empty " << std::endl;
                else
                    tag_31 = row[7];

                if (row[8].empty())
                    std::cout << " tag 32 can't be empty " << std::endl;
                else
                    tag_32 = row[8];

                tag_552 = row[9];

                if (row[10].empty())
                    std::cout << " tag a 54 can't be empty " << std::endl;
                else
                    tag_a_54 = row[10];

                tag_a_581 = row[11];

                if (row[12].empty())
                    std::cout << " tag 453[1] can't be empty " << std::endl;
                else
                    tag_a_453 = row[12];

                if (row[13].empty())
                    std::cout << " tag a1 448 can't be empty " << std::endl;
                else
                    tag_a1_448 = row[13];
                if (row[14].empty())
                    std::cout << " tag a1 447 can't be empty " << std::endl;
                else
                    tag_a1_447 = row[14];

                if (row[15].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[15], PartyRoles))
                {
                    tag_a1_452 = row[15];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[16].empty())
                    std::cout << " tag a2 448 can't be empty " << std::endl;
                else
                    tag_a2_448 = row[16];
                if (row[17].empty())
                    std::cout << " tag a2 447 can't be empty " << std::endl;
                else
                    tag_a2_447 = row[17];

                if (row[18].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[18], PartyRoles))
                {
                    tag_a2_452 = row[15];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[19].empty())
                    std::cout << " tag b 54 can't be empty " << std::endl;
                else
                    tag_b_54 = row[19];

                tag_b_581 = row[20];

                if (row[21].empty())
                    std::cout << " tag 453[2] can't be empty " << std::endl;
                else
                    tag_b_453 = row[21];

                if (row[22].empty())
                    std::cout << " tag b1 448 can't be empty " << std::endl;
                else
                    tag_b1_448 = row[22];

                if (row[23].empty())
                    std::cout << " tag b1 447 can't be empty " << std::endl;
                else
                    tag_b1_447 = row[23];

                if (row[24].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[24], PartyRoles))
                {
                    tag_b1_452 = row[24];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[25].empty())
                    std::cout << " tag b2 448 can't be empty " << std::endl;
                else
                    tag_b2_448 = row[25];

                if (row[26].empty())
                    std::cout << " tag b2 447 can't be empty " << std::endl;
                else
                    tag_b2_447 = row[26];

                if (row[27].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[27], PartyRoles))
                {
                    tag_b2_452 = row[27];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }
            }
        }

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderID
        message.getHeader().setField(35, "AE");       // MsgType
        message.getHeader().setField(49, "VP");
        message.getHeader().setField(56, "IDX");
        message.getHeader().setField(34, MsgSeqNum);
        // message.getHeader().setField(43, "N");
        message.getHeader().setField(52, gmt_Time());

        message.setField(571, tag_571); // random
        // if (!tag_856.empty())
        message.setField(856, tag_856); // 2
        message.setField(919, tag_919);
        message.setField(572, utc_date_()); // 0

        message.setField(55, tag_55);
        message.setField(762, tag_762);
        message.setField(31, tag_31);
        message.setField(32, tag_32);
        // message.setField(60,GetFixCurrTime());

        FIX::Group enteringSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgEnteringNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));
        enteringSides.setField(54, tag_a_54);
        enteringSides.setField(581, tag_a_581);

        sgEnteringNoPartyIds.setField(448, tag_a1_448);
        sgEnteringNoPartyIds.setField(447, tag_a1_447);
        sgEnteringNoPartyIds.setField(452, tag_a1_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);

        sgEnteringNoPartyIds.setField(448, tag_a2_448);
        sgEnteringNoPartyIds.setField(447, tag_a2_447);
        sgEnteringNoPartyIds.setField(452, tag_a2_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);
        message.addGroup(enteringSides);

        FIX::Group contraSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgContraNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));

        contraSides.setField(54, tag_b_54);
        contraSides.setField(581, tag_b_581);
        sgContraNoPartyIds.setField(448, tag_b1_448);
        sgContraNoPartyIds.setField(447, tag_b1_447);
        sgContraNoPartyIds.setField(452, tag_b1_452);
        contraSides.addGroup(sgContraNoPartyIds);

        sgContraNoPartyIds.setField(448, tag_b2_448);
        sgContraNoPartyIds.setField(447, tag_b2_447);
        sgContraNoPartyIds.setField(452, tag_b2_452);
        contraSides.addGroup(sgContraNoPartyIds);

        message.addGroup(contraSides);

        FIX::Message checkMsg(message.toString(), _dd, false);

        return message.toString();
    }

    std::string sendCSVOrderMassActionRequest(const std::string dataFileCSVOrderMassActionRequest, const std::string senderID, int msgnum)
    {
        FIX::Message message;
        std::string MsgSeqNum = std::to_string(msgnum);

        std::string tag_11, tag_526, tag_1373, tag_1374, tag_55, tag_762, tag_60;
        std::ifstream infile;

        /*
        Specifies the scope of the action.
        1 = All orders for a security (firm level)
        7 = All orders (for a firm)
        8 = Cancel particular board (for a firm)
        */
        std::vector<std::string> massActionScopes{"1", "7", "8"};

        // infile.open("request_position.csv");
        infile.open(dataFileCSVOrderMassActionRequest);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileCSVOrderMassActionRequest << endl;
        }
        char buffer[1024];
        infile.read(buffer, sizeof(buffer));
        buffer[infile.tellg()] = '\0';

        // parse file, returns vector of rows
        std::vector<Row> result = parse(buffer, strlen(buffer));

        // print out result
        for (size_t r = 0; r < result.size(); r++)
        {
            Row &row = result[r];
            if (r == 1)
            {
                // std::cout << r.size() << std::endl;
                /* for(size_t c=0; c < row.size() - 1; c++) {
                    //std::cout << row[c] << "\t";
                } */

                if (row[0].empty())
                    std::cout << " tag 11 can't be empty " << std::endl;
                else
                    tag_11 = row[0];

                tag_526 = row[1];

                if (row[2].empty())
                {
                    std::cout << " tag 1373 can't be empty " << std::endl;
                }
                else
                    tag_1373 = row[2];

                if (row[3].empty())
                {
                    std::cout << " tag 1374 can't be empty" << std::endl;
                }
                else if (in_array(row[3], massActionScopes))
                {
                    tag_1374 = row[3];
                }
                else
                {
                    std::cout << " fill in tag 1374 with either [1,7,8] " << std::endl;
                }

                tag_55 = row[4];
                tag_762 = row[5];
                if (row[6].empty())
                {
                    std::cout << " tag 60 can't be empty " << std::endl;
                    tag_60 = gmt_Time();
                }
                else
                    tag_60 = row[6];
            }
        }

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderID
        message.getHeader().setField(35, "CA");       // MsgType
        message.getHeader().setField(49, "VP");
        message.getHeader().setField(56, "IDX");
        message.getHeader().setField(34, MsgSeqNum);
        message.getHeader().setField(43, "N");
        message.getHeader().setField(52, gmt_Time());

        message.setField(11, generatorCLORID());
        message.setField(526, generatorCLORID());
        // Specifies the type of action requested. 3 = Cancel Orders
        message.setField(1373, tag_1373);

        message.setField(1374, tag_1374);

        if (tag_1374.compare("1") == 0)
            message.setField(55, tag_55);

        if (tag_1374.compare("8") == 0)
            message.setField(762, tag_762);

        // message.setField(58, "Test sendCSVOrderMassActionRequest");
        message.setField(60, tag_60);

        FIX::Message checkMsg(message.toString(), _dd, false);
        cout << "sendCSVOrderMassActionRequest message: " << message.toString() << std::endl;

        return message.toString();
    }

    std::string sendCSVOrderNegoDealTwoSide(const std::string dataFileCSVOrderNegoDealTwoSide, const std::string senderID, int msgnum)
    {
        // construct fix message order
        FIX::Message message;
        std::string MsgSeqNum = std::to_string(msgnum);

        std::string tag_571, tag_487, tag_856, tag_64, tag_919, tag_55, tag_762, tag_31, tag_32, tag_552, tag_a_54, tag_a_581, tag_a_453, tag_a1_448, tag_a1_447, tag_a1_452, tag_a2_448, tag_a2_447, tag_a2_452, tag_b_54, tag_b_581, tag_b_453, tag_b1_448, tag_b1_447, tag_b1_452, tag_b2_448, tag_b2_447, tag_b2_452;

        char trdRptId[20];
        gen_random(trdRptId, 20);
        std::string tradeReportID(trdRptId);

        std::ifstream infile;
        infile.open(dataFileCSVOrderNegoDealTwoSide);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileCSVOrderNegoDealTwoSide << endl;
        }

        char buffer[2048];
        infile.read(buffer, sizeof(buffer));
        buffer[infile.tellg()] = '\0';

        // parse file, returns vector of rows
        std::vector<Row> result = parse(buffer, strlen(buffer));

        // print out result
        for (size_t r = 0; r < result.size(); r++)
        {
            Row &row = result[r];
            if (r == 1)
            {
                /* for(size_t c=0; c < row.size() - 1; c++) {
                    //std::cout << row[c] << "\t";
                } */

                if (row[0].empty())
                {
                    std::cout << " tag 571 can't be empty " << std::endl;
                    tag_571 = tradeReportID;
                }
                else
                    tag_571 = row[0];

                tag_487 = row[1]; // 2
                tag_856 = row[2]; // 0

                if (row[3].empty())
                    tag_64 = getSettleDate(today);
                else
                    tag_64 = row[3];

                tag_919 = row[4];

                if (row[5].empty())
                    std::cout << " tag 55 can't be empty " << std::endl;
                else
                    tag_55 = row[5];

                tag_762 = row[6];
                if (tag_762.compare("NG") != 0)
                    tag_762 = "NG";

                if (row[7].empty())
                    std::cout << " tag 31 can't be empty " << std::endl;
                else
                    tag_31 = row[7];

                if (row[8].empty())
                    std::cout << " tag 32 can't be empty " << std::endl;
                else
                    tag_32 = row[8];

                tag_552 = row[9];

                if (row[10].empty())
                    std::cout << " tag a 54 can't be empty " << std::endl;
                else
                    tag_a_54 = row[10];

                tag_a_581 = row[11];

                if (row[12].empty())
                    std::cout << " tag 453[1] can't be empty " << std::endl;
                else
                    tag_a_453 = row[12];

                if (row[13].empty())
                    std::cout << " tag a1 448 can't be empty " << std::endl;
                else
                    tag_a1_448 = row[13];
                if (row[14].empty())
                    std::cout << " tag a1 447 can't be empty " << std::endl;
                else
                    tag_a1_447 = row[14];

                if (row[15].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[15], PartyRoles))
                {
                    tag_a1_452 = row[15];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[16].empty())
                    std::cout << " tag a2 448 can't be empty " << std::endl;
                else
                    tag_a2_448 = row[16];
                if (row[17].empty())
                    std::cout << " tag a2 447 can't be empty " << std::endl;
                else
                    tag_a2_447 = row[17];

                if (row[18].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[18], PartyRoles))
                {
                    tag_a2_452 = row[18];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[19].empty())
                    std::cout << " tag b 54 can't be empty " << std::endl;
                else
                    tag_b_54 = row[19];

                if (row[20].empty())
                    std::cout << " tag 453[2] can't be empty " << std::endl;
                else
                    tag_b_453 = row[20];

                if (row[21].empty())
                    std::cout << " tag b1 448 can't be empty " << std::endl;
                else
                    tag_b1_448 = row[21];

                if (row[22].empty())
                    std::cout << " tag b1 447 can't be empty " << std::endl;
                else
                    tag_b1_447 = row[22];

                if (row[23].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[23], PartyRoles))
                {
                    tag_b1_452 = row[23];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[24].empty())
                    std::cout << " tag b2 448 can't be empty " << std::endl;
                else
                    tag_b2_448 = row[24];

                if (row[25].empty())
                    std::cout << " tag b2 447 can't be empty " << std::endl;
                else
                    tag_b2_447 = row[25];

                if (row[26].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[26], PartyRoles))
                {
                    tag_b2_452 = row[26];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }
            }
        }

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderID
        message.getHeader().setField(35, "AE");       // MsgType sendCSVOrderNegoDealTwoSide
        message.getHeader().setField(49, "VP");
        message.getHeader().setField(56, "IDX");
        message.getHeader().setField(34, MsgSeqNum);
        message.getHeader().setField(43, "N");
        message.getHeader().setField(52, gmt_Time());

        char clId[20];
        gen_random(clId, 20);
        std::string clOrdId(clId);

        message.setField(571, tag_571);
        message.setField(487, tag_487);
        message.setField(856, tag_856);
        // message.setField(572,clOrdId);
        message.setField(574, "2");
        message.setField(64, tag_64);
        message.setField(919, tag_919);

        message.setField(55, tag_55);
        message.setField(762, tag_762);
        message.setField(31, tag_31);
        message.setField(32, tag_32);
        // message.setField(58, "Test C++ Jonec API sendCSVOrderNegoDealTwoSide");
        message.setField(60, GetFixCurrTime());

        FIX::Group enteringSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgEnteringNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));
        enteringSides.setField(54, tag_a_54);
        enteringSides.setField(581, tag_a_581);

        sgEnteringNoPartyIds.setField(448, tag_a1_448);
        sgEnteringNoPartyIds.setField(447, tag_a1_447);
        sgEnteringNoPartyIds.setField(452, tag_a1_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);

        sgEnteringNoPartyIds.setField(448, tag_a2_448);
        sgEnteringNoPartyIds.setField(447, tag_a2_447);
        sgEnteringNoPartyIds.setField(452, tag_a2_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);
        message.addGroup(enteringSides);

        FIX::Group contraSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgContraNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));

        contraSides.setField(54, tag_b_54);
        // contraSides.setField(581,tag_b_581);
        sgContraNoPartyIds.setField(448, tag_b1_448);
        sgContraNoPartyIds.setField(447, tag_b1_447);
        sgContraNoPartyIds.setField(452, tag_b1_452);
        contraSides.addGroup(sgContraNoPartyIds);

        sgContraNoPartyIds.setField(448, tag_b2_448);
        sgContraNoPartyIds.setField(447, tag_b2_447);
        sgContraNoPartyIds.setField(452, tag_b2_452);
        contraSides.addGroup(sgContraNoPartyIds);

        message.addGroup(contraSides);

        FIX::Message checkMsg(message.toString(), _dd, false);
        cout << "sendCSVOrderNegoDealTwoSide message: " << message.toString() << std::endl;
        return message.toString();
    }

    std::string sendCSVCancelOrderNegoDealTwoSideInisiator(const std::string dataFileCSVCancelOrderNegoDealTwoSideInisiator, const std::string senderID)
    {
        // construct fix message order
        FIX::Message message;

        std::string tag_571;
        std::string tag_856;
        std::string tag_572;
        std::string tag_64;
        std::string tag_919;
        std::string tag_55;
        std::string tag_762;
        std::string tag_31;
        std::string tag_32;
        std::string tag_552;
        std::string tag_c_54;
        std::string tag_c_581;
        std::string tag_c_453;
        std::string tag_c1_448;
        std::string tag_c1_447;
        std::string tag_c1_452;
        std::string tag_c2_448;
        std::string tag_c2_447;
        std::string tag_c2_452;
        std::string tag_d_54;
        std::string tag_d_581;
        std::string tag_d_453;
        std::string tag_d1_448;
        std::string tag_d1_447;
        std::string tag_d1_452;
        std::string tag_d2_448;
        std::string tag_d2_447;
        std::string tag_d2_452;

        char trdRptId[20];
        gen_random(trdRptId, 20);
        std::string tradeReportID(trdRptId);

        std::ifstream infile;
        infile.open(dataFileCSVCancelOrderNegoDealTwoSideInisiator);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileCSVCancelOrderNegoDealTwoSideInisiator << endl;
        }

        char buffer[4048];
        infile.read(buffer, sizeof(buffer));
        buffer[infile.tellg()] = '\0';

        // parse file, returns vector of rows
        std::vector<Row> result = parse(buffer, strlen(buffer));
        for (size_t r = 0; r < result.size(); r++)
        {
            Row &row = result[r];
            if (r == 1)
            {
                // std::cout <<"2"<<std::endl;
                /* for(size_t c=0; c < row.size() - 1; c++) {
                    //std::cout << row[c] << "\t";
                } */
                if (row[0].empty())
                {
                    std::cout << " tag 571 can't be empty " << std::endl;
                    tag_571 = tradeReportID;
                }
                else
                    tag_571 = row[0];

                tag_856 = row[1]; // 2
                tag_572 = row[2]; // 0

                if (row[3].empty())
                    tag_64 = getSettleDate(today);
                else
                    tag_64 = row[3];

                tag_919 = row[4];

                if (row[5].empty())
                    std::cout << " tag 55 can't be empty " << std::endl;
                else
                    tag_55 = row[5];

                tag_762 = row[6];
                if (tag_762.compare("NG") != 0)
                    tag_762 = "NG";

                if (row[7].empty())
                    std::cout << " tag 31 can't be empty " << std::endl;
                else
                    tag_31 = row[7];

                if (row[8].empty())
                    std::cout << " tag 32 can't be empty " << std::endl;
                else
                    tag_32 = row[8];

                tag_552 = row[9];

                if (row[10].empty())
                    std::cout << " tag a 54 can't be empty " << std::endl;
                else
                    tag_c_54 = row[10];

                tag_c_581 = row[11];

                if (row[12].empty())
                    std::cout << " tag 453[1] can't be empty " << std::endl;
                else
                    tag_c_453 = row[12];

                if (row[13].empty())
                    std::cout << " tag c1 448 can't be empty " << std::endl;
                else
                    tag_c1_448 = row[13];

                if (row[14].empty())
                    std::cout << " tag c1 447 can't be empty " << std::endl;
                else
                    tag_c1_447 = row[14];

                if (row[15].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[15], PartyRoles))
                {
                    tag_c1_452 = row[15];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[16].empty())
                    std::cout << " tag c2 448 can't be empty " << std::endl;
                else
                    tag_c2_448 = row[16];

                if (row[17].empty())
                    std::cout << " tag c2 447 can't be empty " << std::endl;
                else
                    tag_c2_447 = row[17];

                if (row[18].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[18], PartyRoles))
                {
                    tag_c2_452 = row[18];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[19].empty())
                    std::cout << " tag d 54 can't be empty " << std::endl;
                else
                    tag_d_54 = row[19];

                tag_d_581 = row[20];

                if (row[21].empty())
                    std::cout << " tag 453[2] can't be empty " << std::endl;
                else
                    tag_d_453 = row[21];

                if (row[22].empty())
                    std::cout << " tag d1 448 can't be empty " << std::endl;
                else
                    tag_d1_448 = row[22];

                if (row[23].empty())
                    std::cout << " tag d1 447 can't be empty " << std::endl;
                else
                    tag_d1_447 = row[23];

                if (row[24].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[24], PartyRoles))
                {
                    tag_d1_452 = row[24];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[25].empty())
                    std::cout << " tag d2 448 can't be empty " << std::endl;
                else
                    tag_d2_448 = row[25];

                if (row[26].empty())
                    std::cout << " tag d2 447 can't be empty " << std::endl;
                else
                    tag_d2_447 = row[26];

                if (row[27].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[27], PartyRoles))
                {
                    tag_d2_452 = row[27];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }
            }
        }

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderID
        message.getHeader().setField(35, "AE");       // MsgType sendCSVCancelOrderNegoDealTwoSideInisiator

        message.setField(571, tag_571);
        message.setField(856, tag_856); // 3
        message.setField(572, tag_572);

        message.setField(64, tag_64);
        message.setField(919, tag_919); // 1

        message.setField(55, tag_55);
        message.setField(762, tag_762);
        message.setField(31, tag_31);
        message.setField(32, tag_32);
        // message.setField(58, "Test C++ Jonec API sendCSVCancelOrderNegoDealTwoSideInisiator");
        // message.setField(60,GetFixCurrTime());

        message.setField(552, tag_552);

        FIX::Group enteringSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgEnteringNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));
        enteringSides.setField(54, tag_c_54);
        enteringSides.setField(581, tag_c_581);

        sgEnteringNoPartyIds.setField(448, tag_c1_448);
        sgEnteringNoPartyIds.setField(447, tag_c1_447);
        sgEnteringNoPartyIds.setField(452, tag_c1_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);

        sgEnteringNoPartyIds.setField(448, tag_c2_448);
        sgEnteringNoPartyIds.setField(447, tag_c2_447);
        sgEnteringNoPartyIds.setField(452, tag_c2_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);
        message.addGroup(enteringSides);

        FIX::Group contraSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgContraNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));
        contraSides.setField(54, tag_d_54);
        contraSides.setField(581, tag_d_581);

        sgContraNoPartyIds.setField(448, tag_d1_448);
        sgContraNoPartyIds.setField(447, tag_d1_447);
        sgContraNoPartyIds.setField(452, tag_d1_452);
        contraSides.addGroup(sgContraNoPartyIds);

        sgContraNoPartyIds.setField(448, tag_d2_448);
        sgContraNoPartyIds.setField(447, tag_d2_447);
        sgContraNoPartyIds.setField(452, tag_d2_452);
        contraSides.addGroup(sgContraNoPartyIds);

        message.addGroup(contraSides);

        try
        {
            FIX::Message checkMsg(message.toString(), _dd, false);
            cout << "sendCSVCancelOrderNegoDealTwoSideInisiator message: " << message.toString() << std::endl;
        }
        catch (std::exception &e)
        {
            cerr << e.what() << endl;
        }

        return message.toString();
    }

    std::string sendCSVOrderCancelRequest(const std::string dataFileCSVOrderCancelRequest, const std::string senderID)
    {
        FIX::Message message;
        std::string tag_11;

        char clId[20];
        gen_random(clId, 20);

        std::string clOrdId(clId);

        std::string tag_37;
        std::string tag_41;
        std::string tag_55;
        std::string tag_762;
        std::string tag_54;
        std::string tag_60;

        std::ifstream infile;
        infile.open(dataFileCSVOrderCancelRequest);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileCSVOrderCancelRequest << endl;
        }
        char buffer[1024];
        infile.read(buffer, sizeof(buffer));
        buffer[infile.tellg()] = '\0';

        // parse file, returns vector of rows
        std::vector<Row> result = parse(buffer, strlen(buffer));

        // print out result
        for (size_t r = 0; r < result.size(); r++)
        {
            Row &row = result[r];
            std::cout << std::endl;
            if (r == 1)
            {
                /* for(size_t c=0; c < row.size() - 1; c++) {
                    //std::cout << row[c] << "\t";
                } */

                if (row[0].empty())
                    tag_11 = clOrdId;
                else
                    tag_11 = row[0];

                tag_37 = row[1];
                if (row[2].empty())
                    std::cout << " tag 41 can't be empty " << std::endl;
                else
                    tag_41 = row[2];
                if (row[3].empty())
                    std::cout << " tag 55 can't be empty " << std::endl;
                else
                    tag_55 = row[3];
                tag_762 = row[4];
                if (row[5].empty())
                    std::cout << " tag 54 can't be empty " << std::endl;
                else
                    tag_54 = row[5];
                if (row[6].empty())
                    tag_60 = GetFixCurrTime();
                else
                    tag_60 = row[6];

                std::cout << "tag_11   : " << tag_11 << std::endl;
                std::cout << "tag_37   : " << tag_37 << std::endl;
                std::cout << "tag_41   : " << tag_41 << std::endl;
                std::cout << "tag_55   : " << tag_55 << std::endl;
                std::cout << "tag_762  : " << tag_762 << std::endl;
                std::cout << "tag_54   : " << tag_54 << std::endl;
                std::cout << "tag_60   : " << tag_60 << std::endl;
            }
        }

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderID
        message.getHeader().setField(35, "F");        // MsgType

        message.setField(11, tag_11);
        message.setField(41, tag_41);
        message.setField(37, tag_37);

        message.setField(54, tag_54);
        message.setField(55, tag_55);
        message.setField(60, tag_60);
        message.setField(58, "Test C++ Jonec API sendCSVOrderCancelRequest");

        FIX::Group NoPartyIDs(453, 448, FIX::message_order(448, 447, 452, 0));
        NoPartyIDs.setField(448, "555556");
        NoPartyIDs.setField(447, "C");
        NoPartyIDs.setField(452, "5");
        message.addGroup(NoPartyIDs);

        NoPartyIDs.setField(448, tag_11);
        NoPartyIDs.setField(447, "C");
        NoPartyIDs.setField(452, "3");
        message.addGroup(NoPartyIDs);

        FIX::Message checkMsg(message.toString(), _dd, false);
        cout << "sendCSVOrderCancelRequest message: " << message.toString() << std::endl;

        return message.toString();
    }

    std::string sendCSVOrderNegoAdvertisement(const std::string dataFileCSVOrderNegoAdvertisement, const std::string senderID)
    {
        /// construct fix message order
        FIX::Message message;
        char clId[20];
        gen_random(clId, 20);

        std::string clOrdId(clId);
        std::string tag_11;
        std::string tag_21;
        std::string tag_581;
        std::string tag_529;
        std::string tag_38;
        std::string tag_40;
        std::string tag_44;
        std::string tag_54;
        std::string tag_60;
        std::string tag_58;
        std::string tag_59;
        std::string tag_55;
        std::string tag_762;
        std::string tag_453;
        std::string tag_a_448;
        std::string tag_a_447;
        std::string tag_a_452;
        std::string tag_b_448;
        std::string tag_b_447;
        std::string tag_b_452;

        std::ifstream infile;
        // infile.open("new_order.csv");
        infile.open(dataFileCSVOrderNegoAdvertisement);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileCSVOrderNegoAdvertisement << endl;
        }
        char buffer[1024];
        infile.read(buffer, sizeof(buffer));
        buffer[infile.tellg()] = '\0';

        // parse file, returns vector of rows
        std::vector<Row> result = parse(buffer, strlen(buffer));

        // print out result
        for (size_t r = 0; r < result.size(); r++)
        {
            Row &row = result[r];
            if (r == 1)
            {
                /* for(size_t c=0; c < row.size() - 1; c++) {
                    std::cout << row[c] << "\t";
                } */
                // std::cout << row.back() << std::endl;

                if (row[0].empty())
                {
                    std::cout << " tag 11 can't be empty " << std::endl;
                    tag_11 = clOrdId;
                }
                else
                    tag_11 = row[0];

                tag_21 = row[1];

                if (row[2].empty())
                {
                    std::cout << " tag 581 can't be empty " << std::endl;
                }
                else
                    tag_581 = row[2];

                if (in_array(row[3], OrderRestrictions))
                {
                    tag_529 = row[3];
                }
                else
                {
                    std::cout << " fill in tag 529 with either ['q','r','s','t','u','v','w','x','y','z','Q','R','S','T','U','V','W','X','Y','Z'] " << std::endl;
                }

                if (row[4].empty())
                {
                    std::cout << " tag 38 can't be empty " << std::endl;
                }
                else
                    tag_38 = row[4];

                if (in_array(row[5], OrdTypes))
                {
                    tag_40 = row[5];
                }
                else
                {
                    std::cout << " fill in tag 40 with either ['1','2'] " << std::endl;
                }

                tag_44 = row[6];

                if (row[7].empty())
                {
                    std::cout << " tag 54 can't be empty " << std::endl;
                }
                else
                    tag_54 = row[7];

                if (row[8].empty())
                {
                    std::cout << " tag 60 can't be empty " << std::endl;
                    tag_60 = GetFixCurrTime();
                }
                else
                    tag_60 = row[8];

                tag_58 = row[9];
                tag_59 = row[10];

                if (row[11].empty())
                {
                    std::cout << " tag 55 can't be empty " << std::endl;
                }
                else
                    tag_55 = row[11];

                tag_762 = row[12];
                if (tag_762.compare("NG") != 0)
                    tag_762 = "NG";

                if (row[13].empty())
                    std::cout << " tag 453 can't be empty " << std::endl;
                else
                    tag_453 = row[13];

                if (row[14].empty())
                    std::cout << " tag 448 can't be empty " << std::endl;
                else
                    tag_a_448 = row[14];

                if (row[15].empty())
                    std::cout << " tag 447 can't be empty " << std::endl;
                else
                    tag_a_447 = row[15];

                if (row[16].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[16], PartyRoles))
                {
                    tag_a_452 = row[16];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[17].empty())
                {
                    std::cout << " tag 448[2] can't be empty " << std::endl;
                    tag_b_448 = clOrdId;
                }
                else
                    tag_b_448 = row[17];

                if (row[18].empty())
                    std::cout << " tag 447[2] can't be empty " << std::endl;
                else
                    tag_b_447 = row[18];

                if (row[19].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[19], PartyRoles))
                {
                    tag_b_452 = row[19];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }
            }
        }

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderCompID
        message.getHeader().setField(35, "D");        // MsgType

        /*const int len = 20;
        char clId[len];
        gen_random(clId, len);
        std::string clOrdId(clId);*/

        message.setField(11, tag_11);
        ;
        message.setField(21, tag_21);
        message.setField(581, tag_581);
        message.setField(529, tag_529);
        message.setField(38, tag_38);
        message.setField(40, tag_40);

        message.setField(44, tag_44); // order price
        message.setField(54, tag_54);
        // std::cout << "_CurrTime: " << GetFixCurrTime() << std::endl;
        message.setField(60, tag_60);
        // std::cout << "_CurrTime: " << _CurrTime << std::endl;
        message.setField(58, tag_58);
        message.setField(59, tag_59);
        message.setField(55, tag_55);

        message.setField(762, tag_762);
        // FIX::Group repeating_group(453, 2);
        // FIX::Group(453,448,FIX::message_order(448,447,452,802,0))
        FIX::Group NoPartyIDs(453, 448, FIX::message_order(448, 447, 452, 0));
        NoPartyIDs.setField(448, tag_a_448);
        NoPartyIDs.setField(447, tag_a_447);
        NoPartyIDs.setField(452, tag_a_452);
        message.addGroup(NoPartyIDs);

        NoPartyIDs.setField(448, tag_b_448);
        NoPartyIDs.setField(447, tag_b_447);
        NoPartyIDs.setField(452, tag_b_452);
        message.addGroup(NoPartyIDs);

        FIX::Message checkMsg(message.toString(), _dd, false);
        cout << "Test sendCSVOrderNegoAdvertisement message: " << message.toString() << std::endl;

        return message.toString();
    }

    std::string sendCSVOrderNegoDealTwoSideConfirmator(const std::string dataFileCSVOrderNegoDealTwoSideConfirmator, const std::string senderID)
    {
        // construct fix message order
        FIX::Message message;

        // const int len = 20;
        char trdRptId[20];
        gen_random(trdRptId, 20);

        std::string tradeReportID(trdRptId);

        std::string tag_571;
        std::string tag_487;
        std::string tag_856;
        std::string tag_572;
        std::string tag_64;
        std::string tag_919;
        std::string tag_55;
        std::string tag_762;
        std::string tag_31;
        std::string tag_32;
        std::string tag_552;
        std::string tag_a_54;
        std::string tag_a_581;
        std::string tag_a_453;
        std::string tag_a1_448;
        std::string tag_a1_447;
        std::string tag_a1_452;
        std::string tag_a2_448;
        std::string tag_a2_447;
        std::string tag_a2_452;
        std::string tag_b_54;
        std::string tag_b_581;
        std::string tag_b_453;
        std::string tag_b1_448;
        std::string tag_b1_447;
        std::string tag_b1_452;
        std::string tag_b2_448;
        std::string tag_b2_447;
        std::string tag_b2_452;
        std::string tag_b3_448;
        std::string tag_b3_447;
        std::string tag_b3_452;

        std::ifstream infile;
        infile.open(dataFileCSVOrderNegoDealTwoSideConfirmator);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileCSVOrderNegoDealTwoSideConfirmator << endl;
        }
        char buffer[2048];
        infile.read(buffer, sizeof(buffer));
        buffer[infile.tellg()] = '\0';

        // parse file, returns vector of rows
        std::vector<Row> result = parse(buffer, strlen(buffer));

        // print out result
        for (size_t r = 0; r < result.size(); r++)
        {
            Row &row = result[r];
            if (r == 1)
            {
                /* for(size_t c=0; c < row.size() - 1; c++) {
                    //std::cout << row[c] << "\t";
                } */

                if (row[0].empty())
                {
                    std::cout << " tag 571 can't be empty " << std::endl;
                    tag_571 = tradeReportID;
                }
                else
                    tag_571 = row[0];

                tag_487 = row[1]; // 2
                tag_856 = row[2]; // 2
                tag_572 = row[3]; // 0

                if (row[4].empty())
                    std::cout << " tag 55 can't be empty " << std::endl;
                else
                    tag_55 = row[4];

                tag_919 = row[5];
                tag_762 = row[6];
                if (tag_762.compare("NG") != 0)
                    tag_762 = "NG";

                if (row[7].empty())
                    std::cout << " tag 31 can't be empty " << std::endl;
                else
                    tag_31 = row[7];

                if (row[8].empty())
                    std::cout << " tag 32 can't be empty " << std::endl;
                else
                    tag_32 = row[8];

                tag_552 = row[9];

                if (row[10].empty())
                    std::cout << " tag a 54 can't be empty " << std::endl;
                else
                    tag_a_54 = row[10];

                tag_a_581 = row[11];

                if (row[12].empty())
                    std::cout << " tag 453[1] can't be empty " << std::endl;
                else
                    tag_a_453 = row[12];

                if (row[13].empty())
                    std::cout << " tag a1 448 can't be empty " << std::endl;
                else
                    tag_a1_448 = row[13];
                if (row[14].empty())
                    std::cout << " tag a1 447 can't be empty " << std::endl;
                else
                    tag_a1_447 = row[14];

                if (row[15].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[15], PartyRoles))
                {
                    tag_a1_452 = row[15];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[16].empty())
                    std::cout << " tag a2 448 can't be empty " << std::endl;
                else
                    tag_a2_448 = row[16];
                if (row[17].empty())
                    std::cout << " tag a2 447 can't be empty " << std::endl;
                else
                    tag_a2_447 = row[17];

                if (row[18].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[18], PartyRoles))
                {
                    tag_a2_452 = row[18];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[19].empty())
                    std::cout << " tag b 54 can't be empty " << std::endl;
                else
                    tag_b_54 = row[19];

                tag_b_581 = row[20];

                if (row[21].empty())
                    std::cout << " tag 453[2] can't be empty " << std::endl;
                else
                    tag_b_453 = row[21];

                if (row[22].empty())
                    std::cout << " tag b1 448 can't be empty " << std::endl;
                else
                    tag_b1_448 = row[22];

                if (row[23].empty())
                    std::cout << " tag b1 447 can't be empty " << std::endl;
                else
                    tag_b1_447 = row[23];

                if (row[24].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[24], PartyRoles))
                {
                    tag_b1_452 = row[24];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[25].empty())
                    std::cout << " tag b2 448 can't be empty " << std::endl;
                else
                    tag_b2_448 = row[25];

                if (row[26].empty())
                    std::cout << " tag b2 447 can't be empty " << std::endl;
                else
                    tag_b2_447 = row[26];

                if (row[27].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[27], PartyRoles))
                {
                    tag_b2_452 = row[27];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[28].empty())
                    std::cout << " tag b3 448 can't be empty " << std::endl;
                else
                    tag_b3_448 = row[28];

                if (row[29].empty())
                    std::cout << " tag b3 447 can't be empty " << std::endl;
                else
                    tag_b3_447 = row[29];

                if (row[30].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[30], PartyRoles))
                {
                    tag_b3_452 = row[30];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }
            }
        }

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderID
        message.getHeader().setField(35, "AE");       // MsgType sendCSVOrderNegoDealTwoSideConfirmator

        // const int len = 20; char clId[len];
        // gen_random(clId, len); std::string clOrdId(clId);

        message.setField(571, tag_571);
        message.setField(856, tag_856);
        message.setField(572, tag_572);

        // message.setField(64,tag_64);
        message.setField(919, tag_919);

        message.setField(55, tag_55);
        message.setField(762, tag_762);
        message.setField(31, tag_31);
        message.setField(32, tag_32);

        // message.setField(58, "Test C++ Jonec API sendCSVOrderNegoDealTwoSideConfirmator");

        message.setField(552, tag_552);

        FIX::Group enteringSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgEnteringNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));
        enteringSides.setField(54, tag_a_54);
        enteringSides.setField(581, tag_a_581);

        sgEnteringNoPartyIds.setField(448, tag_a1_448);
        sgEnteringNoPartyIds.setField(447, tag_a1_447);
        sgEnteringNoPartyIds.setField(452, tag_a1_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);

        sgEnteringNoPartyIds.setField(448, tag_a2_448);
        sgEnteringNoPartyIds.setField(447, tag_a2_447);
        sgEnteringNoPartyIds.setField(452, tag_a2_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);
        message.addGroup(enteringSides);

        FIX::Group contraSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgContraNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));

        contraSides.setField(54, tag_b_54);
        contraSides.setField(581, tag_b_581);
        sgContraNoPartyIds.setField(448, tag_b1_448);
        sgContraNoPartyIds.setField(447, tag_b1_447);
        sgContraNoPartyIds.setField(452, tag_b1_452);
        contraSides.addGroup(sgContraNoPartyIds);

        sgContraNoPartyIds.setField(448, tag_b2_448);
        sgContraNoPartyIds.setField(447, tag_b2_447);
        sgContraNoPartyIds.setField(452, tag_b2_452);
        contraSides.addGroup(sgContraNoPartyIds);

        sgContraNoPartyIds.setField(448, tag_b3_448);
        sgContraNoPartyIds.setField(447, tag_b3_447);
        sgContraNoPartyIds.setField(452, tag_b3_452);
        contraSides.addGroup(sgContraNoPartyIds);

        message.addGroup(contraSides);

        FIX::Message checkMsg(message.toString(), _dd, false);
        cout << "sendCSVOrderNegoDealTwoSideConfirmator message: " << message.toString() << std::endl;

        return message.toString();
    }

    std::string sendCSVCancelOrderNegoDealTwoSideConfirmator(const std::string dataFileCSVCancelOrderNegoDealTwoSideConfirmator, const std::string senderID)
    {
        // construct fix message order
        FIX::Message message;
        // const int len = 20;
        char trdRptId[20];
        gen_random(trdRptId, 20);

        std::string tradeReportID(trdRptId);

        std::string tag_571;
        std::string tag_487;
        std::string tag_856;
        std::string tag_572;
        std::string tag_64;
        std::string tag_919;
        std::string tag_55;
        std::string tag_762;
        std::string tag_31;
        std::string tag_32;
        std::string tag_552;
        std::string tag_a_54;
        std::string tag_a_581;
        std::string tag_a_453;
        std::string tag_a1_448;
        std::string tag_a1_447;
        std::string tag_a1_452;
        std::string tag_a2_448;
        std::string tag_a2_447;
        std::string tag_a2_452;
        std::string tag_b_54;
        std::string tag_b_581;
        std::string tag_b_453;
        std::string tag_b1_448;
        std::string tag_b1_447;
        std::string tag_b1_452;
        std::string tag_b2_448;
        std::string tag_b2_447;
        std::string tag_b2_452;

        std::ifstream infile;
        infile.open(dataFileCSVCancelOrderNegoDealTwoSideConfirmator);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileCSVCancelOrderNegoDealTwoSideConfirmator << endl;
        }

        char buffer[2048];
        infile.read(buffer, sizeof(buffer));
        buffer[infile.tellg()] = '\0';

        // parse file, returns vector of rows
        std::vector<Row> result = parse(buffer, strlen(buffer));

        // print out result
        for (size_t r = 0; r < result.size(); r++)
        {
            Row &row = result[r];
            if (r == 1)
            {
                /* for(size_t c=0; c < row.size() - 1; c++) {
                    std::cout << row[c] << "\t";
                } */

                if (row[0].empty())
                {
                    std::cout << " tag 571 can't be empty " << std::endl;
                    tag_571 = tradeReportID;
                }
                else
                    tag_571 = row[0];

                tag_487 = row[1]; // 2
                tag_856 = row[2]; // 2
                tag_572 = row[3]; // 0

                if (row[4].empty())
                    tag_64 = getSettleDate(today);
                else
                    tag_64 = row[4];

                tag_919 = row[5];

                if (row[6].empty())
                    std::cout << " tag 55 can't be empty " << std::endl;
                else
                    tag_55 = row[6];

                tag_762 = row[7];
                if (tag_762.compare("NG") != 0)
                    tag_762 = "NG";

                if (row[8].empty())
                    std::cout << " tag 31 can't be empty " << std::endl;
                else
                    tag_31 = row[8];

                if (row[9].empty())
                    std::cout << " tag 32 can't be empty " << std::endl;
                else
                    tag_32 = row[9];

                tag_552 = row[10];

                if (row[11].empty())
                    std::cout << " tag a 54 can't be empty " << std::endl;
                else
                    tag_a_54 = row[11];

                tag_a_581 = row[12];

                if (row[13].empty())
                    std::cout << " tag 453[1] can't be empty " << std::endl;
                else
                    tag_a_453 = row[13];

                if (row[14].empty())
                    std::cout << " tag a1 448 can't be empty " << std::endl;
                else
                    tag_a1_448 = row[14];
                if (row[15].empty())
                    std::cout << " tag a1 447 can't be empty " << std::endl;
                else
                    tag_a1_447 = row[15];

                if (row[16].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[16], PartyRoles))
                {
                    tag_a1_452 = row[16];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[17].empty())
                    std::cout << " tag a2 448 can't be empty " << std::endl;
                else
                    tag_a2_448 = row[17];
                if (row[18].empty())
                    std::cout << " tag a2 447 can't be empty " << std::endl;
                else
                    tag_a2_447 = row[18];

                if (row[19].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[19], PartyRoles))
                {
                    tag_a2_452 = row[19];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[20].empty())
                    std::cout << " tag b 54 can't be empty " << std::endl;
                else
                    tag_b_54 = row[20];

                tag_b_581 = row[21];

                if (row[22].empty())
                    std::cout << " tag 453[2] can't be empty " << std::endl;
                else
                    tag_b_453 = row[22];

                if (row[23].empty())
                    std::cout << " tag b1 448 can't be empty " << std::endl;
                else
                    tag_b1_448 = row[23];

                if (row[24].empty())
                    std::cout << " tag b1 447 can't be empty " << std::endl;
                else
                    tag_b1_447 = row[24];

                if (row[25].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[25], PartyRoles))
                {
                    tag_b1_452 = row[25];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[26].empty())
                    std::cout << " tag b2 448 can't be empty " << std::endl;
                else
                    tag_b2_448 = row[26];

                if (row[27].empty())
                    std::cout << " tag b2 447 can't be empty " << std::endl;
                else
                    tag_b2_447 = row[27];

                if (row[28].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[28], PartyRoles))
                {
                    tag_b2_452 = row[28];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }
            }
        }

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderID
        message.getHeader().setField(35, "AE");       // MsgType sendCSVCancelOrderNegoDealTwoSideConfirmator

        // const int len = 20; char clId[len];
        // gen_random(clId, len); std::string clOrdId(clId);

        message.setField(571, tag_571);
        message.setField(856, tag_856);
        message.setField(572, tag_572);

        message.setField(64, tag_64);
        message.setField(919, tag_919);

        message.setField(55, tag_55);
        message.setField(762, tag_762);
        message.setField(31, tag_31);
        message.setField(32, tag_32);
        // message.setField(58, "Test C++ Jonec API sendCSVCancelOrderNegoDealTwoSideConfirmator");
        message.setField(552, tag_552);

        FIX::Group enteringSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgEnteringNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));
        enteringSides.setField(54, tag_a_54);
        enteringSides.setField(581, tag_a_581);

        sgEnteringNoPartyIds.setField(448, tag_a1_448);
        sgEnteringNoPartyIds.setField(447, tag_a1_447);
        sgEnteringNoPartyIds.setField(452, tag_a1_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);

        sgEnteringNoPartyIds.setField(448, tag_a2_448);
        sgEnteringNoPartyIds.setField(447, tag_a2_447);
        sgEnteringNoPartyIds.setField(452, tag_a2_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);
        message.addGroup(enteringSides);

        FIX::Group contraSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgContraNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));

        contraSides.setField(54, tag_b_54);
        contraSides.setField(581, tag_b_581);
        sgContraNoPartyIds.setField(448, tag_b1_448);
        sgContraNoPartyIds.setField(447, tag_b1_447);
        sgContraNoPartyIds.setField(452, tag_b1_452);
        contraSides.addGroup(sgContraNoPartyIds);

        sgContraNoPartyIds.setField(448, tag_b2_448);
        sgContraNoPartyIds.setField(447, tag_b2_447);
        sgContraNoPartyIds.setField(452, tag_b2_452);
        contraSides.addGroup(sgContraNoPartyIds);

        message.addGroup(contraSides);

        try
        {
            FIX::Message checkMsg(message.toString(), _dd, false);
            cout << "sendCSVCancelOrderNegoDealTwoSideConfirmator message: " << message.toString() << std::endl;
        }
        catch (std::exception &e)
        {
            cerr << e.what() << endl;
        }
        return message.toString();
    }

    std::string sendCSVCancelNegoDealInternalCrossingConfirmator(const std::string dataFileCSVCancelNegoDealInternalCrossingConfirmator, const std::string senderID)
    {
        // cancel_negointernalcros_confirmator.csv
        FIX::Message message;

        std::string tag_571;
        std::string tag_856;
        std::string tag_572;
        std::string tag_64;
        std::string tag_919;
        std::string tag_55;
        std::string tag_762;
        std::string tag_31;
        std::string tag_32;
        std::string tag_552;
        std::string tag_a_54;
        std::string tag_a_581;
        std::string tag_a_453;
        std::string tag_a1_448;
        std::string tag_a1_447;
        std::string tag_a1_452;
        std::string tag_a2_448;
        std::string tag_a2_447;
        std::string tag_a2_452;
        std::string tag_b_54;
        std::string tag_b_581;
        std::string tag_b_453;
        std::string tag_b1_448;
        std::string tag_b1_447;
        std::string tag_b1_452;
        std::string tag_b2_448;
        std::string tag_b2_447;
        std::string tag_b2_452;
        std::string tag_b3_448;
        std::string tag_b3_447;
        std::string tag_b3_452;

        char trdRptId[20];
        gen_random(trdRptId, 20);
        std::string tradeReportID(trdRptId);

        std::ifstream infile;
        infile.open(dataFileCSVCancelNegoDealInternalCrossingConfirmator);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileCSVCancelNegoDealInternalCrossingConfirmator << endl;
        }
        char buffer[1024];
        infile.read(buffer, sizeof(buffer));
        buffer[infile.tellg()] = '\0';

        // parse file, returns vector of rows
        std::vector<Row> result = parse(buffer, strlen(buffer));

        // print out result
        for (size_t r = 0; r < result.size(); r++)
        {
            Row &row = result[r];
            if (r == 1)
            {
                // for(size_t c=0; c < row.size() - 1; c++) {
                //	std::cout << row[c] << "\t";
                // }

                if (row[0].empty())
                {
                    std::cout << " tag 571 can't be empty " << std::endl;
                    tag_571 = tradeReportID;
                }
                else
                    tag_571 = row[0];

                tag_856 = row[1]; // 2
                tag_572 = row[2]; // 0

                if (row[3].empty())
                    tag_64 = getSettleDate(today);
                else
                    tag_64 = row[3];

                tag_919 = row[4];

                if (row[5].empty())
                    std::cout << " tag 55 can't be empty " << std::endl;
                else
                    tag_55 = row[5];

                tag_762 = row[6];
                if (tag_762.compare("NG") != 0)
                    tag_762 = "NG";

                if (row[7].empty())
                    std::cout << " tag 31 can't be empty " << std::endl;
                else
                    tag_31 = row[7];

                if (row[8].empty())
                    std::cout << " tag 32 can't be empty " << std::endl;
                else
                    tag_32 = row[8];

                tag_552 = row[9];

                if (row[10].empty())
                    std::cout << " tag a 54 can't be empty " << std::endl;
                else
                    tag_a_54 = row[10];

                tag_a_581 = row[11];

                if (row[12].empty())
                    std::cout << " tag 453[1] can't be empty " << std::endl;
                else
                    tag_a_453 = row[12];

                if (row[13].empty())
                    std::cout << " tag a1 448 can't be empty " << std::endl;
                else
                    tag_a1_448 = row[13];
                if (row[14].empty())
                    std::cout << " tag a1 447 can't be empty " << std::endl;
                else
                    tag_a1_447 = row[14];

                if (row[15].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[15], PartyRoles))
                {
                    tag_a1_452 = row[15];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[16].empty())
                    std::cout << " tag a2 448 can't be empty " << std::endl;
                else
                    tag_a2_448 = row[16];
                if (row[17].empty())
                    std::cout << " tag a2 447 can't be empty " << std::endl;
                else
                    tag_a2_447 = row[17];

                if (row[18].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[18], PartyRoles))
                {
                    tag_a2_452 = row[18];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[19].empty())
                    std::cout << " tag b 54 can't be empty " << std::endl;
                else
                    tag_b_54 = row[19];

                tag_b_581 = row[20];

                if (row[21].empty())
                    std::cout << " tag 453[2] can't be empty " << std::endl;
                else
                    tag_b_453 = row[21];

                if (row[22].empty())
                    std::cout << " tag b1 448 can't be empty " << std::endl;
                else
                    tag_b1_448 = row[22];

                if (row[23].empty())
                    std::cout << " tag b1 447 can't be empty " << std::endl;
                else
                    tag_b1_447 = row[23];

                if (row[24].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[24], PartyRoles))
                {
                    tag_b1_452 = row[24];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[25].empty())
                    std::cout << " tag b2 448 can't be empty " << std::endl;
                else
                    tag_b2_448 = row[25];

                if (row[26].empty())
                    std::cout << " tag b2 447 can't be empty " << std::endl;
                else
                    tag_b2_447 = row[26];

                if (row[27].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[27], PartyRoles))
                {
                    tag_b2_452 = row[27];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[28].empty())
                    std::cout << " tag b3 448 can't be empty " << std::endl;
                else
                    tag_b3_448 = row[28];

                if (row[29].empty())
                    std::cout << " tag b3 447 can't be empty " << std::endl;
                else
                    tag_b3_447 = row[29];

                if (row[30].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[30], PartyRoles))
                {
                    tag_b3_452 = row[30];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }
            }
        }

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderCompID
        message.getHeader().setField(35, "AE");       // MsgType sendCSVCancelNegoDealInternalCrossingConfirmator

        message.setField(571, tag_571); // random
        message.setField(572, tag_572); // 0
        // if (!tag_856.empty())
        message.setField(856, tag_856); // 2
        message.setField(919, tag_919);

        message.setField(55, tag_55);
        message.setField(762, tag_762);
        message.setField(31, tag_31);
        message.setField(32, tag_32);
        // message.setField(58, "Test C++ Jonec API sendCSVCancelNegoDealInternalCrossingConfirmator");
        // message.setField(60,GetFixCurrTime());

        FIX::Group enteringSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgEnteringNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));
        enteringSides.setField(54, tag_a_54);
        enteringSides.setField(581, tag_a_581);

        sgEnteringNoPartyIds.setField(448, tag_a1_448);
        sgEnteringNoPartyIds.setField(447, tag_a1_447);
        sgEnteringNoPartyIds.setField(452, tag_a1_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);

        sgEnteringNoPartyIds.setField(448, tag_a2_448);
        sgEnteringNoPartyIds.setField(447, tag_a2_447);
        sgEnteringNoPartyIds.setField(452, tag_a2_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);
        message.addGroup(enteringSides);

        FIX::Group contraSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgContraNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));

        contraSides.setField(54, tag_b_54);
        contraSides.setField(581, tag_b_581);
        sgContraNoPartyIds.setField(448, tag_b1_448);
        sgContraNoPartyIds.setField(447, tag_b1_447);
        sgContraNoPartyIds.setField(452, tag_b1_452);
        contraSides.addGroup(sgContraNoPartyIds);

        sgContraNoPartyIds.setField(448, tag_b2_448);
        sgContraNoPartyIds.setField(447, tag_b2_447);
        sgContraNoPartyIds.setField(452, tag_b2_452);
        contraSides.addGroup(sgContraNoPartyIds);

        sgContraNoPartyIds.setField(448, tag_b3_448);
        sgContraNoPartyIds.setField(447, tag_b3_447);
        sgContraNoPartyIds.setField(452, tag_b3_452);
        contraSides.addGroup(sgContraNoPartyIds);
        message.addGroup(contraSides);

        FIX::Message checkMsg(message.toString(), _dd, false);
        cout << "sendCSVCancelNegoDealInternalCrossingConfirmator message: " << message.toString() << std::endl;

        return message.toString();
    }

    std::string sendCSVOrderNegoDealInternalCrossingConfirmator(const std::string dataFileCSVOrderNegoDealInternalCrossingConfirmator, const std::string senderID)
    {
        // construct fix message order
        FIX::Message message;

        std::string tag_571;
        std::string tag_487;
        std::string tag_856;
        std::string tag_572;

        std::string tag_55;
        std::string tag_919;
        std::string tag_762;
        std::string tag_31;
        std::string tag_32;
        std::string tag_552;

        std::string tag_a_54;
        std::string tag_a_581;
        std::string tag_a_453;
        std::string tag_a1_448;
        std::string tag_a1_447;
        std::string tag_a1_452;
        std::string tag_a2_448;
        std::string tag_a2_447;
        std::string tag_a2_452;

        std::string tag_b_54;
        std::string tag_b_581;
        std::string tag_b_453;
        std::string tag_b1_448;
        std::string tag_b1_447;
        std::string tag_b1_452;
        std::string tag_b2_448;
        std::string tag_b2_447;
        std::string tag_b2_452;
        std::string tag_b3_448;
        std::string tag_b3_447;
        std::string tag_b3_452;

        char trdRptId[20];
        gen_random(trdRptId, 20);
        std::string tradeReportID(trdRptId);

        // char trdRptId[20];
        std::ifstream infile;

        // infile.open("negdeal_internal_confirmator.csv");
        infile.open(dataFileCSVOrderNegoDealInternalCrossingConfirmator);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileCSVOrderNegoDealInternalCrossingConfirmator << endl;
        }
        char buffer[1024];
        infile.read(buffer, sizeof(buffer));
        buffer[infile.tellg()] = '\0';

        // parse file, returns vector of rows
        std::vector<Row> result = parse(buffer, strlen(buffer));

        // print out result
        for (size_t r = 0; r < result.size(); r++)
        {
            Row &row = result[r];
            if (r == 1)
            {
                /* for(size_t c=0; c < row.size() - 1; c++) {
                    std::cout << row[c] << "\t";
                } */

                if (row[0].empty())
                {
                    std::cout << " tag 571 can't be empty " << std::endl;
                    tag_571 = tradeReportID;
                }
                else
                    tag_571 = row[0];

                tag_487 = row[1];
                tag_856 = row[2];
                tag_572 = row[3];

                if (row[4].empty())
                    std::cout << " tag 55 can't be empty " << std::endl;
                else
                    tag_55 = row[4];

                tag_919 = row[5];
                tag_762 = row[6];
                if (tag_762.compare("NG") != 0)
                    tag_762 = "NG";

                if (row[7].empty())
                    std::cout << " tag 31 can't be empty " << std::endl;
                else
                    tag_31 = row[7];

                if (row[8].empty())
                    std::cout << " tag 32 can't be empty " << std::endl;
                else
                    tag_32 = row[8];

                tag_552 = row[9];

                if (row[10].empty())
                    std::cout << " tag a 54 can't be empty " << std::endl;
                else
                    tag_a_54 = row[10];

                tag_a_581 = row[11];

                if (row[12].empty())
                    std::cout << " tag 453[1] can't be empty " << std::endl;
                else
                    tag_a_453 = row[12];

                if (row[13].empty())
                    std::cout << " tag a1 448 can't be empty " << std::endl;
                else
                    tag_a1_448 = row[13];
                if (row[14].empty())
                    std::cout << " tag a1 447 can't be empty " << std::endl;
                else
                    tag_a1_447 = row[14];

                if (row[15].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[15], PartyRoles))
                {
                    tag_a1_452 = row[15];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[16].empty())
                    std::cout << " tag a2 448 can't be empty " << std::endl;
                else
                    tag_a2_448 = row[16];
                if (row[17].empty())
                    std::cout << " tag a2 447 can't be empty " << std::endl;
                else
                    tag_a2_447 = row[17];

                if (row[18].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[18], PartyRoles))
                {
                    tag_a2_452 = row[18];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[19].empty())
                    std::cout << " tag b 54 can't be empty " << std::endl;
                else
                    tag_b_54 = row[19];

                tag_b_581 = row[20];

                if (row[21].empty())
                    std::cout << " tag 453[2] can't be empty " << std::endl;
                else
                    tag_b_453 = row[21];

                if (row[22].empty())
                    std::cout << " tag b1 448 can't be empty " << std::endl;
                else
                    tag_b1_448 = row[22];

                if (row[23].empty())
                    std::cout << " tag b1 447 can't be empty " << std::endl;
                else
                    tag_b1_447 = row[23];

                if (row[24].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[24], PartyRoles))
                {
                    tag_b1_452 = row[24];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[25].empty())
                    std::cout << " tag b2 448 can't be empty " << std::endl;
                else
                    tag_b2_448 = row[25];

                if (row[26].empty())
                    std::cout << " tag b2 447 can't be empty " << std::endl;
                else
                    tag_b2_447 = row[26];

                if (row[27].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[27], PartyRoles))
                {
                    tag_b2_452 = row[24];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[28].empty())
                    std::cout << " tag b3 448 can't be empty " << std::endl;
                else
                    tag_b3_448 = row[28];

                if (row[29].empty())
                    std::cout << " tag b3 447 can't be empty " << std::endl;
                else
                    tag_b3_447 = row[29];

                if (row[30].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[30], PartyRoles))
                {
                    tag_b3_452 = row[30];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }
            }
        }

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderCompID
        message.getHeader().setField(35, "AE");       // MsgType sendCSVOrderNegoDealInternalCrossingConfirmator

        message.setField(571, tag_571); // random
        message.setField(487, tag_487); // 0
        // if (!tag_856.empty())
        message.setField(856, tag_856); // 2

        message.setField(572, tag_572);

        message.setField(919, tag_919);

        message.setField(55, tag_55);
        message.setField(762, tag_762);
        message.setField(31, tag_31);
        message.setField(32, tag_32);
        // message.setField(58, "Test C++ Jonec API sendCSVOrderNegoDealInternalCrossingConfirmator");
        // message.setField(60,GetFixCurrTime());

        FIX::Group enteringSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgEnteringNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));
        enteringSides.setField(54, tag_a_54);
        enteringSides.setField(581, tag_a_581);

        sgEnteringNoPartyIds.setField(448, tag_a1_448);
        sgEnteringNoPartyIds.setField(447, tag_a1_447);
        sgEnteringNoPartyIds.setField(452, tag_a1_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);

        sgEnteringNoPartyIds.setField(448, tag_a2_448);
        sgEnteringNoPartyIds.setField(447, tag_a2_447);
        sgEnteringNoPartyIds.setField(452, tag_a2_452);
        enteringSides.addGroup(sgEnteringNoPartyIds);
        message.addGroup(enteringSides);

        FIX::Group contraSides(552, 54, FIX::message_order(54, 581, 0));
        FIX::Group sgContraNoPartyIds(453, 448, FIX::message_order(448, 447, 452, 0));

        contraSides.setField(54, tag_b_54);
        contraSides.setField(581, tag_b_581);
        sgContraNoPartyIds.setField(448, tag_b1_448);
        sgContraNoPartyIds.setField(447, tag_b1_447);
        sgContraNoPartyIds.setField(452, tag_b1_452);
        contraSides.addGroup(sgContraNoPartyIds);

        sgContraNoPartyIds.setField(448, tag_b2_448);
        sgContraNoPartyIds.setField(447, tag_b2_447);
        sgContraNoPartyIds.setField(452, tag_b2_452);
        contraSides.addGroup(sgContraNoPartyIds);

        sgContraNoPartyIds.setField(448, tag_b3_448);
        sgContraNoPartyIds.setField(447, tag_b3_447);
        sgContraNoPartyIds.setField(452, tag_b3_452);
        contraSides.addGroup(sgContraNoPartyIds);

        message.addGroup(contraSides);

        FIX::Message checkMsg(message.toString(), _dd, false);
        cout << "sendCSVOrderNegoDealInternalCrossingConfirmator message: " << message.toString() << std::endl;

        return message.toString();
    }

    std::string sendCSVOrderCancelReplaceRequest(const std::string dataFileOrderCancelReplaceRequest, const std::string senderID)
    {
        FIX::Message message;
        std::string tag_11;

        char clId[20];
        gen_random(clId, 20);

        std::string clOrdId(clId);

        std::string tag_37;
        std::string tag_41;
        std::string tag_55;
        std::string tag_762;
        std::string tag_453;
        std::string tag_a_448;
        std::string tag_a_447;
        std::string tag_a_452;
        std::string tag_b_448;
        std::string tag_b_447;
        std::string tag_b_452;
        std::string tag_581;
        std::string tag_38;
        std::string tag_40;
        std::string tag_44;
        std::string tag_54;
        std::string tag_60;
        std::string tag_59;
        std::string tag_58;

        std::ifstream infile;
        infile.open(dataFileOrderCancelReplaceRequest);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileOrderCancelReplaceRequest << endl;
        }
        char buffer[1024];
        infile.read(buffer, sizeof(buffer));
        buffer[infile.tellg()] = '\0';

        // parse file, returns vector of rows
        std::vector<Row> result = parse(buffer, strlen(buffer));

        // print out result
        for (size_t r = 0; r < result.size(); r++)
        {
            Row &row = result[r];
            // std::cout << std::endl;
            if (r == 1)
            {
                /* for(size_t c=0; c < row.size() - 1; c++) {
                    //std::cout << row[c] << "\t";
                } */

                if (row[0].empty())
                {
                    std::cout << " tag 11 can't be empty " << std::endl;
                    tag_11 = clOrdId;
                }
                else
                    tag_11 = row[0];

                if (row[1].empty())
                    tag_37 = row[1];
                else
                    tag_37 = row[1];

                if (row[2].empty())
                    std::cout << " tag 41 can't be empty " << std::endl;
                else
                    tag_41 = row[2];

                if (row[3].empty())
                    std::cout << " tag 55 can't be empty " << std::endl;
                else
                    tag_55 = row[3];

                if (row[4].empty())
                    tag_762 = row[4];
                else
                    tag_762 = row[4];

                if (row[5].empty())
                    std::cout << " tag 453 can't be empty " << std::endl;
                else
                    tag_453 = row[5];

                if (row[6].empty())
                    std::cout << " tag 448 can't be empty " << std::endl;
                else
                    tag_a_448 = row[6];

                if (row[7].empty())
                    std::cout << " tag 447 can't be empty " << std::endl;
                else
                    tag_a_447 = row[7];

                // if (row[8].empty()) std::cout <<" tag 452 can't be empty " << std::endl;
                // else tag_a_452=row[8];
                if (row[8].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[8], PartyRoles))
                {
                    tag_a_452 = row[8];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[9].empty())
                    std::cout << " tag 448 can't be empty " << std::endl;
                else
                    tag_b_448 = row[9];

                if (row[10].empty())
                    std::cout << " tag 447 can't be empty " << std::endl;
                else
                    tag_b_447 = row[10];

                if (row[11].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[11], PartyRoles))
                {
                    tag_b_452 = row[11];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[12].empty())
                {
                    std::cout << " tag 581 can't be empty " << std::endl;
                }
                else if (in_array(row[12], AccountTypes))
                {
                    tag_581 = row[12];
                }
                else
                {
                    std::cout << " fill in tag 581 with either [1,3,100,101] " << std::endl;
                }

                if (row[13].empty())
                    std::cout << " tag 38 can't be empty " << std::endl;
                else
                    tag_38 = row[13];

                if (in_array(row[14], OrdTypes))
                {
                    tag_40 = row[14];
                }
                else
                {
                    std::cout << " fill in tag 40 with either ['1','2'] " << std::endl;
                }

                if (row[15].empty())
                    tag_44 = row[15];
                else
                    tag_44 = row[15];

                if (row[16].empty())
                    tag_54 = row[16];
                else
                    tag_54 = row[16];

                if (row[17].empty())
                {
                    std::cout << " tag 60 can't be empty " << std::endl;
                    tag_60 = GetFixCurrTime();
                }
                else
                    tag_60 = row[17];

                if (row[18].empty())
                    tag_59 = row[18];
                else
                    tag_59 = row[18];

                if (row[19].empty())
                    tag_58 = row[19];
                else
                    tag_58 = row[19];
            }
        }

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderID
        message.getHeader().setField(35, "G");        // MsgType

        message.setField(11, tag_11);
        message.setField(41, tag_11);
        message.setField(37, tag_37);
        message.setField(38, tag_38);
        message.setField(40, tag_40);
        message.setField(44, tag_44);
        message.setField(54, tag_54);
        message.setField(55, tag_55);
        message.setField(58, "Test C++ Jonec API sendCSVOrderCancelReplaceRequest");
        message.setField(59, tag_59);
        message.setField(60, tag_60);

        FIX::Group NoPartyIDs(453, 448, FIX::message_order(448, 447, 452, 0));
        NoPartyIDs.setField(448, tag_a_448);
        NoPartyIDs.setField(447, tag_a_447);
        NoPartyIDs.setField(452, tag_a_452);
        message.addGroup(NoPartyIDs);

        NoPartyIDs.setField(448, tag_b_448);
        NoPartyIDs.setField(447, tag_b_447);
        NoPartyIDs.setField(452, tag_b_452);
        message.addGroup(NoPartyIDs);
        message.setField(581, tag_581);

        FIX::Message checkMsg(message.toString(), _dd, false);
        cout << "sendCSVOrderCancelReplaceRequest message: " << message.toString() << std::endl;

        return message.toString();
    }

} // namespace msg

#endif