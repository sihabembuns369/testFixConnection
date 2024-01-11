#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <vector>
#include "color.hpp"
class FileCustom
{
public:
    std::vector<std::string> myVector;
   inline bool ReadConFile(std::string filename)
    {

        filesname = filename;
        if (checkFileExtension())
        {
            try
            {
                // Membaca file XML
                boost::property_tree::read_xml(filesname, pt);
                // Mengakses root node
                boost::property_tree::ptree connection = pt.get_child("root.connection");
                boost::property_tree::ptree Appconfiguration = pt.get_child("root.Appconfiguration");
            
                    myVector.clear();


                // Menelusuri setiap elemen dalam root node
                for (const auto &element : connection)
                {
                    // std::cout << "Tag: " << element.first << std::endl;
                    // std::cout << "  Value: " << element.second.data() << std::endl;
                        // std::cout << "  value: "<< attribute.second.data() << std::endl;
                    myVector.push_back(element.second.data());
                }

                for (const auto &element: Appconfiguration)
                {
                    std::cout << "  tag: " << element.first << std::endl;
                    if (element.first == "HouseKeeping")
                    {
                        if (element.second.data() == "y" )
                        {
                         _houseKeeping = true;
                        }else{
                            _houseKeeping =  false;
                        }
                    }else if(element.first == "msgSeqNum"){
                        _msgSeqNum = stoi(element.second.data());
                    }
                    

                }
                
            }
            catch (const boost::property_tree::xml_parser_error &e)
            {
                std::cerr << RED << "Failed to read configuration file. Error message:" << RESET << e.what() << std::endl;
                // return 1;
            }
            return true;

        }else{
            return false;
        }
    }
    int saveSeqNum(int value){
         try
        {
           
            boost::property_tree::ptree tree;
            // chnage config
            tree.put("root.connection.host",myVector[0]);
            tree.put("root.connection.port", myVector[1]);
            tree.put("root.connection.senderID", myVector[2]);
            tree.put("root.connection.targetComp", myVector[3]);
            tree.put("root.connection.username",myVector[4]);
            tree.put("root.connection.tradeuser", myVector[5]);
            tree.put("root.connection.password",myVector[6]);
            tree.put("root.Appconfiguration.HouseKeeping",HouseKeeping());
            tree.put("root.Appconfiguration.msgSeqNum",value);

            boost::property_tree::write_xml(filesname, tree, std::locale(),  boost::property_tree::xml_writer_settings<char>('\t', 1));

            std::cout << "The configuration file has been successfully changed." << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            // return 1;
        }
    }
    bool HouseKeeping(){
        return _houseKeeping;
    }
    int msgSeqNum(){
        return _msgSeqNum;
    }

    void writeToXml() //melakukan konfigurasi jika user menyetelnya
    {
        try
        {
           
            boost::property_tree::ptree tree;
            // chnage config
            tree.put("root.connection.host",myVector[0]);
            tree.put("root.connection.port", myVector[1]);
            tree.put("root.connection.senderID", myVector[2]);
            tree.put("root.connection.targetComp", myVector[3]);
            tree.put("root.connection.username",myVector[4]);
            tree.put("root.connection.tradeuser", myVector[5]);
            tree.put("root.connection.password",myVector[6]);
             tree.put("root.Appconfiguration.HouseKeeping",HouseKeeping());
            tree.put("root.Appconfiguration.msgSeqNum",msgSeqNum());

            boost::property_tree::write_xml(filesname, tree, std::locale(),  boost::property_tree::xml_writer_settings<char>('\t', 1));

            std::cout << "The configuration file has been successfully changed." << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            // return 1;
        }
    }

    bool checkFileExtension()
    {
        std::string configFile = filesname;
        size_t found = configFile.find(".cnf");
        if (found != std::string::npos)
        {
            std::string file = configFile.substr(found);
            if (file.compare(".cnf"))
            {
                std::cout << RED<< "unsupported file extensions" << RESET << std::endl;
                return false;
            }
            else
            {
                // std::cout << "supported file extensions " << std::endl;
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    void printFile()
    {
        for (const auto &value : myVector)
        {
            std::cout << value << " ";
        }
    }

private:
    boost::property_tree::ptree pt;
    std::string filesname;
    bool _houseKeeping;
    int _msgSeqNum;
};

// int main()
// {
//     FileCustom file;

//     file.ReadConFile("../conf/read.cnf");
//     file.printFile();
//     file.writeToXml();
//     file.printFile();

//     return 0;
// }
