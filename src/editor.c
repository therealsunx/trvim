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

  editorUpdateSize();
  atexit(freeEditor);
}

void freeEditor(){
  freeStack(&editor.cmdstk);
  freeBuffer(&editor.buf);
}

void editorUpdateSize() {
  vec2 _sz;
  if (getWindowSize(&_sz.y, &_sz.x) == -1)
    die("invalid window size");

  if (_sz.x == editor.screen_size.x && _sz.y == editor.screen_size.y)
    return;
  editor.screen_size.x = _sz.x;
  editor.screen_size.y = _sz.y;
  bufferUpdateSize(&editor.buf, editor.screen_size.x, editor.screen_size.y - 2);
}

void editorDrawBuffers(abuf *ab) {
  bufferDrawRows(&editor.buf, ab);
  bufferDrawStatusBar(&editor.buf, ab);
}

void editorShowCursor(abuf *ab) { bufferShowCursor(&editor.buf, ab); }

void editorDrawStsMsgBar(abuf *ab) {
  char cmdsts[editor.cmdstk.top];
  for(int i=0; i<editor.cmdstk.top; i++) cmdsts[i] = editor.cmdstk.vals[i];

  int psize = editor.screen_size.x-editor.cmdstk.top;

  abAppend(ab, "\x1b[K", 3);
  if (time(NULL) - editor.statusmsg_time < 5) {
    int _slen = strlen(editor.statusmsg);
    int mlen = clamp(_slen, 0, psize);
    if (mlen) abAppend(ab, &editor.statusmsg[_slen-mlen], mlen);
    psize -= mlen;
  }

  for(;psize>0;psize--) abAppend(ab, " ", 1);

  abAppend(ab, cmdsts, editor.cmdstk.top);
}

void editorSetStatusMsg(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(editor.statusmsg, sizeof(editor.statusmsg), fmt, ap);
  va_end(ap);
  editor.statusmsg_time = time(NULL);
}

void editorStatusBarUpdate(){
  if(editor.mode == NORMAL) return;
  editor.statusmsg_time = time(NULL);
  if(editor.mode == INSERT) editorSetStatusMsg("-- INSERT --");
  else if(editor.mode == VISUAL) editorSetStatusMsg("-- VISUAL --");
}

void editorRefreshScreen() {
  editorUpdateSize();
  editorScroll();
  abuf ab = ABUF_INIT;

  abAppend(&ab, "\x1b[?25l", 6); // hide cursor
  abAppend(&ab, "\x1b[H", 3);    // move cursor to start before drawing

  if(editor.mode != INSERT) abAppend(&ab, "\x1b[2 q", 5);
  else abAppend(&ab, "\x1b[6 q", 5);

  editorDrawBuffers(&ab);
  editorDrawStsMsgBar(&ab);
  editorShowCursor(&ab);

  write(STDOUT_FILENO, ab.buf, ab.len);
  abFree(&ab);
}

void editorProcessKeyPress() {
  int c = readKey();

  switch(c){
    case HOME_KEY:
      editorGotoEnd(JMP_BACK, 1);
      break;
    case END_KEY:
      editorGotoEnd(0, 1);
      break;

    case PAGE_UP:
    case PAGE_DOWN: 
      editorPageScroll(c);
      break;

    default:
      if(editor.mode == NORMAL)
        editorNormalModeKeyProc(c);
      else if(editor.mode == INSERT)
        editorInsertModeKeyProc(c);
      else if(editor.mode == VISUAL)
        editorVisualModeKeyProc(c);
  }
}

void editorNormalModeKeyProc(int c) {
  static int qcount = 2;
  switch (c) {
    case CTRl_Z:
      if (editor.buf.dirty && --qcount > 0) {
        editorSetStatusMsg(
            "WARNING! unchanged changes. Press CTRL-Z again to force quit");
        return;
      }
      clearTerminal();
      exit(0);
      break;

    case CTRL_A:
      editorSaveBuffer();
      break;

    default:
      push(&editor.cmdstk, c);
  }
  editorProcessCommand();
  qcount=2;
}

void editorProcessCommand(){
  // parse the command
  // if executable, then empty the stack and execute command
  // else just return

  parsedcmd_t pc = parseCommand(&editor.cmdstk);
  if(!pc.repx) pc.repx++;
  // take action
  switch(pc.cmd){
    case 'i':
      editorSwitchMode(INSERT);
      break;
    case 'a':
      editorSwitchMode(INSERT);
      editorMoveCursor(ARROW_RIGHT, 1);
      break;
    case '/':
      editorFind("/%s");
      break;
    case '0':
      editorGotoEnd(JMP_BACK, 1);
      break;

    case '$':
      editorGotoEnd(0, pc.repx);
      break;
    case '}':
      editorParaNav(0, pc.repx);
      break;
    case '{':
      editorParaNav(JMP_BACK, pc.repx);
      break;

    case 'w':
      editorGotoNextWord(0, pc.repx);
      break;
    case 'W':
      editorGotoNextWord(JMP_PUNC, pc.repx);
      break;
    case 'e':
      editorGotoNextWord(JMP_END, pc.repx);
      break;
    case 'E':
      editorGotoNextWord(JMP_END|JMP_PUNC, pc.repx);
      break;
    case 'b':
      editorGotoNextWord(JMP_BACK, pc.repx);
      break;
    case 'B':
      editorGotoNextWord(JMP_BACK|JMP_PUNC, pc.repx);
      break;

    case 'h':
    case 'j':
    case 'k':
    case 'l':
    case ARROW_LEFT:
    case ARROW_RIGHT:
    case ARROW_UP:
    case ARROW_DOWN:
      editorMoveCursor(pc.cmd, pc.repx);
      break;

    case 'H':
      editorAbsoluteJump(pc.repx);
      break;

    case 'f':
      if(pc.arg1 == 0) return;
      editorFindChar(pc.arg1, 0, pc.repx);
      break;
    case 'F':
      if(pc.arg1 == 0) return;
      editorFindChar(pc.arg1, JMP_BACK, pc.repx);
      break;
    case 'r':
      if(pc.arg1 == 0) return;
      if(pc.arg1 < 127) editorReplaceChar(pc.arg1, pc.repx);
      break;

    case 0:
      return;
    default:
      break;
  }
  emptyStack(&editor.cmdstk);
}

void editorInsertModeKeyProc(int c) {
  switch(c){
    case RETURN:
      editorInsertNewLine();
      break;
    case BACKSPACE:
    case CTRL_H:
      editorDelChar(-1);
      break;
    case DEL_KEY:
      editorDelChar(0);
      break;

    case ARROW_LEFT:
    case ARROW_RIGHT:
    case ARROW_UP:
    case ARROW_DOWN:
      editorMoveCursor(c, 1);
      break;

    case CTRL_C:
    case ESCAPE:
      editorSwitchMode(NORMAL);
      break;
    default:
      editorInsertChar(c);
      break;
  }
}

void editorSwitchMode(int mode){
  if(mode == NORMAL) editor.statusmsg_time = 0;

  if(editor.mode == INSERT)
    bufferMoveCursor(&editor.buf, ARROW_LEFT, mode, 1);
  editor.mode = mode;
}

void editorVisualModeKeyProc(int c){
  switch(c){
  }
}

void editorGotoEnd(int posflg, int repx){
  buffer *buf = &editor.buf;
  if(!posflg) buf->cursor.y += repx-1;
  bufferGotoEnd(buf, editor.mode, posflg);
}

void editorGotoNextWord(int flags, int repx){
  while(repx-- && bufferWordJump(&editor.buf, flags)){};
}

void editorFindChar(char char_, int dirflag, int repx){
  while(repx-- && bufferFindChar(&editor.buf, char_, dirflag)){};
}

void editorReplaceChar(char char_, int repx){
  bufferReplaceChar(&editor.buf, char_, repx);
}

void editorParaNav(int dirflag, int repx){
  while(repx-- && bufferParaNav(&editor.buf, dirflag)) {};
}

void editorPageScroll(int key){ bufferPageScroll(&editor.buf, key); }

void editorMoveCursor(int key, int repx) {
  bufferMoveCursor(&editor.buf, key, editor.mode, repx);
}

void editorAbsoluteJump(int line){
  bufferAbsoluteJump(&editor.buf, line);
}

void editorScroll() { bufferScroll(&editor.buf); }

void editorInsertRow(int index, char *s, size_t len) {
  bufferInsertRow(&editor.buf, index, s, len);
}

void editorDeleteRow(int index) { bufferDeleteRow(&editor.buf, index); }

void editorOpen(char *filename) { bufferOpenFile(&editor.buf, filename); }

void editorInsertChar(int ch) { bufferInsertChar(&editor.buf, ch); }

void editorInsertNewLine() { bufferInsertNewLine(&editor.buf); }

void editorDelChar(int dir) { bufferDelChar(&editor.buf, dir); }

void editorSaveBuffer() {
  if (editor.buf.filename == NULL) {
    if ((editor.buf.filename = editorPrompt("Save as: %s", NULL)) == NULL) {
      editorSetStatusMsg("Save Aborted...");
      return;
    }
  }

  int _sz = bufferSave(&editor.buf);
  if (_sz == -1)
    editorSetStatusMsg("Save Failed I/O error: %s", strerror(errno));
  else
    editorSetStatusMsg("%d bytes written to disk", _sz);
}

char *editorPrompt(char *prompt, void (*callback)(char *, int)) {
  size_t bufsize = 128;
  char *buf = malloc(bufsize);
  size_t buflen = 0;
  buf[buflen] = '\0';

  while (1) {
    editorSetStatusMsg(prompt, buf);
    editorRefreshScreen();

    int c = readKey();

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

  char *query = editorPrompt(prompt? prompt: "Search: %s (Esc to cancel)", editorFindCallback);
  if (query) {
    if (editor.buf.st.query)
      free(editor.buf.st.query);
    editor.buf.st.query = query;
  } else {
    editor.buf.cursor = _st_cur;
    editor.buf.offset = _st_off;
  }
}
