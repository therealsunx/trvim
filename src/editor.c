#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#endif

#include "editor.h"
#include "input.h"
#include "keybinds.h"
#include "settings.h"

// -- data --
editorconf editor;
settings_t settings = DEF_SETTINGS;
struct termios org_termios;

#ifdef _WIN32
HANDLE hStdin;
DWORD orgMode;

void disableRawMode(void) {
  if (!SetConsoleMode(hStdin, orgMode)) {
    die("SetConsoleMode");
  }
}

void enableRawMode(void) {
  hStdin = GetStdHandle(STD_INPUT_HANDLE);
  if (hStdin == INVALID_HANDLE_VALUE) {
    die("GetStdHandle");
  }

  if (!GetConsoleMode(hStdin, &orgMode)) {
    die("GetConsoleMode");
  }

  atexit(disableRawMode);

  DWORD rawMode = orgMode;
  rawMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
  
  if (!SetConsoleMode(hStdin, rawMode)) {
    die("SetConsoleMode");
  }
}
#else
void disableRawMode(void) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &org_termios) == -1)
    die("tcsetattr");
  clearTerminal();
}

void enableRawMode(void) {
  if (tcgetattr(STDIN_FILENO, &org_termios) == -1)
    die("tcsetattr");

  atexit(disableRawMode);

  struct termios raw = org_termios;
  raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON | ICRNL);
  raw.c_lflag &= ~(ECHO | IEXTEN | ICANON | ISIG);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}
#endif

void initEditor(void) {
  editor.mode = NORMAL;
  initWindow(&editor.window);
  initCmdBar(&editor.cmdbar);
  initStack(&editor.cmdstk);

  atexit(freeEditor);
}

void freeEditor(void) {
  freeStack(&editor.cmdstk);
  freeWindow(&editor.window);
}

void editorSetStatusMsg(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(editor.cmdbar.msg, sizeof(editor.cmdbar.msg), fmt, ap);
  va_end(ap);
  editor.cmdbar.msg_t = time(NULL);
}

void editorStatusBarUpdate(void) {
  if (editor.mode == NORMAL) return;
  editor.cmdbar.msg_t = time(NULL);
  if (editor.mode == INSERT)
    editorSetStatusMsg("-- INSERT --");
  else if (editor.mode == VISUAL)
    editorSetStatusMsg("-- VISUAL --");
  else if (editor.mode == VISUAL_LINE)
    editorSetStatusMsg("-- VISUAL_LINE --");
}

void editorRefreshScreen(void) {
  windowScrollCursor(&editor.window);
  hideCursor();
  clearTerminal();
  windowDrawViews(&editor.window, editor.mode);
  windowDrawCmdBar(&editor.window, &editor.cmdbar, &editor.cmdstk);
  windowShowCursor(&editor.window, editor.mode);
}

buffer_t *editorGetCurrentBuffer(void) {
  return windowGetCurBuffer(&editor.window);
}
view_t *editorGetCurrentView(void){
  return windowGetCurView(&editor.window);
}

void _editorSzUpdtcb(void) {
  if (_winUpdateCheck(&editor.window)) {
    _onSizeUpdate(&editor.window);
    editorRefreshScreen();
  }
}

int editorReadKey(void) { return readKey(_editorSzUpdtcb); }

void editorProcessKeyPress(void) {
  int c = editorReadKey();

  view_t *view = windowGetCurView(&editor.window);

  if (viewMasterKeys(view, c, editor.mode)) {
    viewUpdateSelection(view, editor.mode, 1);
    emptyStack(&editor.cmdstk);
    return;
  }

  if (editor.mode == INSERT) {
    if (viewBack2Normal(view, c, editor.mode)) editorSwitchMode(NORMAL);
    else viewInsertEdit(view, c);
    return;
  }

  push(&editor.cmdstk, c);
  parsedcmd_t pc = parseCommand(&editor.cmdstk);
  if(!pc.cmd) return;
  int cmd_st = ST_NOOP;
  if (!pc.repx) pc.repx++;

  if (editor.mode == VISUAL || editor.mode == VISUAL_LINE) {
    cmd_st = viewBack2Normal(view, c, editor.mode);
    if(cmd_st != ST_NOOP){
      viewUpdateSelection(view, editor.mode, 1);
      editorSwitchMode(NORMAL);
      emptyStack(&editor.cmdstk);
      return;
    }

    cmd_st = viewVisualOp(view, &pc, editor.mode);
    if(cmd_st != ST_NOOP) {
      if(cmd_st != ST_WAIT) emptyStack(&editor.cmdstk);
      viewUpdateSelection(view, editor.mode, 1);
      return;
    }
  }

  if (editor.mode == NORMAL) {
    cmd_st = windowOpCmdHandle(&editor.window, &pc);
    if(cmd_st != ST_NOOP){
      emptyStack(&editor.cmdstk);
      return;
    }

    cmd_st = viewInsCmdHandle(view, pc.cmd);
    if (cmd_st != ST_NOOP) {
      if(cmd_st == ST_SUCCESS) editorSwitchMode(INSERT);
      emptyStack(&editor.cmdstk);
      return;
    }
    cmd_st = viewVisCmdHandle(view, pc.cmd);
    if(cmd_st){
      emptyStack(&editor.cmdstk);
      editorSwitchMode(cmd_st);
      return;
    }
  }

  cmd_st = viewMvmtCmdHandle(view, pc.cmd, pc.repx, editor.mode);
  if(cmd_st != ST_NOOP){
    viewUpdateSelection(view, editor.mode, 1);
    emptyStack(&editor.cmdstk);
    return;
  }
  cmd_st = viewMiscCmdHandle(view, &pc);

  if (cmd_st != ST_WAIT) {
    viewUpdateSelection(view, editor.mode, 1);
    emptyStack(&editor.cmdstk);
    return;
  }
}

int editorEditFileCommand(char *cmd){
  int _cl = strlen(cmd);
  if(_cl <= 2) return 0;
  if(cmd[0] != 'e' || cmd[1] != ' ') return 0;

  char *filename = &cmd[2];
  editorOpen(filename);
  return 1;
}
int editorAddNewView(char *cmd){
  int _cl = strlen(cmd);
  if(_cl<2) return 0;
  if(cmd[1] != 's') return 0;
  if(cmd[0] == 'v'){
    windowAddView(&editor.window);
    return 1;
  }
  return 0;
}

void editorCmdPromptProc(char *prompt) {
  char *cmd = editorPrompt(prompt ? prompt : "Enter command:", NULL);
  if (!cmd) return;

  int _cl = strlen(cmd);
  int _cmdtype = CMD_NONE;
  int val = 0, i = 0;

  while (i < _cl) {
    if (cmd[i] >= '0' && cmd[i] <= '9') {
      _cmdtype = CMD_NUM;
      val = val * 10 + cmd[i] - '0';
    } else break;
    i++;
  }

  if (_cmdtype == CMD_NUM) {
    viewMvmtCmdHandle(windowGetCurView(&editor.window), 'J', val, NORMAL);
    return;
  }
  if (editor.mode == VISUAL_LINE || editor.mode == VISUAL) {
    editorSetStatusMsg("\x1b[91m Not an command during vis. mode: %s\x1b[0m", cmd);
    return;
  }

  if(editorEditFileCommand(cmd)) return;
  if(editorAddNewView(cmd)) return;

  while (i < _cl) {
    if (cmd[i] == 'w') {
      if (_cmdtype & (CMD_WRITE | CMD_QUIT | CMD_ALL | CMD_FORCE)) {
        _cmdtype |= CMD_ERROR;
        break;
      }
      _cmdtype |= CMD_WRITE;
    } else if (cmd[i] == 'q') {
      if (_cmdtype & (CMD_QUIT | CMD_ALL | CMD_FORCE)) {
        _cmdtype |= CMD_ERROR;
        break;
      }
      _cmdtype |= CMD_QUIT;
    } else if (cmd[i] == 'a') {
      if (_cmdtype & (CMD_ALL | CMD_FORCE)) {
        _cmdtype |= CMD_ERROR;
        break;
      }
      _cmdtype |= CMD_ALL;
    } else if (cmd[i] == '!') {
      if (_cmdtype & CMD_FORCE) {
        _cmdtype |= CMD_ERROR;
        break;
      }
      _cmdtype |= CMD_FORCE;
    } else {
      _cmdtype |= CMD_ERROR;
      break;
    }
    i++;
  }
  if (_cmdtype&CMD_ERROR) {
    editorSetStatusMsg("\x1b[91m Not an editor Command: %s\x1b[0m", cmd);
    return;
  }
  editorSaveQuitBuffers(_cmdtype);
}

void editorSwitchMode(int mode) {
  if (mode == NORMAL) editor.cmdbar.msg_t = 0;
  editor.mode = mode;
}
void editorOpen(char *filename) {
  windowOpenFile(&editor.window, filename);
}

void editorSaveBuffer(buffer_t *buf) {
  if (buf->filename == NULL) {
    editorStatusBarUpdate();
    if ((buf->filename = editorPrompt("Save as: %s", NULL)) == NULL) {
      editorSetStatusMsg("Save Aborted...");
      return;
    }
  }
  int _sz = bufferSave(buf);
  if (_sz == -1)
    editorSetStatusMsg("Save Failed I/O error: %s", strerror(errno));
  else
    editorSetStatusMsg("%d bytes written to disk", _sz);
}

void editorSaveQuitBuffers(int flags){
  if(flags&CMD_WRITE){
    windowSaveBuffers(&editor.window, flags&CMD_ALL);
  }
  if(flags&CMD_QUIT){
    if (windowDirtyBufCheck(&editor.window) && !(flags&CMD_FORCE)) {
      editorSetStatusMsg( "\x1b[91m Unsaved Changes. Use q! to force quit buffer_t. \x1b[0m");
      return;
    }
    windowQuitBuffers(&editor.window, flags&CMD_ALL);
  }
  if(windowShouldClose(&editor.window)) exit(0);
}

char *editorPrompt(char *prompt, void (*callback)(char *, int)) {
  size_t bufsize = 128;
  char *buf = malloc(bufsize);
  size_t buflen = 0;
  buf[buflen] = '\0';

  emptyStack(&editor.cmdstk);
  while (1) {
    editorSetStatusMsg(prompt, buf);
    editorRefreshScreen();
    // NOTE : if I ever implement cmdbar as buffer, then look at this
    showCursor(editor.window.size.y - CMDBAR_SZ, strlen(editor.cmdbar.msg));

    int c = editorReadKey();

    if (c == ESCAPE) {
      editorSetStatusMsg("");
      if (callback)
        callback(buf, c);
      free(buf);
      return NULL;
    } else if (c == BACKSPACE || c == CTRL_H || c == DEL_KEY) {
      if (buflen != 0)
        buf[--buflen] = '\0';
    } else if (c == RETURN) {
      editorSetStatusMsg("");
      if (callback)
        callback(buf, c);
      return buf;
    } else if (!iscntrl(c) && c < 128) {
      if (buflen == bufsize - 1) {
        bufsize *= 2;
        buf = realloc(buf, bufsize);
      }
      buf[buflen++] = c;
      buf[buflen] = '\0';
    }
    if (callback)
      callback(buf, c);
  }
}

void editorFindCallback(char *query, int key) {
  static int _st_hlind;
  static unsigned char *_st_hl = NULL;

  view_t *view = editorGetCurrentView();
  buffer_t *_cbuf = windowGetBufOfView(&editor.window, view);

  if (_st_hl) {
    memcpy(_cbuf->rows[_st_hlind].hlchars, _st_hl,
           _cbuf->rows[_st_hlind].rsize);
    free(_st_hl);
    _st_hl = NULL;
  }

  if (key == RETURN || key == ESCAPE)
    return;
  if (query == NULL)
    return;
  int dir = 0;
  if (key == ARROW_UP)
    dir = -1;
  else if (key == ARROW_DOWN)
    dir = 1;

  if (bufferFind(_cbuf, query, &view->cursor, dir)) {
    // save state for reverting later
    _st_hlind = view->cursor.y;
    erow *row = &_cbuf->rows[_st_hlind];
    _st_hl = malloc(row->rsize);
    memcpy(_st_hl, row->hlchars, row->rsize);

    // highlight found query result
    memset(&row->hlchars[rowCursorToRenderX(row, view->cursor.x)], TK_MATCH,
           strlen(query));
  }
}

void editorFind(char *prompt) {
  view_t *view = editorGetCurrentView();
  vec2 _st_cur = view->cursor;
  vec2 _st_off = view->offset;

  char *query = editorPrompt(prompt ? prompt : "Search: %s (Esc to cancel)",
                             editorFindCallback);
  if (!query) {
    view->cursor = _st_cur;
    view->offset = _st_off;
  }
}

