#include <iostream>
#include <fstream>
#include <string>

int main()
{
    // Buka file untuk dibaca
    std::ifstream inputFile("./LogFile/DbPesan/2024-01-26_[  DbPesan  ]_.log");
    __LINE__;
    // Periksa apakah file berhasil dibuka
    if (!inputFile.is_open())
    {
        std::cerr << "Gagal membuka file." << std::endl;
        return 1; // Keluar dari program dengan status error
    }

    // Variabel untuk menyimpan setiap baris
    std::string line;
    int line_number = 1;

    // Baca dan tampilkan setiap baris dari file
    while (std::getline(inputFile, line))
    {

        // if (line_number == 2)
        // {
        std::cout << "baris ke: " << line_number << "  " << line << std::endl;
        // }

        line_number++;
    }

    // Tutup file setelah selesai membaca
    inputFile.close();

    return 0; // Keluar dari program dengan status sukses
}
