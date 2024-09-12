#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "editor.h"
#include "settings.h"
#include "utils.h"
#include "input.h"

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
  editor.cursor_x = 0;
  editor.cursor_y = 0;
  editor.render_x = 0;
  editor.numrows = 0;
  editor.offset_x = 0;
  editor.offset_y = 0;
  editor.row = NULL;
  editor.filename = NULL;
  editor.statusmsg[0] = '\0';
  editor.statusmsg_time = 0;

  if (getWindowSize(&editor.screen_rows, &editor.screen_cols) == -1)
    die("invalid window size");
  editor.screen_rows-=2;
}

void addWelcomeMsg(abuf *ab) {
  char wlc[80];
  int wlclen =
      snprintf(wlc, sizeof(wlc), "therealtxt editor v %s", EDITOR_VERSION);
  if (wlclen > editor.screen_cols)
    wlclen = editor.screen_cols;

  int padding = (editor.screen_cols - wlclen) / 2;
  if (padding) {
    abAppend(ab, "~", 1);
    padding--;
  }
  while (padding--) {
    abAppend(ab, " ", 1);
  }
  abAppend(ab, wlc, wlclen);
}

void editorDrawRows(abuf *ab) {
  for (int y = 0; y < editor.screen_rows; y++) {

    int _fr = y+editor.offset_y;
    if(_fr >= editor.numrows){
      if (editor.numrows==0 && y == editor.screen_rows / 3) addWelcomeMsg(ab);
      else abAppend(ab, "~", 1);
    } else {
      int len = editor.row[_fr].rsize - editor.offset_x;
      if(len < 0) len = 0;
      if(len > editor.screen_cols) len = editor.screen_cols;
      abAppend(ab, &editor.row[_fr].renderch[editor.offset_x], len);
    }

    abAppend(ab, "\x1b[K", 3); // clear the line before drawing
    abAppend(ab, "\r\n", 2);
  }
}

void editorDrawStatusBar(abuf *buf){
  abAppend(buf, "\x1b[7m", 4);

  char lstatus[80], rstatus[80];
  int len = snprintf(lstatus, sizeof(lstatus),
      "%.20s", editor.filename ? editor.filename : "[No name]");
  if(len > editor.screen_cols) len = editor.screen_cols;

  int rlen = snprintf(rstatus, sizeof(rstatus),
      "%d,%d", editor.cursor_y, editor.cursor_x);

  abAppend(buf, lstatus, len);
  for(;len<editor.screen_cols-rlen;len++) abAppend(buf, " ", 1);
  abAppend(buf, rstatus, rlen);

  abAppend(buf, "\r\n", 2);
  abAppend(buf, "\x1b[m", 3);
}

void editorDrawStsMsgBar(abuf *ab){
  abAppend(ab, "\x1b[K", 3);
  if(time(NULL) - editor.statusmsg_time < 5){
    int mlen = strlen(editor.statusmsg);
    if(mlen > editor.screen_cols) mlen = editor.screen_cols;
    if(mlen) abAppend(ab, editor.statusmsg, mlen);
  }
}

void editorSetStatusMsg(const char *fmt, ...){
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

  editorDrawRows(&ab);
  editorDrawStatusBar(&ab);
  editorDrawStsMsgBar(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", editor.cursor_y-editor.offset_y + 1, editor.render_x -editor.offset_x + 1);
  abAppend(&ab, buf, strlen(buf));
  abAppend(&ab, "\x1b[?25h", 6); // show the cursor

  write(STDOUT_FILENO, ab.buf, ab.len);
  abFree(&ab);
}


void editorProcessKeyPress() {
  int c = readKey();

  switch (c) {
    case CTRl_Z:
      clearTerminal();
      exit(0);
      break;

    case HOME_KEY: editor.cursor_x = 0; break;
    case END_KEY:
      if(editor.cursor_y < editor.numrows) {
        editor.cursor_x = editor.row[editor.cursor_y].size-1;
        editor.st_cx = editor.cursor_x;
      }
      break;
    case PAGE_UP:
    case PAGE_DOWN: {
        if(c == PAGE_UP) editor.cursor_y = editor.offset_y+settings.scrollpadding;
        else {
          editor.cursor_y = editor.offset_y+editor.screen_rows-1-settings.scrollpadding;
          if(editor.cursor_y > editor.numrows) editor.cursor_y = editor.numrows;
        }
        int _times = editor.screen_rows;
        while (_times--) editorMoveCursor(c == PAGE_UP? ARROW_UP: ARROW_DOWN);
      }
      break;
    case ARROW_LEFT:
    case ARROW_RIGHT:
    case ARROW_UP:
    case ARROW_DOWN:
      editorMoveCursor(c);
      break;
  }
}

void editorMoveCursor(int key) {
  erow *row = editor.cursor_y >= editor.numrows ? NULL : &editor.row[editor.cursor_y];

  switch (key) {
    case ARROW_LEFT:
      if (editor.cursor_x > 0) editor.cursor_x--;
      editor.st_cx = editor.cursor_x;
      break;
    case ARROW_RIGHT:
      if (row && editor.cursor_x < row->size-1) editor.cursor_x++;
      editor.st_cx = editor.cursor_x;
      break;
    case ARROW_UP:
      if(editor.cursor_y > 0) editor.cursor_y--;
      break;
    case ARROW_DOWN:
      if (editor.cursor_y<editor.numrows-1) editor.cursor_y++;
      break;
  }

  // cursor state preservation
  if (editor.cursor_y < editor.numrows){
    row = &editor.row[editor.cursor_y];
    editor.cursor_x = editor.st_cx;
    if(editor.cursor_x >= row->size) editor.cursor_x = row->size==0?0:row->size-1;
  }
}

void editorScroll(){
  int _cy = editor.cursor_y-editor.offset_y;

  // vertical scroll handler
  if(_cy > (editor.screen_rows-1-settings.scrollpadding))
    editor.offset_y = editor.cursor_y-editor.screen_rows+settings.scrollpadding+1;
  else if(_cy < settings.scrollpadding){
    editor.offset_y = editor.cursor_y-settings.scrollpadding;
    if(editor.offset_y<0) editor.offset_y = 0;
  }

  // horizontal scroll handler
  editor.render_x = editor.cursor_y < editor.numrows? rowCursorToRenderX(&editor.row[editor.cursor_y], editor.cursor_x): 0;
  int _cx = editor.render_x-editor.offset_x;
  if(_cx >= editor.screen_cols-1) editor.offset_x = editor.render_x-editor.screen_cols+1;
  else if(_cx < 0) editor.offset_x = editor.render_x;
}

void editorAppendRows(char *s, size_t len){
  editor.row = realloc(editor.row, sizeof(erow) * (editor.numrows+1));

  erow *_rw = &editor.row[editor.numrows];
  _rw->size = len;
  _rw->chars = malloc(len+1);
  memcpy(_rw->chars, s, len);
  _rw->chars[len]='\0';
  _rw->rsize=0;
  _rw->renderch=NULL;
  rowUpdate(_rw);
  editor.numrows++;
}

void editorOpen(char* filename){
  free(editor.filename);
  editor.filename = strdup(filename);

  FILE *fp = fopen(editor.filename, "r");
  if(!fp) die("failed to open file");

  char *line = NULL;
  size_t lcap = 0;
  ssize_t llen;

  while((llen = getline(&line, &lcap, fp)) != -1){
    while(llen>0 && (line[llen-1]=='\n' || line[llen-1]=='\r')) llen--;
    editorAppendRows(line, llen);
  }
  free(line);
  fclose(fp);
}
