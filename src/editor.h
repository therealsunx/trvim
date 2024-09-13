#include <stdio.h>
#include <termios.h>
#include <time.h>

#include "keybinds.h"
#include "row.h"
#include "abuf.h"

#define EDITOR_VERSION "0.0.1"

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

// --editor init functions --
void disableRawMode();
void enableRawMode();
void initEditor();
void addWelcomeMsg(abuf *ab);

// -- editor prcs --
void editorDrawRows(abuf *appendbuffer);
void editorDrawStatusBar(abuf *buf);
void editorDrawStsMsgBar(abuf *ab);
void editorSetStatusMsg(const char *fmt, ...);
void editorRefreshScreen();
void editorProcessKeyPress();
void editorMoveCursor(int charKey);
void editorScroll();
void editorAppendRows(char *s, size_t len);
void editorOpen(char* filename);

// -- editor operations --
void editorInsertChar(int ch);
void editorSaveBuffer();
char *editorRowtoStr(int *buflen);
