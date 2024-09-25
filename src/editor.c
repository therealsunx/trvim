#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "editor.h"
#include "input.h"
#include "settings.h"

// -- data --
editorconf editor;
extern settingsType settings;

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &editor.org_termios) == -1)
    die("tcsetattr");
  clearTerminal();
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &editor.org_termios) == -1)
    die("tcsetattr");

  atexit(disableRawMode);

  struct termios raw = editor.org_termios;
  raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON | ICRNL);
  raw.c_lflag &= ~(ECHO | IEXTEN | ICANON | ISIG);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

void initEditor() {
  initBuffer(&editor.buf);
  editor.statusmsg[0] = '\0';
  editor.statusmsg_time = 0;
  editor.mode = NORMAL;
  initStack(&editor.cmdstk);

  editorCheckSizeUpdate();
  atexit(freeEditor);
}

void freeEditor() {
  freeStack(&editor.cmdstk);
  freeBuffer(&editor.buf);
}

int editorCheckSizeUpdate() {
  vec2 _sz;
  if (getWindowSize(&_sz.y, &_sz.x) == -1)
    die("invalid window size");

  if (_sz.x == editor.screen_size.x && _sz.y == editor.screen_size.y)
    return 0;
  editor.screen_size.x = _sz.x;
  editor.screen_size.y = _sz.y;
  bufferUpdateSize(&editor.buf, editor.screen_size.x, editor.screen_size.y - 2);
  return 1;
}

void editorDrawBuffers(abuf *ab) {
  int flags = 0; // TODO: change after implementing multiple buffers
  if(editor.mode == VISUAL || editor.mode == VISUAL_LINE) flags = editor.mode;
  bufferDrawRows(&editor.buf, ab, flags);
  bufferDrawStatusBar(&editor.buf, ab);
}

void editorShowCursor() {
  buffer *buf = &editor.buf;
  bufferShowCursor(buf); 
}

void editorDrawStsMsgBar(abuf *ab) {
  char cmdsts[editor.cmdstk.top];
  for (int i = 0; i < editor.cmdstk.top; i++)
    cmdsts[i] = editor.cmdstk.vals[i];

  int psize = editor.screen_size.x - editor.cmdstk.top;

  abAppend(ab, "\x1b[K", 3);
  if (time(NULL) - editor.statusmsg_time < 5) {
    int _slen = strlen(editor.statusmsg);
    int mlen = clamp(_slen, 0, psize);
    if (mlen)
      abAppend(ab, &editor.statusmsg[_slen - mlen], mlen);
    psize -= mlen;
  }

  for (; psize > 0; psize--)
    abAppend(ab, " ", 1);

  abAppend(ab, cmdsts, editor.cmdstk.top);
}

void editorSetStatusMsg(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(editor.statusmsg, sizeof(editor.statusmsg), fmt, ap);
  va_end(ap);
  editor.statusmsg_time = time(NULL);
}

void editorStatusBarUpdate() {
  if (editor.mode == NORMAL)
    return;
  editor.statusmsg_time = time(NULL);
  if (editor.mode == INSERT)
    editorSetStatusMsg("-- INSERT --");
  else if (editor.mode == VISUAL)
    editorSetStatusMsg("-- VISUAL --");
  else if (editor.mode == VISUAL_LINE)
    editorSetStatusMsg("-- VISUAL_LINE --");
}

void editorRefreshScreen() {
  editorStatusBarUpdate();
  editorScroll();
  abuf ab = ABUF_INIT;

  abAppend(&ab, "\x1b[?25l", 6); // hide cursor
  abAppend(&ab, "\x1b[H", 3);    // move cursor to start before drawing

  if (editor.mode != INSERT)
    abAppend(&ab, "\x1b[2 q", 5);
  else
    abAppend(&ab, "\x1b[6 q", 5);

  editorDrawBuffers(&ab);
  editorDrawStsMsgBar(&ab);
  write(STDOUT_FILENO, ab.buf, ab.len);
  abFree(&ab);
  editorShowCursor();

}

buffer *editorGetCurrentBuffer() {
  // TODO : get buffer from list of active buffers
  // when multiple buffers are supported
  return &editor.buf;
}

void _editorSzUpdtcb() {
  if (editorCheckSizeUpdate())
    editorRefreshScreen();
}

int editorReadKey() { return readKey(_editorSzUpdtcb); }

void editorProcessKeyPress() {
  int c = editorReadKey();

  buffer *buf = editorGetCurrentBuffer();

  switch (c) {
    case HOME_KEY:
      bufferGotoEnd(buf, editor.mode, JMP_BACK);
      break;
    case END_KEY:
      bufferGotoEnd(buf, editor.mode, 0);
      break;

    case PAGE_UP:
    case PAGE_DOWN:
      bufferPageScroll(buf, c);
      break;

    default:
      if (editor.mode == INSERT) editorInsertModeKeyProc(c);
      else editorNormalModeKeyProc(c);
  }
  if(editor.mode == VISUAL_LINE || editor.mode == VISUAL)
    bufferUpdateSelection(buf, editor.mode, 1);
}

void editorNormalModeKeyProc(int c) {
  static int qcount = 2;
  switch (c) {
    case CTRL_Z:
      if (editor.buf.dirty && --qcount) {
        editorSetStatusMsg( "WARNING! unchanged changes. Press CTRL-Z again to force quit");
        return;
      }
      exit(0);
      break;

    default:
      push(&editor.cmdstk, c);
  }
  editorProcessCommand();
  qcount = 2;
}

void editorProcessCommand() {
  parsedcmd_t pc = parseCommand(&editor.cmdstk);
  if (!pc.repx)
    pc.repx++;
  // take action

  buffer *buf = editorGetCurrentBuffer();

  if(editor.mode == NORMAL){ // only specific to normal mode
    int success = 1;
    switch (pc.cmd) {
      case 'i':
        editorSwitchMode(INSERT);
        break;
      case 'I':
        buf->cursor.x = firstCharIndex(buf->rows[buf->cursor.y].chars);
        buf->st.cursx = buf->cursor.x;
        editorSwitchMode(INSERT);
        break;
      case 'a':
        editorSwitchMode(INSERT);
        bufferMoveCursor(buf, ARROW_RIGHT, editor.mode, 1);
        break;
      case 'A':
        editorSwitchMode(INSERT);
        bufferGotoEnd(buf, editor.mode, 0);
        break;
      case 'o':
        editorSwitchMode(INSERT);
        bufferGotoEnd(buf, editor.mode, 0);
        bufferInsertNewLine(buf);
        break;
      case 'O':
        editorSwitchMode(INSERT);
        bufferGotoEnd(buf, editor.mode, 0);
        bufferInsertNewLine(buf);
        buf->cursor.y--;
        bufferSwapRow(buf, buf->cursor.y, buf->cursor.y+1);
        break;
      case 'r':
        if (pc.arg1 == 0)
          return;
        if (pc.arg1 < 127)
          bufferReplaceChar(buf, pc.arg1, pc.repx);
        break;
      case 'v':
        editorSwitchMode(VISUAL);
        break;
      case 'V':
        editorSwitchMode(VISUAL_LINE);
        break;
      default:
        success = 0;
    }
    if(success){
      emptyStack(&editor.cmdstk);
      return;
    }
  }

  if(editor.mode == VISUAL || editor.mode == VISUAL_LINE){
    // TODO : visual mode specific keybinds
    int success = 1;
    switch(pc.cmd){
      case CTRL_C:
      case ESCAPE:
        editorSwitchMode(NORMAL);
        break;
      case 'o':
      case 'O':
        bufferSwapSelCursor(buf);
        break;
      case 'd':
      case 'x':
        bufferDeleteSelection(buf);
        editorSwitchMode(NORMAL);
        break;
      case 'c':
        {
          int ec = buf->cursor.y;
          bufferDeleteSelection(buf);
          editorSwitchMode(INSERT);
          if(editor.mode == VISUAL) bufferMoveCursor(buf, ARROW_RIGHT, editor.mode, 1);
          else if(ec >= buf->row_size) {
            bufferGotoEnd(buf, editor.mode, 0);
            bufferInsertNewLine(buf);
          }
        }
        break;
      case 'r':
        if (pc.arg1 == 0) return;
        if (pc.arg1 < 127) bufferReplaceSelection(buf, pc.arg1);
        break;
      case 'v':
        editor.mode = editor.mode==VISUAL_LINE?VISUAL:VISUAL_LINE;
        break;
      default:
        success=0;
    }
    if(success){
      emptyStack(&editor.cmdstk);
      return;
    }
  }

  switch (pc.cmd) { // both normal and visual mode
    case '/':
      emptyStack(&editor.cmdstk);
      editorFind("/%s");
      break;
    case '0':
      bufferGotoEnd(buf, editor.mode, JMP_BACK);
      break;
    case '_':
      buf->cursor.x = firstCharIndex(buf->rows[buf->cursor.y].chars);
      buf->st.cursx = buf->cursor.x;
      break;
    case '$':
      buf->cursor.y += pc.repx - 1;
      bufferGotoEnd(buf, editor.mode, 0);
      break;
    case '}':
      while (pc.repx-- && bufferParaNav(buf, 0)) {
      }
      break;
    case '{':
      while (pc.repx-- && bufferParaNav(buf, JMP_BACK)) {
      }
      break;
    case 'w':
      while (pc.repx-- && bufferWordJump(buf, 0)) {
      }
      break;
    case 'W':
      while (pc.repx-- && bufferWordJump(buf, JMP_PUNC)) {
      }
      break;
    case 'e':
      while (pc.repx-- && bufferWordJump(buf, JMP_END)) {
      }
      break;
    case 'E':
      while (pc.repx-- && bufferWordJump(buf, JMP_END | JMP_PUNC)) {
      }
      break;
    case 'b':
      while (pc.repx-- && bufferWordJump(buf, JMP_BACK)) {
      }
      break;
    case 'B':
      while (pc.repx-- && bufferWordJump(buf, JMP_BACK | JMP_PUNC)) {
      }
      break;

    case 'h':
    case 'j':
    case 'k':
    case 'l':
    case ARROW_LEFT:
    case ARROW_RIGHT:
    case ARROW_UP:
    case ARROW_DOWN:
      bufferMoveCursor(buf, pc.cmd, editor.mode, pc.repx);
      break;

    case 'G':
      buf->cursor.y = buf->row_size-1;
      buf->cursor.x = clamp(buf->cursor.x, 0, buf->rows[buf->cursor.y].size-1);
      break;
    case 'J':
      bufferAbsoluteJump(buf, pc.repx);
      break;
    case 'H':
      buf->cursor.y = buf->offset.y + settings.scrollpadding;
      if (buf->cursor.y >= buf->row_size)
        buf->cursor.y = buf->row_size - 1;
      break;
    case 'L':
      buf->cursor.y = buf->offset.y + buf->size.y - 1 - settings.scrollpadding;
      if (buf->cursor.y >= buf->row_size)
        buf->cursor.y = buf->row_size - 1;
      break;

    case ':':
      emptyStack(&editor.cmdstk);
      editorCmdPromptProc(":%s");
      break;
    case 'f':
      if (pc.arg1 == 0)
        return;
      while (pc.repx-- && bufferFindChar(buf, pc.arg1, 0)) {
      }
      break;
    case 'F':
      if (pc.arg1 == 0)
        return;
      while (pc.repx-- && bufferFindChar(buf, pc.arg1, JMP_BACK)) {
      }
      break;

    case 0:
      return;
    default:
      break;
  }
  emptyStack(&editor.cmdstk);
}

void editorCmdPromptProc(char *prompt) {
  char *cmd = editorPrompt(prompt ? prompt : "Enter command:", NULL);
  if(!cmd) return;

  int _cl = strlen(cmd);
  int _cmdtype = CMD_NONE;
  int val = 0, i = 0;

  while (i < _cl) {
    if (cmd[i] >= '0' && cmd[i] <= '9') {
      _cmdtype = CMD_NUM;
      val = val * 10 + cmd[i] - '0';
    } else
      break;
    i++;
  }

  if (_cmdtype == CMD_NUM) {
    bufferAbsoluteJump(editorGetCurrentBuffer(), val);
    return;
  }
  if(editor.mode == VISUAL_LINE || editor.mode == VISUAL) {
    editorSetStatusMsg("\x1b[91m Not an command during vis. mode: %s\x1b[0m", cmd);
    return;
  }

  while (i < _cl) {
    if (cmd[i] == 'w') {
      if (_cmdtype & (CMD_WRITE | CMD_QUIT | CMD_ALL | CMD_QUIT)) {
        _cmdtype = CMD_ERROR;
        break;
      }
      _cmdtype |= CMD_WRITE;
    } else if (cmd[i] == 'q') {
      if (_cmdtype & (CMD_QUIT | CMD_ALL | CMD_QUIT)) {
        _cmdtype = CMD_ERROR;
        break;
      }
      _cmdtype |= CMD_QUIT;
    } else if (cmd[i] == 'a') {
      if (_cmdtype & (CMD_ALL | CMD_QUIT)) {
        _cmdtype = CMD_ERROR;
        break;
      }
      _cmdtype |= CMD_ALL;
    } else if (cmd[i] == '!') {
      if (!(_cmdtype & CMD_QUIT)) {
        _cmdtype = CMD_ERROR;
        break;
      }
      _cmdtype |= CMD_FORCE;
    } else {
      _cmdtype = CMD_ERROR;
      break;
    }
    i++;
  }
  if(_cmdtype == CMD_ERROR){
    editorSetStatusMsg("\x1b[91m Not an editor Command: %s\x1b[0m", cmd);
    return;
  }
  editorSaveQuitBuffers(_cmdtype);
}

void editorInsertModeKeyProc(int c) {
  buffer *buf = editorGetCurrentBuffer();
  switch (c) {
    case RETURN:
      bufferInsertNewLine(buf);
      break;
    case BACKSPACE:
    case CTRL_H:
      bufferDelChar(buf, -1);
      break;
    case DEL_KEY:
      bufferDelChar(buf, 0);
      break;

    case ARROW_LEFT:
    case ARROW_RIGHT:
    case ARROW_UP:
    case ARROW_DOWN:
      bufferMoveCursor(buf, c, editor.mode, 1);
      break;

    case CTRL_C:
    case ESCAPE:
      editorSwitchMode(NORMAL);
      break;
    default:
      bufferInsertChar(buf, c);
      break;
  }
}

void editorSwitchMode(int mode) {
  if (mode == NORMAL)
    editor.statusmsg_time = 0;
  else if (mode == VISUAL || mode == VISUAL_LINE){
    buffer *buf = editorGetCurrentBuffer();
    bufferUpdateSelection(buf, mode, 0);
  }

  if (editor.mode == INSERT)
    bufferMoveCursor(&editor.buf, ARROW_LEFT, mode, 1);
  editor.mode = mode;
}

void editorVisLineModeProc(int c) {
  static int qcount = 2;
  switch(c){
    case CTRL_Z:
      if(editor.buf.dirty && --qcount) {
        editorSetStatusMsg( "WARNING! unchanged changes. Press CTRL-Z again to force quit");
        return;
      }
      exit(0);
      break;
    case CTRL_C:
    case ESCAPE:
      editorSwitchMode(NORMAL);
      break;
    default:
      push(&editor.cmdstk, c);
  }
  
  parsedcmd_t pc = parseCommand(&editor.cmdstk);
  if (!pc.repx) pc.repx++;

  buffer *buf = editorGetCurrentBuffer();

  switch(pc.cmd){
    case 'h':
    case 'j':
    case 'k':
    case 'l':
    case ARROW_LEFT:
    case ARROW_RIGHT:
    case ARROW_UP:
    case ARROW_DOWN:
      bufferMoveCursor(buf, pc.cmd, editor.mode, pc.repx);
      break;
    case ':':
      emptyStack(&editor.cmdstk);
      editorCmdPromptProc(":%s");
      break;
    case '/':
      emptyStack(&editor.cmdstk);
      editorFind("/%s");
      break;
    default:
      break;
  }
  emptyStack(&editor.cmdstk);
}

void editorScroll() { bufferScroll(editorGetCurrentBuffer()); }

void editorOpen(char *filename) { bufferOpenFile(&editor.buf, filename); }

void editorSaveBuffer(buffer *buf) {
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

void editorSaveQuitBuffers(int flags) {
  // TODO: multiple buffers handling ....
  buffer *buf = &editor.buf;
  if (flags&CMD_WRITE) editorSaveBuffer(buf);
  if ((flags&CMD_QUIT)){
    if(buf->dirty && !(flags&CMD_FORCE)) {
      editorSetStatusMsg("\x1b[91m Unsaved Changes. Use q! to force quit buffer. \x1b[0m");
    } else {
      exit(0);
    }
  }
}

char *editorPrompt(char *prompt, void (*callback)(char *, int)) {
  size_t bufsize = 128;
  char *buf = malloc(bufsize);
  size_t buflen = 0;
  buf[buflen] = '\0';

  while (1) {
    editorSetStatusMsg(prompt, buf);
    editorRefreshScreen();
    showCursor(editor.screen_size.y, strlen(editor.statusmsg)+1);

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

  buffer *_cbuf = &editor.buf;

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

  if (bufferFind(_cbuf, query, dir) == 0) {
    // save state for reverting later
    _st_hlind = _cbuf->cursor.y;
    erow *row = &_cbuf->rows[_st_hlind];
    _st_hl = malloc(row->rsize);
    memcpy(_st_hl, row->hlchars, row->rsize);

    // highlight found query result
    memset(&row->hlchars[rowCursorToRenderX(row, _cbuf->cursor.x)], TK_MATCH,
           strlen(query));
  }
}

void editorFind(char *prompt) {
  vec2 _st_cur = editor.buf.cursor;
  vec2 _st_off = editor.buf.offset;

  char *query = editorPrompt(prompt ? prompt : "Search: %s (Esc to cancel)",
                             editorFindCallback);
  if (query) {
    if (editor.buf.st.query)
      free(editor.buf.st.query);
    editor.buf.st.query = query;
  } else {
    editor.buf.cursor = _st_cur;
    editor.buf.offset = _st_off;
  }
}
