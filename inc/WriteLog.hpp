#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <sstream>

#include <cstdio>
#include <dirent.h>
// #include "../inc/netstat.hpp"

// Warna ANSI Escape Codes
#define RESET     "\033[0m"
#define BLACK     "\033[30m"
#define RED       "\033[31m"
#define GREEN     "\033[32m"
#define YELLOW    "\033[33m"
#define BLUE      "\033[34m"
#define MAGENTA   "\033[35m"
#define CYAN      "\033[36m"
#define WHITE     "\033[37m"
#define BOLDBLACK     "\033[1m\033[30m"
#define BOLDRED       "\033[1m\033[31m"
#define BOLDGREEN     "\033[1m\033[32m"
#define BOLDYELLOW    "\033[1m\033[33m"
#define BOLDBLUE      "\033[1m\033[34m"
#define BOLDMAGENTA   "\033[1m\033[35m"
#define BOLDCYAN      "\033[1m\033[36m"
#define BOLDWHITE     "\033[1m\033[37m"


class WriteLog{
    public:
        WriteLog(const std::string &logFilename= "") : logFilename (logFilename) {}

        std::string currentDateTime() {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
        std::stringstream ss;
        ss << buf;
       std::string files = "./LogFile/" + ss.str().substr(0,10) +".log";
       std::ifstream file(files.c_str());
    //    std::cout << files.c_str() << std::endl;
        if (file.is_open()){
            // std::cout << "file ada" << std::endl;
            return ss.str();
        }else
        {
            //   std::cout << "file tidak ada" << std::endl;
            return buf;
        }
        
        return buf;
        
       
    }

    std::string timecurrent(){
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y%m%d-%X", &tstruct);

        return buf;
    }

    void HouseKeeping(const char *folderAsal,const char *folderTujuan ){
    DIR *dirAsal;
    struct dirent *entry;

    // Buka folder asal
    if ((dirAsal = opendir(folderAsal)) != nullptr) {
        while ((entry = readdir(dirAsal)) != nullptr) {
            if (entry->d_type == DT_REG) {
                std::string fileNamaAsal = folderAsal + std::string("/") + entry->d_name;
                std::string fileNamaTujuan = folderTujuan + std::string("/") + entry->d_name;
                // std::cout << "file: " + fileNamaAsal << std::endl;

                if (fileNamaAsal == folderAsal + std::string("/") + currentDateTime().substr(0,10)+".log" )
                {
                //  std::cout << "file detection: " << std::endl;
                }else{
                   // Memindahkan file
                if (rename(fileNamaAsal.c_str(), fileNamaTujuan.c_str()) != 0) {
                    perror("Gagal memindahkan file");
                } else {
                    std::cout << "File " << fileNamaAsal << " berhasil dipindahkan ke " << fileNamaTujuan << std::endl;
                }
                }
                

               
            }
        }
        closedir(dirAsal);
    } else {
        perror("Error accessing folder");
        // return 1;
    }


    }


    void logMessage(const std::string& message , const std::string color) {
        HouseKeeping("LogFile/" ,"LogFile/HouseKeeping");
        // Buka file log untuk menulis (tambahkan mode)
        std::ofstream logFile("./LogFile/"+ currentDateTime().substr(0,10)+".log", std::ios::app);

        // Periksa apakah file berhasil dibuka
        if (!logFile.is_open()) {
            std::cerr << "Gagal membuka file log" << std::endl;
            return;
        }

        // Tulis timestamp dan pesan log ke file
        logFile << YELLOW << "[" << currentDateTime() << "] " RESET <<  " " << color << message << RESET << std::endl;

        // Tutup file log
        logFile.close();
    }

//     std::string tmToString(const tm* timeInfo) {
//         std::stringstream ss;
//         std::cout << "test" << std::endl;
//         ss << std::put_time(timeInfo, "%Y-%m-%d %X");
//         WriteLog("./LogFile/"+ ss.str() + ".log");
//         return ss.str();
// }


    private:
        std::string logFilename;

};