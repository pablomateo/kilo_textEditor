/* Kilo Text Editor - By Pablo Mateo */

    // We need to include it because read() & STDIN_FILENO come from it
#include <unistd.h>
    // We need to include it because other methods come from it.
#include <termios.h>

void enableRawMode() {
  struct termios raw;
  tcgetattr(STDIN_FILENO, &raw);
      // Canonical mode = you see in terminal what you ype.
      // Raw mode = You don't see it
      // So, in this line, we turn off echoing
        // If after closing the program it doen't go back to canonical mode, type: reset (ENTER)
  raw.c_lflag &= ~(ECHO);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
  enableRawMode();
      // We create a character
  char c;
        // And ask to read 1 byte until there are no more to read
        // To make it stop -> Ctrl + D
  //while (read(STDIN_FILENO, &c, 1) == 1);
      //The same as above, but will stop to read when we put a q and press Enter.
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');

  return 0;
}
