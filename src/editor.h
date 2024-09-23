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
int editorCheckSizeUpdate();
void editorDrawBuffers(abuf *appendbuffer);
void editorShowCursor(abuf *ab);
void editorDrawStsMsgBar(abuf *ab);
void editorSetStatusMsg(const char *fmt, ...);
void editorStatusBarUpdate();
buffer *editorGetCurrentBuffer();

void editorRefreshScreen();
int editorReadKey();
void editorProcessKeyPress();
void editorNormalModeKeyProc(int key);
void editorInsertModeKeyProc(int key);
void editorSwitchMode(int mode);
void editorVisualModeKeyProc(int key);

void editorPageScroll(int key);
void editorScroll();

void editorOpen(char* filename);

void editorProcessCommand();

// -- editor operations --
void editorSaveBuffer();
char *editorPrompt(char *query, void (*callback)(char*, int));

void editorFind(char *prompt);

