#include <stdio.h>
#include <termios.h>
#include "keybinds.h"

#define ABUF_INIT {NULL, 0}
#define EDITOR_VERSION "0.0.1"

typedef struct {
  int size;
  char *chars;
} erow;

typedef struct {
  int cursor_x, cursor_y, st_cx;
  int screen_rows, screen_cols;
  int numrows, offsety, offsetx;
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
void editorAppendRows(char *s, size_t len);
void editorOpen(char* filename);
int editorReadKey();
void editorScroll();