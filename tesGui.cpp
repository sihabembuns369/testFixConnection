
#include "inc/GuiConsole.hpp"
#include <iostream>
int main()
{
    const char *menuChoices[] = {"Logon", "Logout", "Setting", "Exit", ""};
    int choicesCount = sizeof(menuChoices) / sizeof(menuChoices[0]);

    GuiConsole menu(menuChoices, choicesCount);
    //  printw("%s", std::to_string(selectedOption).c_str());

    int ch;
    do
    {
        int selectedOption = menu.displayMenu();
        printw("Opsi Terpilih\n");

        printw("%s", std::to_string(selectedOption).c_str());

    } while ((ch = getch()) != 'q');

    endwin(); // Memanggil refresh() di akhir loop dan sebelum mengecek nilai c

    // if (selectedOption != -1)
    // {
    //     // Do something with the selected option
    //     // For now, just print the selected option
    //     mvprintw(10, 0, "Selected Option: %s", choices[selectedOption]);
    //     refresh();
    //     getch(); // Wait for any key to be pressed
    // }

    return 0;
}