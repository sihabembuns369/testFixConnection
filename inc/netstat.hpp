#include <iostream>
#include <cstdio>
#include <memory>
#include <string>

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

int main() {
    // Menjalankan perintah netstat dan menangkap hasilnya
    std::string netstatOutput = exec("netstat");

    // Menampilkan hasilnya
    std::cout << "Netstat Output:\n" << netstatOutput << std::endl;

    return 0;
}
