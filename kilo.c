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
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>


/*** defines ***/
// Whatever key goes with Ctrl gets ignored
#define CTRL_KEY(k) ((k) & 0x1f)
#define KILO_VERSION "1.0.0"
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE
enum editorKey {
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

/*** data ***/
typedef struct erow {
  int size;
  char *chars;
} erow;
struct editorConfig {
    // Cursor position
  int cx, cy;
  int screenrows;
  int screencols;
  int numrows;
  erow row;
  struct termios orig_termios;
};
struct editorConfig E;


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
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
  //tcgetattr(STDIN_FILENO, &E.orig_termios);
      // We go back to Canonical mode
  atexit(disableRawMode);
  struct termios raw = E.orig_termios;
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
int editoReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1': return HOME_KEY;
            case '3': return DEL_KEY;
            case '4': return END_KEY;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            case '7': return HOME_KEY;
            case '8': return END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
          case 'A': return ARROW_UP;
          case 'B': return ARROW_DOWN;
          case 'C': return ARROW_RIGHT;
          case 'D': return ARROW_LEFT;
          case 'H': return HOME_KEY;
          case 'F': return END_KEY;
        }
      }
    } else if (seq[0] == 'O') {
      switch (seq[1]) {
        case 'H': return HOME_KEY;
        case 'F': return END_KEY;
      }
    }
    return '\x1b';
  } else {
    return c;
  }
}

int getCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';

  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

  return 0;
}

int getWindowsSize(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    return getCursorPosition(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

/*** file i/o ***/
void editorOpen(char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp) die("fopen");
  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;
  linelen = getline(&line, &linecap, fp);
  if (linelen != -1) {
    while (linelen > 0 && (line[linelen - 1] == '\n' ||
                           line[linelen - 1] == '\r'))
      linelen--;
    E.row.size = linelen;
    E.row.chars = malloc(linelen + 1);
    memcpy(E.row.chars, line, linelen);
    E.row.chars[linelen] = '\0';
    E.numrows = 1;
  }
  free(line);
  fclose(fp);
}


/*** append buffer ***/
  // Instead of multiple write() calls which can flicker the screen
  // We prefer a unique big write() call.
struct abuf {
  char *b;
  int len;
};
#define ABUF_INIT {NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len) {
    // We ask realloc to give us a block of memory
    // Size = current string + string we are appending.
  char *new = realloc(ab->b, ab->len + len);
  if(new == NULL) return;
  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}
  // Deallocates the memory used,
void abFree(struct abuf *ab) {
  free(ab->b);
}

/*** input ***/
void editorMoveCursor(int key) {
  switch (key) {
    case ARROW_LEFT:
      if (E.cx != 0) {
        E.cx--;
      }
      break;
    case ARROW_RIGHT:
      if (E.cx != E.screencols - 1) {
        E.cx++;
      }
      break;
    case ARROW_UP:
      if (E.cy != 0) {
        E.cy--;
      }
      break;
    case ARROW_DOWN:
      if (E.cy != E.screenrows - 1) {
        E.cy++;
      }
      break;
  }
}
  // Waits for key press and handles it.
void editorProcessKeyPress() {
  int c = editoReadKey();
  switch(c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
    case HOME_KEY:
      E.cx =0;
      break;
    case END_KEY:
      E.cx = E.screencols -1;
      break;
    case PAGE_UP:
    case PAGE_DOWN:
        {
          int times = E.screenrows;
          while (times--)
            editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        }
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
      editorMoveCursor(c);
      break;
  }
}

/*** output ***/
  // we are going to draw tildes on the side, like Vim does.
  // For now, 24 rows
void editorDrawRows(struct abuf *ab) {
  int y;
  //for(y = 0; y <24; y++) {
  for (y = 0; y < E.screenrows; y++) {
    if (y <= E.numrows) {
      if (E.numrows == 0 && y == E.screenrows / 3) {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome),
          "Kilo editor by Pablo Mateo-- version %s", KILO_VERSION);
        if (welcomelen > E.screencols) welcomelen = E.screencols;
          // Center Welcome message
        int padding = (E.screencols - welcomelen) / 2;
        if (padding) {
          abAppend(ab, "~", 1);
          padding--;
        }
        while (padding--) abAppend(ab, " ", 1);
        abAppend(ab, welcome, welcomelen);
      } else {
        //write(STDOUT_FILENO, "~", 1);
        abAppend(ab, "~", 1);
      }
    } else {
      int len = E.row.size;
      if (len > E.screencols) len = E.screencols;
      abAppend(ab, E.row.chars, len);
    }
    abAppend(ab, "\x1b[K", 3);
    if (y < E.screenrows - 1) {
      //write(STDOUT_FILENO, "\r\nW, 2");
      abAppend(ab, "\r\n", 2);
    }
  }
}

void editorRefreshScreen() {
  struct abuf ab = ABUF_INIT;
    // Set mode for the cursor.
  abAppend(&ab, "\x1b[?25l", 6);
  abAppend(&ab, "\x1b[H", 3);
  editorDrawRows(&ab);
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
  abAppend(&ab, buf, strlen(buf));
  //abAppend(&ab, "\x1b[H", 3);
    // Reset mode for the cursor.
  abAppend(&ab, "\x1b[?25h", 6);
  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
    // We are writing 4 bytes out to the terminal
      // First => \x1b (escape character) (27 in decimal)
      // Second => [2J
      // Escape character + [ => escape sequence
        // J => Clear the screen
        // 2 => Full screen
          // <esc>[1J will clear the screen up to the cursor
          // <esc>[0J will clear the screen from the cursor to the end...
  //write(STDOUT_FILENO, "\x1b[2J", 4);
    // Reposition the cursor
      // H => Cursor position
  //write(STDOUT_FILENO, "\x1b[H", 3);
  //editorDrawRows();
  //write(STDOUT_FILENO, "\x1b[H", 3);
}


/*** init ***/
void initEditor() {
  E.cx = 0;
  E.cy = 0;
  E.numrows = 0;
  if(getWindowsSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

int main(int argc, char *argv[]) {
  enableRawMode();
  initEditor();
  if(argc >= 2) {
    editorOpen(argv[1]);
  }

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
