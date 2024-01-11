#include "inc/FileCustom.hpp"
#include "inc/WriteLog.hpp"


int main()
{
    FileCustom file;
    WriteLog log;
    file.ReadConFile("./conf/read.cnf");
    file.printFile();
    std::cout<< "\n"; 

    std::cout << "seqnum: " << file.msgSeqNum() << std::endl;
    log.logMessage("test okeokeoke", GREEN, inp, file.HouseKeeping());
    std::string inpt;
    int t;
    while (true)
    {
      std::cout << "masukan y: " <<std::endl;
      std::cin >> inpt;

      if (inpt == "y" )
      {
        t++;
        file.saveSeqNum(t);
      }
      
    }
    
//    std::cout << "HouseKeeping: "<< file.HouseKeeping() << std::endl;
    // file.writeToXml();
    // file.printFile();

    return 0;
}
