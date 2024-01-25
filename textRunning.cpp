#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
void clearConsole()
{
    // ANSI escape code untuk membersihkan layar terminal
    std::cout << "\033[2J\033[H";
}

void displayRunningText(std::string text)
{
    while (true)
    {
        int consoleWidth = 100; // Set to your desired console width

        // Calculate the number of spaces needed to center the text
        int spaces = (consoleWidth - text.length()) / 2;
        // Tampilkan teks di console
        std::cout << std::setw(spaces + text.length()) << text << " " << std::flush; // Add a space to clear any remaining characters

        // Geser teks ke kiri
        char temp = text[0];
        std::string newText = text.substr(1) + temp;

        // Tunggu sejenak (dalam milidetik)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Kembali ke awal baris dengan carriage return
        std::cout << "\r" << std::flush;

        text = newText;
    }
}

int main()
{
    // Teks yang akan bergerak
    std::string runningText = "Hello, Running Text! ";

    // Membuat thread untuk menampilkan running text

    std::cout << "\n";
    std::thread textThread(displayRunningText, runningText);

    // Menunggu hingga pengguna menekan Enter untuk mengakhiri program
    std::cin.get();

    // Memberi tahu thread untuk berhenti
    textThread.detach();

    return 0;
}
