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
                boost::property_tree::ptree root = pt.get_child("root");

                // Menelusuri setiap elemen dalam root node
                for (const auto &element : root)
                {
                    // std::cout << "Tag: " << element.first << std::endl;
                    myVector.clear();

                    // Menelusuri atribut setiap elemen (jika ada)
                    for (const auto &attribute : element.second)
                    {
                        // std::cout << "  Attribute: " << attribute.first << " = " << attribute.second.data() << std::endl;
                        // std::cout << "  value: "<< attribute.second.data() << std::endl;
                        myVector.push_back(attribute.second.data());
                    }

                    // Mendapatkan nilai teks dari elemen (jika ada)
                    // std::cout << "  Text: " << element.second.data() << std::endl;
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

    void writeToXml()
    {
        try
        {
            // Membuat pohon properti (property tree)
            boost::property_tree::ptree tree;
            // Menambahkan elemen dan atribut ke dalam pohon properti
            tree.put("root.connection.host",myVector[0]);
            tree.put("root.connection.port", myVector[1]);
            tree.put("root.connection.senderID", myVector[2]);
            tree.put("root.connection.targetComp", myVector[3]);
            tree.put("root.connection.username",myVector[4]);
            tree.put("root.connection.tradeuser", myVector[5]);
            tree.put("root.connection.password",myVector[6]);

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
        for (const auto &element : myVector)
        {
            std::cout << element << " ";
        }
    }

private:
    boost::property_tree::ptree pt;
    std::string filesname;
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
