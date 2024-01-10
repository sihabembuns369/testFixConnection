#include <stdio.h>
#include "inc/WriteLog.hpp"
#include "inc/color.hpp"


int main() {

    WriteLog log;

    
    char controlA = '\x01'; // Representasi karakter ^A dalam bahasa C
    printf("Karakter ^A: %c\n", controlA);
     std::cout << "Ini adalah pesan dengan \x01 menggunakan karakter escape ASCII.\n";
    log.logMessage("  karakter soh ^A %c\n: " + controlA, GREEN);

    return 0;
}
