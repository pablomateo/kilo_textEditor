/* Kilo Text Editor - By Pablo Mateo */

    // We need to include it because read() & STDIN_FILENO come from it
#include <unistd.h>
    // We need to include it because other methods come from it.
#include <termios.h>
    // Standard Library
#include <stdlib.h>
    //Used to show what we are typing
#include <ctype.h>
#include <stdio.h>

struct termios orig_termios;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
      // We go back to Canonical mode
  atexit(disableRawMode);
  struct termios raw = orig_termios;
      // This code disables canonical mode
      // ISIG disables signals SIGINT (Ctrl + c) & SIGTSTP (Ctrl + z)
          // So we disable them both too
  raw.c_lflag &= ~(ECHO | ICANON);
      // Method to read attributes into termios
  // ----------
  //tcgetattr(STDIN_FILENO, &raw);
      // Canonical mode = you see in terminal what you ype.
      // Raw mode = You don't see it
      // So, in this line, we turn off echoing
        // If after closing the program it doen't go back to canonical mode, type: reset (ENTER)
            // But we reset Terminal to its former state with method disableRawMode()
      // ECHO es a bitflag ===================================>    00000000000000000000000000001000
      // We use the bitwise NOT operator (~) to turn it into =>    11111111111111111111111111110111
  raw.c_lflag &= ~(ECHO);
      // Method to apply them to Terminal
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
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
        // iscntrl() tests whether a character is a control character. +
        // Control characters are nonprintable characters that we don’t want to print to the screen
    if(iscntrl(c)) {
        // printf() can print multiple representations of a byte
        // %d tells it to format the byte as a decimal number (its ASCII code)
      printf("%d\n", c);
            // We can see how each key is represented
            // $: 97 ('a')
            // $: 98 ('b')
            // $: 99 ('c')
            // $: 100 ('d')
                // Special characters have other representations
                // Ctrl + a = 1
                // Ctrl + b = 2
    } else {
        // %c tells it to write out the byte directly, as a character.
      printf("%d ('%c')\n", c, c);
    }
  }
  return 0;
}
