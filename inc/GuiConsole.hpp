#ifndef MENUCLASS_HPP
#define MENUCLASS_HPP

#include <ncurses.h>
#include <menu.h>
#include <cstdlib> // Include this header for calloc

#include <iostream>

class GuiConsole
{
public:
    GuiConsole(const char *menuChoices[], int choicesCount);
    ~GuiConsole();

    int displayMenu();
    int GuiInit();

private:
    ITEM **myItems;
    MENU *myMenu;
    int choicesCount;

    WINDOW *win;
};

GuiConsole::GuiConsole(const char *menuChoices[], int choicesCount) : myItems(nullptr), myMenu(nullptr), choicesCount(choicesCount)
{
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    GuiInit();

    myItems = (ITEM **)calloc(choicesCount + 1, sizeof(ITEM *));
    for (int i = 0; i < choicesCount; ++i)
        myItems[i] = new_item(menuChoices[i], "");
    myItems[choicesCount] = (ITEM *)NULL;
    myMenu = new_menu((ITEM **)myItems);
    menu_opts_off(myMenu, O_SHOWDESC);
    set_menu_format(myMenu, 1, choicesCount);

    // set_menu_win(myMenu, win);
    set_menu_sub(myMenu, derwin(win, 20, 10, 4, 1));
    post_menu(myMenu);
    mvwprintw(win, 10, 1, "Pilih opsi menggunakan tombol panah atas dan bawah, lalu tekan 'Enter'.");
    refresh();
}

GuiConsole::~GuiConsole() // destructor
{
    unpost_menu(myMenu);
    free_menu(myMenu);
    for (int i = 0; i < choicesCount; ++i)
        free_item(myItems[i]);
    endwin();
}

int GuiConsole::displayMenu()
{
    int c;
    ITEM *selectedOption;
    int retrn = -1; // Inisialisasi retrn di luar loop
    const char *hi;
    while ((c = getch()) != 'q')
    {
        switch (c)
        {
        case KEY_RIGHT:
            menu_driver(myMenu, REQ_RIGHT_ITEM);
            break;
        case KEY_LEFT:
            menu_driver(myMenu, REQ_LEFT_ITEM);
            break;
        case 10:        // Tombol 'Enter'
        case KEY_ENTER: // Tombol 'Enter'
            selectedOption = current_item(myMenu);
            if (item_index(selectedOption) == choicesCount - 1)
            {
                // Keluar dari loop jika "Keluar" dipilih
                c = 'q';
                break;
            }
            else
            {
                // Menyimpan indeks yang dipilih
                retrn = item_index(selectedOption);
                return retrn;
            }
            break;
        }
    }
}

int GuiConsole::GuiInit()
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    win = newwin(10, 30, (rows - 10) / 2, (cols - 30) / 2);
    box(win, 0, 0);
    // refresh();
}
#endif
