#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ctype.h>

#include "editor.h"
#include "input.h"
#include "settings.h"

// -- data --
editorconf editor;
extern settingsType settings;

char test[] = "hehehshshsh \
               heheheheheheh";
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

  if (getWindowSize(&editor.screen_size.y, &editor.screen_size.x) == -1)
    die("invalid window size");

  editor.buf.size.y = editor.screen_size.y-2;
  editor.buf.size.x = editor.screen_size.x;
}

void editorDrawBuffers(abuf *ab) {
  bufferDrawRows(&editor.buf, ab);
  bufferDrawStatusBar(&editor.buf, ab);
}

void editorShowCursor(abuf *ab){
  bufferShowCursor(&editor.buf, ab);
}

void editorDrawStsMsgBar(abuf *ab) {
  abAppend(ab, "\x1b[K", 3);
  if (time(NULL) - editor.statusmsg_time < 5) {
    int mlen = strlen(editor.statusmsg);
    if (mlen > editor.screen_size.x)
      mlen = editor.screen_size.x;
    if (mlen)
      abAppend(ab, editor.statusmsg, mlen);
  }
}

void editorSetStatusMsg(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(editor.statusmsg, sizeof(editor.statusmsg), fmt, ap);
  va_end(ap);
  editor.statusmsg_time = time(NULL);
}

void editorRefreshScreen() {
  editorScroll();
  abuf ab = ABUF_INIT;

  abAppend(&ab, "\x1b[?25l", 6); // hide cursor
  abAppend(&ab, "\x1b[H", 3);    // move cursor to start before drawing

  editorDrawBuffers(&ab);
  editorDrawStsMsgBar(&ab);
  editorShowCursor(&ab);

  write(STDOUT_FILENO, ab.buf, ab.len);
  abFree(&ab);
}

void editorProcessKeyPress() {
  int c = readKey();
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

  case HOME_KEY:
    editor.buf.cursor.x = 0;
    break;

  case END_KEY:
    if (editor.buf.cursor.y < editor.buf.row_size) {
      editor.buf.cursor.x = editor.buf.rows[editor.buf.cursor.y].size - 1;
      editor.buf.st.cursx = editor.buf.cursor.x;
    }
    break;

  case PAGE_UP:
  case PAGE_DOWN: {
    if (c == PAGE_UP)
      editor.buf.cursor.y = editor.buf.offset.y + settings.scrollpadding;
    else {
      editor.buf.cursor.y =
          editor.buf.offset.y + editor.buf.size.y - 1 - settings.scrollpadding;
      if (editor.buf.cursor.y >= editor.buf.row_size)
        editor.buf.cursor.y = editor.buf.row_size-1;
    }
    int _times = editor.buf.size.y;
    while (_times--)
      editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
  } break;

  case ARROW_LEFT:
  case ARROW_RIGHT:
  case ARROW_UP:
  case ARROW_DOWN:
    editorMoveCursor(c);
    break;

  case CTRL_A:
    editorSaveBuffer();
    break;

  case CTRL_F:
    editorFind();
    break;

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

  case CTRL_C:
  case ESCAPE:
    // TODO: normal mode switch
    break;
  default:
    editorInsertChar(c);
    break;
  }
  qcount = 2;
}

void editorMoveCursor(int key) {
  bufferMoveCursor(&editor.buf, key);
}

void editorScroll() {
  bufferScroll(&editor.buf);
}

void editorInsertRow(int index, char *s, size_t len) {
  bufferInsertRow(&editor.buf, index, s, len);
}

void editorDeleteRow(int index){
  bufferDeleteRow(&editor.buf, index);
}

void editorOpen(char *filename) {
  bufferOpenFile(&editor.buf, filename);
}

void editorInsertChar(int ch) {
  bufferInsertChar(&editor.buf, ch);
}

void editorInsertNewLine(){
  bufferInsertNewLine(&editor.buf);
}

void editorDelChar(int dir){
  bufferDelChar(&editor.buf, dir);
}

void editorSaveBuffer() {
  if (editor.buf.filename == NULL){
    if((editor.buf.filename = editorPrompt("Save as: %s", NULL)) == NULL){
      editorSetStatusMsg("Save Aborted...");
      return;
    }
  }

  int _sz = bufferSave(&editor.buf);
  if(_sz == -1)
    editorSetStatusMsg("Save Failed I/O error: %s", strerror(errno));
  else
    editorSetStatusMsg("%d bytes written to disk", _sz);
}


char *editorPrompt(char *prompt, void(*callback)(char*, int)){
  size_t bufsize = 128;
  char *buf = malloc(bufsize);
  size_t buflen = 0;
  buf[buflen] = '\0';

  while(1){
    editorSetStatusMsg(prompt, buf);
    editorRefreshScreen();

    int c = readKey();

    if(c == ESCAPE){
      editorSetStatusMsg("");
      if(callback) callback(buf, c);
      free(buf);
      return NULL;
    }else if(c == BACKSPACE || c == CTRL_H || c == DEL_KEY){
      if(buflen != 0) buf[--buflen] = '\0';
    } else if(c == RETURN){
      editorSetStatusMsg("");
      if(callback) callback(buf, c);
      return buf;
    }else if(!iscntrl(c) && c <128){
      if(buflen == bufsize-1){
        bufsize*=2;
        buf = realloc(buf, bufsize);
      }
      buf[buflen++] = c;
      buf[buflen] = '\0';
    }
    if(callback) callback(buf, c);
  }
}

void editorFindCallback(char *query, int key){
  static int _st_hlind;
  static unsigned char *_st_hl = NULL;

  buffer *_cbuf = &editor.buf;

  if(_st_hl){
    memcpy(_cbuf->rows[_st_hlind].hlchars, _st_hl, _cbuf->rows[_st_hlind].rsize);
    free(_st_hl);
    _st_hl=NULL;
  }

  if(key == RETURN || key == ESCAPE) return;
  if(query == NULL) return;
  int dir = 0;
  if(key == ARROW_UP) dir=-1;
  else if(key == ARROW_DOWN) dir=1;

  if(bufferFind(_cbuf, query, dir)==0){
    // save state for reverting later
    _st_hlind = _cbuf->cursor.y;
    erow *row = &_cbuf->rows[_st_hlind];
    _st_hl = malloc(row->rsize);
    memcpy(_st_hl, row->hlchars, row->rsize);

    // highlight found query result
    memset(&row->hlchars[rowCursorToRenderX(row, _cbuf->cursor.x)], TK_MATCH, strlen(query));
  }
}

void editorFind(){
  vec2 _st_cur = editor.buf.cursor;
  vec2 _st_off = editor.buf.offset;

  char *query = editorPrompt("Search: %s (Esc to cancel)", editorFindCallback);
  if(query) {
    if(editor.buf.st.query) free(editor.buf.st.query);
    editor.buf.st.query = query;
  } else{
    editor.buf.cursor = _st_cur;
    editor.buf.offset = _st_off;
  }
}
