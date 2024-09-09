#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "editor.h"

// -- data --
editorconf editor;

// -- definitions --
void abAppend(abuf *ab, const char *s, int len) {
  char *new = realloc(ab->buf, ab->len + len);

  if (new == NULL)
    return;
  memcpy(&new[ab->len], s, len);
  ab->buf = new;
  ab->len += len;
}

void abFree(abuf *ab) { free(ab->buf); }

void die(const char *s) { // because func name:kill was already used
  editorClearScreen();
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &editor.org_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &editor.org_termios) == -1)
    die("tcsetattr");

  atexit(disableRawMode);

  struct termios raw = editor.org_termios;
  raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON | ICRNL);
  raw.c_lflag &= ~(ECHO | IEXTEN | ICANON | ISIG);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

void initEditor() {
  if (getWindowSize(&editor.screenRows, &editor.screenCols) == -1)
    die("invalid window size");
}

int getWindowSize(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
      return -1;
    return getCursorPosition(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

int getCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R')
      break;
    i++;
  }
  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[')
    return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
    return -1;
  return 0;
}

void editorClearScreen() {
  write(STDIN_FILENO, "\x1b[2J", 4);
  write(STDIN_FILENO, "\x1b[H", 3);
}

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      die("reading input failed");
  }
  return c;
}

void editorProcessKeyPress() {
  char c = editorReadKey();

  switch (c) {
  case CTRL_KEY('c'):
    editorClearScreen();
    exit(0);
    break;
  }
}

void addWelcomeMsg(abuf *ab) {
  char wlc[80];
  int wlclen =
      snprintf(wlc, sizeof(wlc), "therealtxt editor v %s", EDITOR_VERSION);
  if (wlclen > editor.screenCols)
    wlclen = editor.screenCols;
  
  int padding = (editor.screenCols - wlclen)/2;
  if(padding){
    abAppend(ab, "~", 1);
    padding--;
  }
  while(padding--){
    abAppend(ab, " ", 1);
  }
  abAppend(ab, wlc, wlclen);
}

void editorDrawRows(abuf *ab) {
  for (int y = 0; y < editor.screenRows ; y++) {
    if (y == editor.screenRows / 2)
      addWelcomeMsg(ab);
    else
      abAppend(ab, "~", 1);

    abAppend(ab, "\x1b[K", 3); // clear the line before drawing
    if (y < editor.screenRows - 1)
      abAppend(ab, "\r\n", 2);
  }
}

void editorRefreshScreen() {
  abuf ab = ABUF_INIT;

  abAppend(&ab, "\x1b[?25l", 6); // hide cursor
  abAppend(&ab, "\x1b[H", 3);    // move cursor to start before drawing

  editorDrawRows(&ab);

  abAppend(&ab, "\x1b[H", 3);    // move cursor to start at the end
  abAppend(&ab, "\x1b[?25h", 6); // show the cursor

  write(STDOUT_FILENO, ab.buf, ab.len);
  abFree(&ab);
}
