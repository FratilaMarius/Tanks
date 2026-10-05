#include <stdlib.h>
#include <ncurses.h>

int main() {  
  initscr();            // Initialize the ncurses screen
  printw("Hello, World!"); // Print "Hello, World!" to the screen
  refresh();            // Refresh the screen to show the output
  getch();              // Wait for user input
  endwin();             // End ncurses mode

}