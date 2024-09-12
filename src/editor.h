#include <stdio.h>
#include <termios.h>
#include <time.h>
#include "keybinds.h"

#define ABUF_INIT {NULL, 0}
#define EDITOR_VERSION "0.0.1"

typedef struct {
  int size, rsize;
  char *chars, *renderch;
} erow;

typedef struct {
  int cursor_x, cursor_y, st_cx, render_x;
  int screen_rows, screen_cols;
  int numrows, offset_y, offset_x;
  char* filename;
  char statusmsg[64];
  time_t statusmsg_time;
  erow *row;
  struct termios org_termios;
} editorconf ;

typedef struct {
  char *buf;
  int len;
} abuf;

// -- append Buffer methods --
void abAppend(abuf *appendbuffer, const char *string, int length);
void abFree(abuf *appendbuffer);

// --editor init functions --
void die(const char *message);
void disableRawMode();
void enableRawMode();
void initEditor();

// -- utils: getters --
int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *x, int *y);

// -- editor prcs --
void editorClearScreen();
void editorDrawRows(abuf *appendbuffer);
void editorRefreshScreen();
void editorProcessKeyPress();
void editorMoveCursor(int charKey);
void editorUpdateRow(erow *row);
void editorAppendRows(char *s, size_t len);
void editorOpen(char* filename);
int editorReadKey();
void editorScroll();
int editorCxtoRx(erow *row, int cursor_x);
void editorDrawStatusBar(abuf *buf);
void editorSetStatusMsg(const char *fmt, ...);
void editorDrawStsMsgBar(abuf *ab);
