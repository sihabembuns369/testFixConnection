#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <sstream>

#include <sys/utsname.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <dirent.h>
#include "color.hpp"
std::string inp = "input";
std::string outp = "output";

class WriteLog
{
public:
    WriteLog(const std::string &logFilename = "") : logFilename(logFilename) {}
    std::string gmt_Time();
    std::string timecurrent();
    void logMessage(const std::string &message, const std::string color, const std::string mode, bool autoHouseKeeping);
    void SaveToDb(const std::string &pesan);

private:
    std::string logFilename;
    std::string logdir;
    std::string fileLogDir = "./LogFile/";
    std::string _message;
    std::string currentDateTime();
    void readOsInfo();
    void HouseKeeping(const char *folderAsal, const char *folderTujuan, std::string filename);
};

std::string WriteLog::currentDateTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    std::stringstream ss;
    ss << buf;
    std::string files = "./LogFile/" + ss.str().substr(0, 10) + ".log";
    std::ifstream file(files.c_str());
    //    std::cout << files.c_str() << std::endl;
    if (file.is_open())
    {
        // std::cout << "file ada" << std::endl;
        return ss.str();
    }
    else
    {
        //   std::cout << "file tidak ada" << std::endl;
        return buf;
    }
    return buf;
}
std::string WriteLog::gmt_Time()
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

std::string WriteLog::timecurrent()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y%m%d-%X", &tstruct);

    return buf;
}

void WriteLog::logMessage(const std::string &message, const std::string color, const std::string mode, bool autoHouseKeeping)
{

    logdir = currentDateTime().substr(0, 10) + "_[" + mode + "]_.log";
    _message = message;

    // std::cout << "logDir: " << logdir << std::endl;
    if (autoHouseKeeping == 1 || autoHouseKeeping == true)
    {
        // std::cout << GREEN<<"HouseKeeping on"<<RESET<< std::endl;
        if (mode == inp)
        {
            WriteLog::HouseKeeping("LogFile/FromServer", "LogFile/HouseKeeping/FromServer", logdir);
        }
        else if (mode == outp)
        {
            WriteLog::HouseKeeping("LogFile/FromApp", "LogFile/HouseKeeping/FromApp", logdir);
        }
    }

    // Buka file log untuk menulis (tambahkan mode)

    if (mode == inp)
    {
        fileLogDir = "./LogFile/FromServer/";
    }
    else if (mode == outp)
    {
        fileLogDir = "./LogFile/FromApp/";
    }

    // std::ofstream logFile(fileLogDir + currentDateTime().substr(0,10)+".log", std::ios::app);
    std::ofstream logFile(fileLogDir + currentDateTime().substr(0, 10) + "_[" + mode + "]_.log", std::ios::app);

    // Periksa apakah file berhasil dibuka
    if (!logFile.is_open())
    {
        std::cerr << "Gagal membuka file log" << std::endl;
        return;
    }

    // Tulis timestamp dan pesan log ke file
    logFile << YELLOW << "[" << currentDateTime() << "] " RESET << " " << color << message << RESET << std::endl;
    // Tutup file log
    logFile.close();
}

void WriteLog::SaveToDb(const std::string &pesan)
{

    std::cout << "fungsi save to db fi panggil" << std::endl;
    std::ofstream dbPesan("./LogFile/DbPesan/" + currentDateTime().substr(0, 10) + "_[  DbPesan  ]_.log", std::ios::app);
    if (!dbPesan.is_open())
    {
        std::cerr << "Gagal membuka file log2" << std::endl;
        return;
    }
    dbPesan << pesan << std::endl;
    dbPesan.close();
}

void WriteLog::readOsInfo()
{
    struct utsname info;
    std::string message;
    const char *osInfo[] = {info.nodename, info.sysname, info.version, info.machine};
    const int size = 5;
    std::string space[size];
    int count = 0;
    pid_t pid = getpid();
    if (uname(&info) == 0)
    {
        for (const auto &name : osInfo)
        {
            count++;
            for (size_t i = 1; i <= (55 - std::strlen(name)); i++)
            {
                // std::cout << i << " ";
                space[count] += " ";
            }
            // std::cout << "\n"
            //           << std::endl;
        }

        std::cout << space[4].size() << "b" << std::endl;
        std::cout << "length: " << std::string(4, 'b') << std::endl;
        message += "[ " + WriteLog::currentDateTime() + " ] Run On PID: " + std::to_string(pid) + "\n";
        message += "#***************************************************************************#\n";
        message += "#                          READ OS INFORMATION                              #\n";
        message += "#---------------------------------------------------------------------------#\n";
        message += "# HostName         : ";
        message += info.nodename;
        message += space[1] + "#\n";
        message += "# Operating System : ";
        message += info.sysname;
        message += space[2] + "#\n";
        message += "# Version          : ";
        message += info.version;
        message += space[3] + "#\n";
        message += "# Architecture     : ";
        message += info.machine;
        message += space[4] + "#\n";
        message += "#***************************************************************************#\n";
        // message += "";
    }
    else
    {
        message = "Gagal mendapatkan informasi sistem operasi.";
    }

    std::ofstream logFile("./LogFile/" + WriteLog::currentDateTime().substr(0, 10) + "_[ OsInfo ]_.log", std::ios::app);
    if (!logFile.is_open())
    {
        std::cerr << "Gagal membuka file log" << std::endl;
        return;
    }
    logFile << message << std::endl;
    logFile.close();
}

void WriteLog::HouseKeeping(const char *folderAsal, const char *folderTujuan, std::string filename)
{
    DIR *dirAsal;
    struct dirent *entry;

    // Buka folder asal
    if ((dirAsal = opendir(folderAsal)) != nullptr)
    {
        while ((entry = readdir(dirAsal)) != nullptr)
        {
            if (entry->d_type == DT_REG)
            {
                std::string fileNamaAsal = folderAsal + std::string("/") + entry->d_name;
                std::string fileNamaTujuan = folderTujuan + std::string("/") + entry->d_name;
                // std::cout << "file: " + fileNamaAsal << std::endl;
                // std::cout << "folderAsal: " <<  folderAsal + std::string("/") + filename << std::endl;

                if (fileNamaAsal == folderAsal + std::string("/") + filename)
                {
                    //  std::cout << "file detection: " << std::endl;
                }
                else
                {
                    // Memindahkan file
                    if (rename(fileNamaAsal.c_str(), fileNamaTujuan.c_str()) != 0)
                    {
                        perror("Gagal memindahkan file");
                    }
                    else
                    {
                        std::cout << "File " << fileNamaAsal << " berhasil dipindahkan ke " << fileNamaTujuan << std::endl;
                    }
                }
            }
        }
        closedir(dirAsal);
    }
    else
    {
        perror("Error accessing folder");
        // return 1;
    }
}