#include <termios.h>
#include "keybinds.h"

#define ABUF_INIT {NULL, 0}
#define EDITOR_VERSION "0.0.1"

typedef struct {
  int size;
  char *chars;
} erow;

typedef struct {
  int cursor_x, cursor_y;
  int screen_rows, screen_cols;
  erow row;
  int numrows;
  struct termios org_termios;
} editorconf ;

typedef struct {
  char *buf;
  int len;
} abuf;

// -- append Buffer methods --
void abAppend(abuf *, const char *, int);
void abFree(abuf *);

// --editor init functions --
void die(const char *s);
void disableRawMode();
void enableRawMode();
void initEditor();

// -- utils: getters --
int getWindowSize(int *, int *);
int getCursorPosition(int *, int *);

// -- editor prcs --
void editorClearScreen();
void editorDrawRows(abuf *);
void editorRefreshScreen();
int editorReadKey();
void editorProcessKeyPress();
void editorMoveCursor(int);
