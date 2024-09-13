#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "buffer.h"
#include "keybinds.h"
#include "settings.h"

extern settingsType settings;

void initBuffer(buffer *buf) {
  *buf = (buffer){
      .cursor = DEF_VEC2,
      .offset = DEF_VEC2,
      .size = DEF_VEC2,
      .row_size = 0,
      .render_x = 0,
      .st_cx = 0,
      .dirty = 0,
      .rows = NULL,
      .filename = NULL,
  };
}

void addWelcomeMsg(buffer *buf, abuf *ab) {
  char wlc[80];
  int wlclen =
      snprintf(wlc, sizeof(wlc), "therealtxt editor v %s", EDITOR_VERSION);
  if (wlclen > buf->size.x)
    wlclen = buf->size.x;

  int padding = (buf->size.x - wlclen) / 2;
  if (padding) {
    abAppend(ab, "~", 1);
    padding--;
  }
  while (padding--) {
    abAppend(ab, " ", 1);
  }
  abAppend(ab, wlc, wlclen);
}

void bufferDrawRows(buffer *buf, abuf *ab){
  for (int y = 0; y < buf->size.y; y++) {
    int _fr = y + buf->offset.y;
    if (_fr >= buf->row_size) {
      if (buf->row_size == 0 && y == buf->size.y / 3)
        addWelcomeMsg(buf, ab);
      else
        abAppend(ab, "~", 1);
    } else {
      int len = buf->rows[_fr].rsize - buf->offset.x;
      if (len < 0)
        len = 0;
      if (len > buf->size.x)
        len = buf->size.x;
      abAppend(ab, &buf->rows[_fr].renderchars[buf->offset.x], len);
    }
    abAppend(ab, "\x1b[K", 3); // clear the line before drawing
    abAppend(ab, "\r\n", 2);
  }
}

void bufferDrawStatusBar(buffer *buf, abuf *ab){
  abAppend(ab, "\x1b[7m", 4);

  char lstatus[80], rstatus[80];
  int len = snprintf(lstatus, sizeof(lstatus), "%.20s %.3s",
                     buf->filename ? buf->filename : "[No name]",
                     buf->dirty ? "[+]" : "");
  if (len > buf->size.x)
    len = buf->size.x;

  int lsz = buf->cursor.y<buf->row_size? buf->rows[buf->cursor.y].size:0;
  int rlen = snprintf(rstatus, sizeof(rstatus), "%d, %d|%d", buf->cursor.y,
                      buf->cursor.x, lsz);

  abAppend(ab, lstatus, len);
  for (; len < buf->size.x - rlen; len++)
    abAppend(ab, " ", 1);
  abAppend(ab, rstatus, rlen);

  abAppend(ab, "\r\n", 2);
  abAppend(ab, "\x1b[m", 3);
}

void bufferShowCursor(buffer *buf, abuf *ab){
  char _cbuf[32];
  snprintf(_cbuf, sizeof(_cbuf), "\x1b[%d;%dH",
           buf->cursor.y - buf->offset.y + 1,
           buf->render_x - buf->offset.x + 1);
  abAppend(ab, _cbuf, strlen(_cbuf));
  abAppend(ab, "\x1b[?25h", 6); // show the cursor
}

void bufferMoveCursor(buffer *buf, int key){
  erow *row = buf->cursor.y >= buf->row_size ?
      NULL : &buf->rows[buf->cursor.y];

  switch (key) {
  case ARROW_LEFT:
    if (buf->cursor.x > 0)
      buf->cursor.x--;
    buf->st_cx = buf->cursor.x;
    break;
  case ARROW_RIGHT:
    if (row && buf->cursor.x < row->size - 1)
      buf->cursor.x++;
    buf->st_cx = buf->cursor.x;
    break;
  case ARROW_UP:
    if (buf->cursor.y > 0)
      buf->cursor.y--;
    break;
  case ARROW_DOWN:
    if (buf->cursor.y < buf->row_size - 1)
      buf->cursor.y++;
    break;
  }

  // cursor state preservation
  if (buf->cursor.y < buf->row_size) {
    row = &buf->rows[buf->cursor.y];
    buf->cursor.x = buf->st_cx;
    if (buf->cursor.x >= row->size)
      buf->cursor.x = row->size == 0 ? 0 : row->size - 1;
  }
}

void bufferScroll(buffer *buf){
  int _cy = buf->cursor.y - buf->offset.y;

  // vertical scroll handler
  if (_cy > (buf->size.y - 1 - settings.scrollpadding))
    buf->offset.y =
        buf->cursor.y - buf->size.y + settings.scrollpadding + 1;
  else if (_cy < settings.scrollpadding) {
    buf->offset.y = buf->cursor.y - settings.scrollpadding;
    if (buf->offset.y < 0)
      buf->offset.y = 0;
  }

  // horizontal scroll handler
  buf->render_x =
      buf->cursor.y < buf->row_size
          ? rowCursorToRenderX(&buf->rows[buf->cursor.y], buf->cursor.x)
          : 0;
  int _cx = buf->render_x - buf->offset.x;
  if (_cx >= buf->size.x - 1)
    buf->offset.x = buf->render_x - buf->size.x + 1;
  else if (_cx < 0)
    buf->offset.x = buf->render_x;
}

void bufferInsertRow(buffer *buf, int index, char *s, size_t len){
  if(index < 0 || index > buf->row_size) return;

  buf->rows = realloc(buf->rows, sizeof(erow) * (buf->row_size + 1));
  memmove(&buf->rows[index+1], &buf->rows[index], (buf->row_size-index) * sizeof(erow));

  erow *_rw = &buf->rows[index];
  _rw->size = len;
  _rw->chars = malloc(len + 1);
  memcpy(_rw->chars, s, len);
  _rw->chars[len] = '\0';
  _rw->rsize = 0;
  _rw->renderchars = NULL;
  rowUpdate(_rw);
  buf->row_size++;
  buf->dirty++;
}

void bufferDeleteRow(buffer *buf, int index){
  if(index < 0 || index >= buf->row_size) return;
  rowFree(&buf->rows[index]);
  memmove(&buf->rows[index], &buf->rows[index+1], (buf->row_size-index-1)*sizeof(erow));
  buf->row_size--;
  buf->dirty++;
}

void bufferOpenFile(buffer *buf, char *filename){
  free(buf->filename);
  buf->filename = strdup(filename);

  FILE *fp = fopen(buf->filename, "r");
  if (!fp)
    die("failed to open file");

  char *line = NULL;
  size_t lcap = 0;
  ssize_t llen;

  while ((llen = getline(&line, &lcap, fp)) != -1) {
    while (llen > 0 && (line[llen - 1] == '\n' || line[llen - 1] == '\r'))
      llen--;
    bufferInsertRow(buf, buf->row_size, line, llen);
  }
  free(line);
  fclose(fp);
  buf->dirty = 0;
}

void bufferInsertChar(buffer* buf, int ch){
  if (buf->cursor.y == buf->row_size) {
    bufferInsertRow(buf, buf->row_size, "", 0);
  }
  rowInsertCharacter(&buf->rows[buf->cursor.y], buf->cursor.x, ch);
  buf->cursor.x++;
  buf->st_cx = buf->cursor.x;
  buf->dirty++;
}

void bufferInsertNewLine(buffer *buf){
  if(buf->cursor.x==0) bufferInsertRow(buf, buf->cursor.y, "", 0);
  else{
    erow *row = &buf->rows[buf->cursor.y];
    bufferInsertRow(buf, buf->cursor.y+1, &row->chars[buf->cursor.x], row->size-buf->cursor.x);
    row = &buf->rows[buf->cursor.y];
    row->size = buf->cursor.x;
    row->chars[row->size] = '\0';
    rowUpdate(row);
  }
  buf->cursor.y ++;
  buf->cursor.x = 0;
  buf->st_cx = 0;
  buf->dirty++;
}

void bufferDelChar(buffer *buf, int dir){
  if(buf->cursor.y >= buf->row_size) return;

  erow *row = &buf->rows[buf->cursor.y];
  int _ps = buf->cursor.x + dir;
  if(_ps>=0) {
    rowDeleteCharacter(row, _ps);
    if(dir<0 || buf->cursor.x == row->size) buf->cursor.x--;
  } else {
    if(buf->cursor.x == 0 && buf->cursor.y == 0) return;
    buf->cursor.x = buf->rows[buf->cursor.y-1].size;
    rowAppendString(&buf->rows[buf->cursor.y-1], row->chars, row->size);
    bufferDeleteRow(buf, buf->cursor.y);
    buf->cursor.y--;
  }
  buf->st_cx = buf->cursor.x;
  buf->dirty++;
}

char *bufferRowtoStr(buffer *buf, int *buflen) {
  int tlen = 0;
  for (int j = 0; j < buf->row_size; j++) {
    tlen += buf->rows[j].size + 1;
  }

  *buflen = tlen;
  char *_cbuf = malloc(tlen);
  for (int j = 0, ind = 0; j < buf->row_size; j++) {
    memcpy(&_cbuf[ind], buf->rows[j].chars, buf->rows[j].size);
    ind += buf->rows[j].size;
    _cbuf[ind++] = '\n';
  }
  return _cbuf;
}

int bufferSave(buffer *buf){
  // buffer should have filename
  if (!buf->filename) return -1;

  int len;
  char *_cbuf = bufferRowtoStr(buf, &len);

  int fd = open(
      buf->filename, O_RDWR | O_CREAT,
      0644); // 0644 is set of permissions for file, added while creating it
  if (fd != -1) {
    if (ftruncate(fd, len) != -1) { // set file size to specified length
      write(fd, _cbuf, len);
      free(_cbuf);
      close(fd);
      buf->dirty = 0;
      return len;
    }
    close(fd);
  }
  free(_cbuf);
  return -1;
}

void bufferFind(buffer *buf, char *query){
  for(int i=0; i<buf->row_size; i++){
    erow *row = &buf->rows[i];
    char *match = strstr(row->renderchars, query);
    if(match){
      buf->cursor.y = i;
      buf->cursor.x = rowRendertoCursorX(row, match - row->renderchars);
      //buf->offset.y = buf->row_size-1;
    }
  }
}
