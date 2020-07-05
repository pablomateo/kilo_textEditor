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
#include <errno.h>

/*** defines ***/
// Whatever key goes with Ctrl gets ignored
#define CTRL_KEY(k) ((k) & 0x1f)


/*** data ***/
struct termios orig_termios;

/*** terminal ***/
  // We will call this functioon when we have an error, to close the program.
void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

void disableRawMode() {
    // If it returns -1 (error) then we call die()
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  tcgetattr(STDIN_FILENO, &orig_termios);
      // We go back to Canonical mode
  atexit(disableRawMode);
  struct termios raw = orig_termios;
      // This code disables canonical mode
      // ICRNL disables Ctrl + m
      // IXON = Input flag -> Ctrl + s & Ctrl + q
      // This also disables this combination
        // Ther rest are other flags less common
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
      // POST = Post Processing Output
      // Takes newline from terminal -> We need to modify our print statetments if we want a new line
  raw.c_oflag &= ~(OPOST);
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
      // ISIG disables signals SIGINT (Ctrl + c) & SIGTSTP (Ctrl + z)
          // So we disable them both too
      // IEXTEN disables Ctrl + v & Ctrl + o
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cflag |= (CS8);

  // We have to create a timeout for read()
    // These are cc (Control characters)
      // VMIN = Minimun number of bytes of inpùt needed before read()
      // VTIME = Maximum amount of time to wait before read returns
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;


      // Method to apply them to Terminal
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

  // Waits for keypress and returns it
char editoReadKey() {
  int nread;
  char c;
  while((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if(nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

/*** input ***/
  // Waits for key press and handles it.
void editorProcessKeyPress() {
  char c = editoReadKey();
  switch(c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
  }
}

/*** output ***/
// we are going to draw tildes on the side, like Vim does.
void editorDrawRows() {
int y;
for(y = 0; y <24; y++) {
  write(STDOUT_FILENO, "~\r\n", 3);
}
}

void editorRefreshScreen() {
    // We are writing 4 bytes out to the terminal
      // First => \x1b (escape character) (27 in decimal)
      // Second => [2J
      // Escape character + [ => escape sequence
        // J => Clear the screen
        // 2 => Full screen
          // <esc>[1J will clear the screen up to the cursor
          // <esc>[0J will clear the screen from the cursor to the end...
  write(STDOUT_FILENO, "\x1b[2J", 4);
    // Reposition the cursor
      // H => Cursor position
  write(STDOUT_FILENO, "\x1b[H", 3);
  editorDrawRows();
  write(STDOUT_FILENO, "\x1b[H", 3);
}


/*** init ***/
int main() {
  enableRawMode();

  while(1) {
    editorRefreshScreen();
    editorProcessKeyPress();
  }
  return 0;

/*
  while (1) {
    // We create a character
    char c = '\0';
      // And ask to read 1 byte until there are no more to read
          // To make it stop -> Ctrl + D
    //while (read(STDIN_FILENO, &c, 1) == 1);
          //The same as above, but will stop to read when we put a q and press Enter.
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
        // iscntrl() tests whether a character is a control character. +
        // Control characters are nonprintable characters that we don’t want to print to the screen
    if (iscntrl(c)) {
        // printf() can print multiple representations of a byte
        // %d tells it to format the byte as a decimal number (its ASCII code)
        // We add the carriage reurn because of the OPOST that nulls it
        // We add \r
      printf("%d\r\n", c);
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
        printf("%d ('%c')\r\n", c, c);
    }
    if (c == CTRL_KEY('q')) break;
  }
  return 0;
*/

}
