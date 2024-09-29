#include <termios.h>

#include "buffer.h"
#include "cmdstack.h"
#include "window.h"
#include "cmdbar.h"

typedef struct {
  window_t window;
  cmdbar_t cmdbar;
  cmdstack_t cmdstk;
  int mode;
} editorconf;

// --editor init functions --
void disableRawMode(void);
void enableRawMode(void);
void initEditor(void);
void freeEditor(void);

void editorSetStatusMsg(const char *fmt, ...);
void editorStatusBarUpdate(void);
void editorRefreshScreen(void);
int editorReadKey(void);
buffer_t *editorGetCurrentBuffer(void);
view_t *editorGetCurrentView(void);
void editorProcessKeyPress(void);
void editorNormalModeKeyProc(int key);
void editorInsertModeKeyProc(int key);
void editorProcessCommand(void);

void editorCmdPromptProc(char *prompt);
int editorEditFileCommand(char *cmd);
int editorAddNewView(char *cmd);

void editorSaveQuitBuffers(int flags);
void editorSwitchMode(int mode);
void editorSaveBuffer(buffer_t *buf);
void editorOpen(char* filename);
char *editorPrompt(char *query, void (*callback)(char*, int));
void editorFind(char *prompt);

