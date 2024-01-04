#ifndef IDXUTIL_H
#define IDXUTIL_H

#include <stdio.h> // needed by kbhit
#include <termios.h> // needed by kbhit
#include <unistd.h> // needed by kbhit
#include <fcntl.h> // needed by kbhit
#include <iostream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <sstream>
#include <climits>
#include <queue>
#include <boost/thread.hpp>

namespace idx
{

void GetStringNextToken(std::string &Receive, std::string &Result, std::string Token);
void Int64ToString(int64_t Val, std::string &Result, const std::string DefaultIfFailed);
void IntToString(int Val, std::string &Result, const std::string DefaultIfFailed);
void StringToInt(std::string Val, int &Result, const int DefaultIfFailed);
void StringToInt64(std::string Val, int64_t &Result, const int16_t DefaultIfFailed);
int kbhit(void);

// trim from start
static inline std::string &ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s)
{
    return ltrim(rtrim(s));
}


}

#endif // IDXUTIL_H



