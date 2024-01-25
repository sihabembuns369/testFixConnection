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

        std::string tag_724;
        std::string tag_263;
        std::string tag_453;
        std::string tag_448;
        std::string tag_447;
        std::string tag_452;
        std::string tag_715;
        std::string tag_60;

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

    std::string sendCSVNuewOrder(const std::string dataFileCSV, const std::string senderID, int msgnum)
    {
        FIX::Message message;
        std::string MsgSeqNum = std::to_string(msgnum);

        char clId[20];
        gen_random(clId, 20);

        std::string clOrdId(clId);

        std::string tag_11;
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
        infile.open(dataFileCSV);
        if (!infile)
        {
            cerr << "Can't open input file " << dataFileCSV << endl;
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
                for (size_t c = 0; c < row.size() - 1; c++)
                {
                    std::cout << row[c] << "\t";
                }

                if (row[0].empty())
                {
                    std::cout << " tag 11 can't be empty " << std::endl;
                    tag_11 = clOrdId;
                }
                else
                    tag_11 = row[0];

                if (row[1].empty())
                {
                    std::cout << " tag 581 can't be empty " << std::endl;
                }
                else if (in_array(row[1], AccountTypes))
                {
                    tag_581 = row[1];
                }
                else
                {
                    std::cout << " fill in tag 581 with either [1,3,100,101] " << std::endl;
                }

                // tag_529=row[2];
                if (in_array(row[2], OrderRestrictions))
                {
                    tag_529 = row[2];
                }
                else
                {
                    std::cout << " fill in tag 529 with either ['q','r','s','t','u','v','w','x','y','z','Q','R','S','T','U','V','W','X','Y','Z'] " << std::endl;
                }

                if (row[3].empty())
                    std::cout << " tag 38 can't be empty " << std::endl;
                else
                    tag_38 = row[3];

                /* if (row[4].empty()) std::cout <<" tag 40 can't be empty " << std::endl;
                else tag_40=row[4];
                 */

                if (in_array(row[4], OrdTypes))
                {
                    tag_40 = row[4];
                }
                else
                {
                    std::cout << " fill in tag 40 with either ['1','2'] " << std::endl;
                }

                tag_44 = row[5];

                if (row[6].empty())
                {
                    std::cout << " tag 54 can't be empty " << std::endl;
                }
                else if (in_array(row[6], Sides))
                {
                    tag_54 = row[6];
                }
                else
                {
                    std::cout << " fill in tag 54 with either [1,2,5,M,P] " << std::endl;
                }

                if (row[7].empty())
                    tag_60 = GetFixCurrTime();
                else
                    tag_60 = row[7];
                tag_58 = row[8];
                tag_59 = row[9];
                if (row[10].empty())
                    std::cout << " tag 55 can't be empty " << std::endl;
                else
                    tag_55 = row[10];
                tag_762 = row[11];

                if (row[12].empty())
                    std::cout << " tag 453 can't be empty " << std::endl;
                else
                    tag_453 = row[12];

                if (row[13].empty())
                    std::cout << " tag 448 can't be empty " << std::endl;
                else
                    tag_a_448 = row[13];

                if (row[14].empty())
                    std::cout << " tag 447 can't be empty " << std::endl;
                else
                    tag_a_447 = row[14];

                // if (row[15].empty()) std::cout <<" tag 452 can't be empty " << std::endl;
                // else tag_a_452=row[15];

                if (row[15].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[15], PartyRoles))
                {
                    tag_a_452 = row[15];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }

                if (row[16].empty())
                    tag_b_448 = clOrdId;
                else
                    tag_b_448 = row[16];
                if (row[17].empty())
                    std::cout << " tag 447[2] can't be empty " << std::endl;
                else
                    tag_b_447 = row[17];

                // if (row[18].empty()) std::cout <<" tag 452[2] can't be empty " << std::endl;
                // else tag_b_452=row[18];
                if (row[18].empty())
                {
                    std::cout << " tag 452 can't be empty " << std::endl;
                }
                else if (in_array(row[18], PartyRoles))
                {
                    tag_b_452 = row[18];
                }
                else
                {
                    std::cout << " fill in tag 452 with either [3,5,7,17,36,37] " << std::endl;
                }
            }
        }

        message.getHeader().setField(8, BEGINSTRING); // BeginString
        message.getHeader().setField(50, senderID);   // SenderID
        message.getHeader().setField(35, "D");        // MsgType
        message.getHeader().setField(49, "VP");
        message.getHeader().setField(56, "IDX");
        message.getHeader().setField(34, MsgSeqNum);
        message.getHeader().setField(43, "N");
        message.getHeader().setField(52, gmt_Time());

        message.setField(11, tag_11);
        message.setField(581, tag_581);
        // message.setField(529, tag_529);
        message.setField(38, tag_38);
        message.setField(40, tag_40);

        message.setField(44, "50000"); // order price
        message.setField(54, tag_54);
        message.setField(60, gmt_Time());
        // message.setField(58, tag_58);
        message.setField(59, tag_59);
        message.setField(55, "ANTM");

        message.setField(762, tag_762);
        FIX::Group NoPartyIDs(453, 448, FIX::message_order(448, 447, 452, 0));
        NoPartyIDs.setField(448, "1000");
        NoPartyIDs.setField(447, "C");
        NoPartyIDs.setField(452, "5");
        message.addGroup(NoPartyIDs);

        // try
        // {
        FIX::Message checkMsg(message.toString(), _dd, false);
        cout << "sendCSVOrderSingle message: " << message.toString() << std::endl;
        // }
        // catch (std::exception &e)
        // {
        //     cerr << e.what() << endl;
        // }
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
        std::string tag_117;
        std::string tag_453;
        std::string tag_a_448;
        std::string tag_a_447;
        std::string tag_a_452;
        std::string tag_b_448;
        std::string tag_b_447;
        std::string tag_b_452;
        std::string tag_581;
        std::string tag_296;
        std::string tag_302;
        std::string tag_304;
        std::string tag_295;
        std::string tag_299;
        std::string tag_55;
        std::string tag_762;
        std::string tag_132;
        std::string tag_133;
        std::string tag_134;
        std::string tag_135;

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
        message.getHeader().setField(43, "N");
        message.getHeader().setField(52, gmt_Time());

        message.setField(571, tag_571); // random
        // if (!tag_856.empty())
        message.setField(856, tag_856); // 2
        message.setField(919, tag_919);
        message.setField(572, tag_572); // 0

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

} // namespace msg

#endif