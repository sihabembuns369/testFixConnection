#include "inc/WriteLog.hpp"
#include <iostream>

using namespace std;
#define BLUE    "\033[34m"

int main(){
WriteLog log;
log.logMessage("tes log", BLUE);

return 0;


}