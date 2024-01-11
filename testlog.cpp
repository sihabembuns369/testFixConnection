#include <iostream>
#include <string>
// #include <boost/spirit/include/qi.hpp>
// #include <boost/fusion/include/adapt_struct.hpp>
#include "inc/WriteLog.hpp"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"

int main() {
    WriteLog log;
    // log.HouseKeeping("LogFile/" ,"LogFile/HouseKeeping");
    log.logMessage("test", YELLOW, inp, 1);

    return 0;
}
