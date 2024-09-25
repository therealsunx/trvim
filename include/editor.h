#include <termios.h>
#include <time.h>

#include "abuf.h"
#include "buffer.h"
#include "cmdstack.h"

typedef struct {
  buffer buf;
  vec2 screen_size;
  char statusmsg[64];
  time_t statusmsg_time;
  int mode;
  cmdstack cmdstk;
} editorconf;

// --editor init functions --
void disableRawMode(void);
void enableRawMode(void);
void initEditor(void);
void freeEditor(void);

// -- editor prcs --
int editorCheckSizeUpdate(void);
void editorDrawBuffers(abuf *appendbuffer);
void editorShowCursor(void);
void editorDrawStsMsgBar(abuf *ab);
void editorSetStatusMsg(const char *fmt, ...);
void editorStatusBarUpdate(void);
buffer *editorGetCurrentBuffer(void);

void editorRefreshScreen(void);
int editorReadKey(void);
void editorProcessKeyPress(void);
void editorNormalModeKeyProc(int key);
void editorInsertModeKeyProc(int key);
void editorSwitchMode(int mode);
void editorUpdateSelection(int flags);

void editorPageScroll(int key);
void editorScroll(void);

void editorOpen(char* filename);

void editorProcessCommand(void);
void editorCmdPromptProc(char *prompt);
void editorSaveQuitBuffers(int flags);

// -- editor operations --
void editorSaveBuffer(buffer *buf);
char *editorPrompt(char *query, void (*callback)(char*, int));

void editorFind(char *prompt);

