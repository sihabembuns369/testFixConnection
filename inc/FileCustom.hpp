#ifndef __FILECUSTOM__
#define __FILECUSTOM__
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <vector>
#include "color.hpp"

class FileCustom
{
public:
    static std::vector<std::string> myVector;
    static bool ReadConFile(std::string filename);
    static int saveSeqNum(int client, int server);
    static bool HouseKeeping();
    static int getmsgSeqNum();
    static void writeToXml();
    static bool checkFileExtension();
    static void printFile();
    static int getmsgSeqNumServer();

private:
    static boost::property_tree::ptree pt;
    static std::string filesname;
    static bool _houseKeeping;
    static int _msgSeqNum;
    static int _msgSeqNumServer;
};

std::vector<std::string> FileCustom::myVector;
boost::property_tree::ptree FileCustom::pt;
std::string FileCustom::filesname;
bool FileCustom::_houseKeeping;
int FileCustom::_msgSeqNum;
int FileCustom::_msgSeqNumServer;

bool FileCustom::ReadConFile(std::string filename)
{
    filesname = filename;
    if (checkFileExtension())
    {
        try
        {
            // Membaca file XML
            boost::property_tree::read_xml(FileCustom::filesname, pt);
            // Mengakses root node
            boost::property_tree::ptree connection = pt.get_child("root.connection");
            boost::property_tree::ptree Appconfiguration = pt.get_child("root.Appconfiguration");

            FileCustom::myVector.clear();

            // Menelusuri setiap elemen dalam root node
            for (const auto &element : connection)
            {
                FileCustom::myVector.push_back(element.second.data());
            }

            for (const auto &element : Appconfiguration)
            {
                if (element.first == "HouseKeeping")
                {
                    if (element.second.data() == "true")
                    {
                        FileCustom::_houseKeeping = true;
                    }
                    else if (element.second.data() == "false")
                    {
                        FileCustom::_houseKeeping = false;
                    }
                }
                else if (element.first == "msgSeqNum")
                {
                    FileCustom::_msgSeqNum = stoi(element.second.data());
                }
                else if (element.first == "msgSeqNumServer")
                {
                    FileCustom::_msgSeqNumServer = stoi(element.second.data());
                }
            }
        }
        catch (const boost::property_tree::xml_parser_error &e)
        {
            std::cerr << RED << "Gagal Untuk Membaca File, [PESAN ERROR]:" << RESET << e.what() << std::endl;
        }
        return true;
    }
    else
    {
        return false;
    }
}

int FileCustom::saveSeqNum(int client, int sever)
{
    try
    {
        boost::property_tree::ptree tree;
        // chnage config
        tree.put("root.connection.host", myVector[0]);
        tree.put("root.connection.port", myVector[1]);
        tree.put("root.connection.senderID", myVector[2]);
        tree.put("root.connection.targetComp", myVector[3]);
        tree.put("root.connection.username", myVector[4]);
        tree.put("root.connection.tradeuser", myVector[5]);
        tree.put("root.connection.password", myVector[6]);
        tree.put("root.Appconfiguration.HouseKeeping", _houseKeeping);
        tree.put("root.Appconfiguration.msgSeqNum", client);
        tree.put("root.Appconfiguration.msgSeqNumServer", sever);

        boost::property_tree::write_xml(FileCustom::filesname, tree, std::locale(), boost::property_tree::xml_writer_settings<char>('\t', 1));

        // std::cout << "File konfigurasi telah berhasil diubah" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

bool FileCustom::HouseKeeping() // setter
{
    return FileCustom::_houseKeeping;
}

int FileCustom::getmsgSeqNum() // setter
{
    return FileCustom::_msgSeqNum;
}

int FileCustom::getmsgSeqNumServer() // setter
{
    FileCustom::ReadConFile(filesname);
    return FileCustom::_msgSeqNumServer;
}

void FileCustom::writeToXml()
{
    try
    {
        boost::property_tree::ptree tree;
        // chnage config
        tree.put("root.connection.host", myVector[0]);
        tree.put("root.connection.port", myVector[1]);
        tree.put("root.connection.senderID", myVector[2]);
        tree.put("root.connection.targetComp", myVector[3]);
        tree.put("root.connection.username", myVector[4]);
        tree.put("root.connection.tradeuser", myVector[5]);
        tree.put("root.connection.password", myVector[6]);
        tree.put("root.Appconfiguration.HouseKeeping", _houseKeeping);
        tree.put("root.Appconfiguration.msgSeqNum", FileCustom::getmsgSeqNum());
        tree.put("root.Appconfiguration.msgSeqNum", FileCustom::getmsgSeqNumServer());

        boost::property_tree::write_xml(filesname, tree, std::locale(), boost::property_tree::xml_writer_settings<char>('\t', 1));

        std::cout << "File konfigurasi telah berhasil diubah" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

bool FileCustom::checkFileExtension()
{
    std::string configFile = FileCustom::filesname;
    size_t found = configFile.find(".cnf");
    if (found != std::string::npos)
    {
        std::string file = configFile.substr(found);
        if (file.compare(".cnf"))
        {
            std::cout << RED << "File Ektensi Tidak Di diukung " << RESET << std::endl;
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}

void FileCustom::printFile()
{
    for (const auto &value : myVector)
    {
        std::cout << value << " ";
    }
}

#endif
// int main()
// {
//     FileCustom file;

//     file.ReadConFile("../conf/read.cnf");
//     file.printFile();
//     file.writeToXml();
//     file.printFile();

//     return 0;
// }
