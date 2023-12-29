#include <iostream>
#include <cstdio>
#include <memory>
#include <string>

class netstat{
    public:



std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

};

// int main() {
//     netstat net;
//     // Menjalankan perintah netstat dan menangkap hasilnya
//     std::string netstatOutput = net.exec("netstat -an | grep 59881");

//     // Menampilkan hasilnya
//     // std::cout  << netstatOutput << std::endl;

//     if (netstatOutput.size() <=0){
//         std::cout << "tidak ada keluaran" << std::endl;
//     }else{
//   std::cout << netstatOutput << std::endl;
//     }
    

//     return 0;
// }
