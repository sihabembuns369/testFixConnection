
#include "../include/IDXUtil.h"

namespace idx
{


int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}


void GetStringNextToken(std::string &Receive, std::string &Result, std::string Token)
{
    if (Receive.empty())
        return;
    else
    {
        size_t found = Receive.find(Token);
        if (found!=std::string::npos)
        {
            Result = Receive.substr(0, found);
            Receive.erase(0, Result.size()+Token.size());
        }
    }
}

void Int64ToString(int64_t Val, std::string &Result, const std::string DefaultIfFailed)
{
    // max 64 bit from -9223372036854775808 to 9223372036854775807
//    char buff[20];
//    try{
//        _i64toa(Val, buff, 10);
//        std::string s(buff, 20);
//        Result = trim(s);
//    }catch(std::exception &e){
//        Result = DefaultIfFailed;
//    }
    try
    {
        if ((Val > LLONG_MIN) && (Val < LLONG_MAX))
        {
            std::ostringstream oss;
            oss << Val;
            Result = oss.str();
        }
        else
            Result = DefaultIfFailed;
    }
    catch(std::exception &e)
    {
        Result = DefaultIfFailed;
    }
}

void IntToString(int Val, std::string &Result, const std::string DefaultIfFailed)
{
    try
    {
        std::ostringstream oss;
        oss << Val;
        Result = oss.str();
    }
    catch(std::exception &e)
    {
        Result = DefaultIfFailed;
    }
}

void StringToInt(std::string Val, int &Result, const int DefaultIfFailed)
{
    try
    {
        std::istringstream iss(Val);
        iss >> Result;
    }
    catch(std::exception &e)
    {
        Result = DefaultIfFailed;
    }
}

void StringToInt64(std::string Val, int64_t &Result, const int16_t DefaultIfFailed)
{
    try
    {
        std::istringstream iss(Val);
        iss >> Result;
    }
    catch(std::exception &e)
    {
        Result = DefaultIfFailed;
    }
}

}
