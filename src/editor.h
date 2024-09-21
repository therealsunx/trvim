#include <stdio.h>
#include <termios.h>
#include <time.h>

#include "keybinds.h"
#include "abuf.h"
#include "buffer.h"
#include "stack.h"

typedef struct {
  buffer buf;
  vec2 screen_size;
  struct termios org_termios;
  char statusmsg[64];
  time_t statusmsg_time;
  int mode;
  cmdstack cmdstk;
} editorconf;

// --editor init functions --
void disableRawMode();
void enableRawMode();
void initEditor();
void freeEditor();

// -- editor prcs --
void editorUpdateSize();
void editorDrawBuffers(abuf *appendbuffer);
void editorShowCursor(abuf *ab);
void editorDrawStsMsgBar(abuf *ab);
void editorSetStatusMsg(const char *fmt, ...);
void editorStatusBarUpdate();

void editorRefreshScreen();
void editorProcessKeyPress();
void editorNormalModeKeyProc(int key);
void editorInsertModeKeyProc(int key);
void editorSwitchMode(int mode);
void editorVisualModeKeyProc(int key);

void editorPageScroll(int key);
void editorScroll();

void editorMoveCursor(int charKey, int repx);
void editorGotoNextWord(int flags, int repx);
void editorFindChar(char char_, int dirflag, int repx);
void editorParaNav(int dirflag, int repx);
void editorGotoEnd(int posflg, int repx);
void editorReplaceChar(char char_, int repx);

void editorInsertRow(int index, char *s, size_t len);
void editorDeleteRow(int index);
void editorOpen(char* filename);

void editorProcessCommand();

// -- editor operations --
void editorInsertChar(int ch);
void editorInsertNewLine();
void editorDelChar(int dir);
void editorSaveBuffer();
char *editorPrompt(char *query, void (*callback)(char*, int));

void editorFind(char *prompt);

