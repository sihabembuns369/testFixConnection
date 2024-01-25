#include <ncurses.h>
#include <menu.h>
#include <cstdlib> // Include this header for calloc

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

const char *choices[] = {
    "Option 1",
    "Option 2",
    "Option 3",
    "Quit",
};

int main()
{
    initscr();            // Initialize ncurses
    raw();                // Enable raw mode, disabling line buffering and character processing
    keypad(stdscr, TRUE); // Enable function key processing

    int n_choices = ARRAY_SIZE(choices);
    ITEM **my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));
    for (int i = 0; i < n_choices; ++i)
        my_items[i] = new_item(choices[i], "");

    my_items[n_choices] = (ITEM *)NULL;

    MENU *my_menu = new_menu((ITEM **)my_items);

    // Set menu options for a horizontal menu
    menu_opts_off(my_menu, O_SHOWDESC);
    set_menu_format(my_menu, 1, n_choices);

    post_menu(my_menu);
    refresh();

    int c;
    while ((c = getch()) != 'q')
    {
        switch (c)
        {
        case KEY_RIGHT:
            menu_driver(my_menu, REQ_RIGHT_ITEM);
            break;
        case KEY_LEFT:
            menu_driver(my_menu, REQ_LEFT_ITEM);
            break;
        case 10:        // 'Enter' key (newline character)
        case KEY_ENTER: // 'Enter' key
        {
            ITEM *current = current_item(my_menu);
            if (item_index(current) == n_choices - 1)
            {
                // Exit the loop if "Quit" is selected
                c = 'q';
            }
            else
            {
                // Print the selected option
                printw("Selected: %s\n", item_name(current));
                // Display a message
                printw("Option Selected\n");
                refresh();
                // Wait for any key to be pressed
                getch();
            }
        }
        break;
        }
    }

    unpost_menu(my_menu);
    free_menu(my_menu);
    for (int i = 0; i < n_choices; ++i)
        free_item(my_items[i]);
    endwin(); // End ncurses

    return 0;
}



  selectedOption = item_index(current_item(myMenu));