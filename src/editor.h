#include <stdio.h>
#include <termios.h>
#include <time.h>

#include "keybinds.h"
#include "abuf.h"
#include "buffer.h"

typedef struct {
  buffer buf;
  vec2 screen_size;
  struct termios org_termios;
  char statusmsg[64];
  time_t statusmsg_time;
} editorconf;

// --editor init functions --
void disableRawMode();
void enableRawMode();
void initEditor();

// -- editor prcs --
void editorUpdateSize();
void editorDrawBuffers(abuf *appendbuffer);
void editorShowCursor(abuf *ab);
void editorDrawStsMsgBar(abuf *ab);
void editorSetStatusMsg(const char *fmt, ...);

void editorRefreshScreen();
void editorProcessKeyPress();
void editorMoveCursor(int charKey);
void editorScroll();
void editorInsertRow(int index, char *s, size_t len);
void editorDeleteRow(int index);
void editorOpen(char* filename);

// -- editor operations --
void editorInsertChar(int ch);
void editorInsertNewLine();
void editorDelChar(int dir);
void editorSaveBuffer();
char *editorPrompt(char *query, void (*callback)(char*, int));

void editorFind();
