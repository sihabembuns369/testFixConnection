#include <iostream>
#include <fstream>
#include <ctime>
#include <string>

// ANSI escape codes for text colors
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"

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
        return ss.str();
    }

    // Fungsi untuk menulis pesan log ke file
    void logMessage(const std::string& message , const std::string color) {
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