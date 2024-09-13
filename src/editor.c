#include <ctype.h>
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

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
#include "utils.h"

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
  editor.dirty = 0;
  editor.statusmsg[0] = '\0';
  editor.statusmsg_time = 0;

  if (getWindowSize(&editor.screen_rows, &editor.screen_cols) == -1)
    die("invalid window size");
  editor.screen_rows -= 2;
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

    int _fr = y + editor.offset_y;
    if (_fr >= editor.numrows) {
      if (editor.numrows == 0 && y == editor.screen_rows / 3)
        addWelcomeMsg(ab);
      else
        abAppend(ab, "~", 1);
    } else {
      int len = editor.row[_fr].rsize - editor.offset_x;
      if (len < 0)
        len = 0;
      if (len > editor.screen_cols)
        len = editor.screen_cols;
      abAppend(ab, &editor.row[_fr].renderchars[editor.offset_x], len);
    }

    abAppend(ab, "\x1b[K", 3); // clear the line before drawing
    abAppend(ab, "\r\n", 2);
  }
}

void editorDrawStatusBar(abuf *buf) {
  abAppend(buf, "\x1b[7m", 4);

  char lstatus[80], rstatus[80];
  int len = snprintf(lstatus, sizeof(lstatus), "%.20s %.3s",
                     editor.filename ? editor.filename : "[No name]",
                     editor.dirty ? "[+]" : "");
  if (len > editor.screen_cols)
    len = editor.screen_cols;

  int lsz = editor.cursor_y<editor.numrows? editor.row[editor.cursor_y].size:0;
  int rlen = snprintf(rstatus, sizeof(rstatus), "%d, %d|%d", editor.cursor_y,
                      editor.cursor_x, lsz);

  abAppend(buf, lstatus, len);
  for (; len < editor.screen_cols - rlen; len++)
    abAppend(buf, " ", 1);
  abAppend(buf, rstatus, rlen);

  abAppend(buf, "\r\n", 2);
  abAppend(buf, "\x1b[m", 3);
}

void editorDrawStsMsgBar(abuf *ab) {
  abAppend(ab, "\x1b[K", 3);
  if (time(NULL) - editor.statusmsg_time < 5) {
    int mlen = strlen(editor.statusmsg);
    if (mlen > editor.screen_cols)
      mlen = editor.screen_cols;
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

  editorDrawRows(&ab);
  editorDrawStatusBar(&ab);
  editorDrawStsMsgBar(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH",
           editor.cursor_y - editor.offset_y + 1,
           editor.render_x - editor.offset_x + 1);
  abAppend(&ab, buf, strlen(buf));
  abAppend(&ab, "\x1b[?25h", 6); // show the cursor

  write(STDOUT_FILENO, ab.buf, ab.len);
  abFree(&ab);
}

void editorProcessKeyPress() {
  int c = readKey();
  static int qcount = 2;

  switch (c) {
  case CTRl_Z:
    if (editor.dirty && --qcount > 0) {
      editorSetStatusMsg(
          "WARNING! unchanged changes. Press CTRL-Z again to force quit");
      return;
    }
    clearTerminal();
    exit(0);
    break;

  case HOME_KEY:
    editor.cursor_x = 0;
    break;

  case END_KEY:
    if (editor.cursor_y < editor.numrows) {
      editor.cursor_x = editor.row[editor.cursor_y].size - 1;
      editor.st_cx = editor.cursor_x;
    }
    break;

  case PAGE_UP:
  case PAGE_DOWN: {
    if (c == PAGE_UP)
      editor.cursor_y = editor.offset_y + settings.scrollpadding;
    else {
      editor.cursor_y =
          editor.offset_y + editor.screen_rows - 1 - settings.scrollpadding;
      if (editor.cursor_y >= editor.numrows)
        editor.cursor_y = editor.numrows-1;
    }
    int _times = editor.screen_rows;
    while (_times--)
      editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
  } break;

  case ARROW_LEFT:
  case ARROW_RIGHT:
  case ARROW_UP:
  case ARROW_DOWN:
    editorMoveCursor(c);
    break;

  case CTRL_S:
    editorSaveBuffer();
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
  erow *row =
      editor.cursor_y >= editor.numrows ? NULL : &editor.row[editor.cursor_y];

  switch (key) {
  case ARROW_LEFT:
    if (editor.cursor_x > 0)
      editor.cursor_x--;
    editor.st_cx = editor.cursor_x;
    break;
  case ARROW_RIGHT:
    if (row && editor.cursor_x < row->size - 1)
      editor.cursor_x++;
    editor.st_cx = editor.cursor_x;
    break;
  case ARROW_UP:
    if (editor.cursor_y > 0)
      editor.cursor_y--;
    break;
  case ARROW_DOWN:
    if (editor.cursor_y < editor.numrows - 1)
      editor.cursor_y++;
    break;
  }

  // cursor state preservation
  if (editor.cursor_y < editor.numrows) {
    row = &editor.row[editor.cursor_y];
    editor.cursor_x = editor.st_cx;
    if (editor.cursor_x >= row->size)
      editor.cursor_x = row->size == 0 ? 0 : row->size - 1;
  }
}

void editorScroll() {
  int _cy = editor.cursor_y - editor.offset_y;

  // vertical scroll handler
  if (_cy > (editor.screen_rows - 1 - settings.scrollpadding))
    editor.offset_y =
        editor.cursor_y - editor.screen_rows + settings.scrollpadding + 1;
  else if (_cy < settings.scrollpadding) {
    editor.offset_y = editor.cursor_y - settings.scrollpadding;
    if (editor.offset_y < 0)
      editor.offset_y = 0;
  }

  // horizontal scroll handler
  editor.render_x =
      editor.cursor_y < editor.numrows
          ? rowCursorToRenderX(&editor.row[editor.cursor_y], editor.cursor_x)
          : 0;
  int _cx = editor.render_x - editor.offset_x;
  if (_cx >= editor.screen_cols - 1)
    editor.offset_x = editor.render_x - editor.screen_cols + 1;
  else if (_cx < 0)
    editor.offset_x = editor.render_x;
}

void editorInsertRow(int index, char *s, size_t len) {
  if(index < 0 || index > editor.numrows) return;

  editor.row = realloc(editor.row, sizeof(erow) * (editor.numrows + 1));
  memmove(&editor.row[index+1], &editor.row[index], (editor.numrows-index) * sizeof(erow));

  erow *_rw = &editor.row[index];
  _rw->size = len;
  _rw->chars = malloc(len + 1);
  memcpy(_rw->chars, s, len);
  _rw->chars[len] = '\0';
  _rw->rsize = 0;
  _rw->renderchars = NULL;
  rowUpdate(_rw);
  editor.numrows++;
  editor.dirty++;
}

void editorDeleteRow(int index){
  if(index < 0 || index >= editor.numrows) return;
  rowFree(&editor.row[index]);
  memmove(&editor.row[index], &editor.row[index+1], (editor.numrows-index-1)*sizeof(erow));
  editor.numrows--;
  editor.dirty++;
}

void editorOpen(char *filename) {
  free(editor.filename);
  editor.filename = strdup(filename);

  FILE *fp = fopen(editor.filename, "r");
  if (!fp)
    die("failed to open file");

  char *line = NULL;
  size_t lcap = 0;
  ssize_t llen;

  while ((llen = getline(&line, &lcap, fp)) != -1) {
    while (llen > 0 && (line[llen - 1] == '\n' || line[llen - 1] == '\r'))
      llen--;
    editorInsertRow(editor.numrows, line, llen);
  }
  free(line);
  fclose(fp);
  editor.dirty = 0;
}

// -- editor operations --

void editorInsertChar(int ch) {
  if (editor.cursor_y == editor.numrows) {
    editorInsertRow(editor.numrows, "", 0);
  }
  rowInsertCharacter(&editor.row[editor.cursor_y], editor.cursor_x, ch);
  editor.cursor_x++;
  editor.st_cx = editor.cursor_x;
  editor.dirty++;
}

void editorInsertNewLine(){
  if(editor.cursor_x==0) editorInsertRow(editor.cursor_y, "", 0);
  else{
    erow *row = &editor.row[editor.cursor_y];
    editorInsertRow(editor.cursor_y+1, &row->chars[editor.cursor_x], row->size-editor.cursor_x);
    row = &editor.row[editor.cursor_y];
    row->size = editor.cursor_x;
    row->chars[row->size] = '\0';
    rowUpdate(row);
  }
  editor.cursor_y ++;
  editor.cursor_x = 0;
  editor.st_cx = 0;
  editor.dirty++;
}

void editorDelChar(int dir){
  if(editor.cursor_y >= editor.numrows) return;

  erow *row = &editor.row[editor.cursor_y];
  int _ps = editor.cursor_x + dir;
  if(_ps>=0) {
    rowDeleteCharacter(row, _ps);
    if(dir<0 || editor.cursor_x == row->size) editor.cursor_x--;
  } else {
    if(editor.cursor_x == 0 && editor.cursor_y == 0) return;
    editor.cursor_x = editor.row[editor.cursor_y-1].size;
    rowAppendString(&editor.row[editor.cursor_y-1], row->chars, row->size);
    editorDeleteRow(editor.cursor_y);
    editor.cursor_y--;
  }
  editor.st_cx = editor.cursor_x;
  editor.dirty++;
}

void editorSaveBuffer() {
  if (editor.filename == NULL){
    if((editor.filename = editorPrompt("Save as: %s")) == NULL){
      editorSetStatusMsg("Save Aborted...");
      return;
    }
  }

  int len;
  char *buf = editorRowtoStr(&len);

  int fd = open(
      editor.filename, O_RDWR | O_CREAT,
      0644); // 0644 is set of permissions for file, added while creating it
  if (fd != -1) {
    if (ftruncate(fd, len) != -1) { // set file size to specified length
      write(fd, buf, len);
      free(buf);
      close(fd);
      editor.dirty = 0;
      editorSetStatusMsg("%d bytes written to disk", len);
      return;
    }
    close(fd);
  }
  free(buf);
  editorSetStatusMsg("Failed to write file: %s", strerror(errno));
}

char *editorRowtoStr(int *buflen) {
  int tlen = 0;
  for (int j = 0; j < editor.numrows; j++) {
    tlen += editor.row[j].size + 1;
  }

  *buflen = tlen;
  char *buf = malloc(tlen);
  for (int j = 0, ind = 0; j < editor.numrows; j++) {
    memcpy(&buf[ind], editor.row[j].chars, editor.row[j].size);
    ind += editor.row[j].size;
    buf[ind++] = '\n';
  }
  return buf;
}

char *editorPrompt(char *prompt){
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
      free(buf);
      return NULL;
    }else if(c == BACKSPACE || c == CTRL_H || c == DEL_KEY){
      if(buflen != 0) buf[--buflen] = '\0';
    } else if(c == RETURN){
      editorSetStatusMsg("");
      return buf;
    }else if(!iscntrl(c) && c <128){
      if(buflen == bufsize-1){
        bufsize*=2;
        buf = realloc(buf, bufsize);
      }
      buf[buflen++] = c;
      buf[buflen] = '\0';
    }
  }
}
