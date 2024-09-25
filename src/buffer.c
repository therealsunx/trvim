#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "keybinds.h"
#include "settings.h"

extern settingsType settings;
extern syntaxhl HLDB[];
extern int HLDB_SIZE;

void initBuffer(buffer *buf) {
  *buf = (buffer){
      .cursor = DEF_VEC2,
      .offset = DEF_VEC2,
      .view_size = DEF_VEC2,
      .size = DEF_VEC2,
      .row_size = 0,
      .render_x = 0,
      .linenumcol_sz = 1,
      .dirty = 0,
      .rows = NULL,
      .filename = NULL,
      .st = DEF_STATE,
      .syntax = NULL,
      .selection = {DEF_VEC2, DEF_VEC2, FORWARD},
  };
}

void freeBuffer(buffer *buf){
  for(int i=0; i<buf->row_size; i++){
    free(buf->rows[i].chars);
  }
  free(buf->filename);
}

void bufferUpdateLineColSz(buffer *buf){
  int _sz = buf->row_size;
  buf->linenumcol_sz = 3; // one bar, one space and at least one char
  while ((_sz /= 10))
    buf->linenumcol_sz++;
  buf->size.x = buf->view_size.x-buf->linenumcol_sz;
}

void bufferUpdateSize(buffer *buf, int sx, int sy) {
  buf->view_size.y = sy;
  buf->view_size.x = sx;
  buf->size.y = sy;
  bufferUpdateLineColSz(buf);
}

void addWelcomeMsg(buffer *buf, abuf *ab) {
  char wlc[80];
  int wlclen =
      snprintf(wlc, sizeof(wlc), "therealtxt editor v%s", EDITOR_VERSION);
  if (wlclen > buf->view_size.x)
    wlclen = buf->view_size.x;

  int padding = (buf->view_size.x - wlclen) / 2;
  while (padding--) abAppend(ab, " ", 1);
  abAppend(ab, wlc, wlclen);
}

void addColumn(buffer *buf, abuf *ab, int linenum){
  char _lstr[32];
  int val = linenum+1, len;

  int _nsz = buf->linenumcol_sz-2; // one bar & one col
  abAppend(ab, "\x1b[48;5;237m \x1b[m\x1b[90m", 20);

  if(linenum < buf->row_size){
    if(settings.flags&REL_LINENUM && linenum!=buf->cursor.y){
      val -= buf->cursor.y+1;
      if(val<0) val=-val;
    }
    len = linenum==buf->cursor.y ?
      snprintf(_lstr, sizeof(_lstr), "\x1b[33m%-*d ", _nsz, val):
      snprintf(_lstr, sizeof(_lstr), "%*d ", _nsz, val);
  } else {
    len = snprintf(_lstr, sizeof(_lstr), "%*s ", _nsz, "~");
  }
  abAppend(ab, _lstr, len);
  abAppend(ab, "\x1b[m", 3);
}

void bufferDrawRows(buffer *buf, abuf *ab, int selflag) {
  vec2 sstate = {BEFORE, BEFORE}; // for selection mode
  boundstype sel = buf->selection;
  if(selflag){
    sel.end.x = rowCursorToRenderX(&buf->rows[sel.end.y], sel.end.x);
    sel.start.x = rowCursorToRenderX(&buf->rows[sel.start.y], sel.start.x);
  }

  for (int y = 0; y < buf->view_size.y; y++) {
    int _fr = y + buf->offset.y;

    // --- line number column ---
    addColumn(buf, ab, _fr);

    if (_fr >= buf->row_size) {
      if (buf->row_size == 0 && y == buf->view_size.y / 3)
        addWelcomeMsg(buf, ab);
    } else {

      // render according to metadata
      int len = clamp(buf->rows[_fr].rsize - buf->offset.x, 0, buf->size.x);

      char *ch = &buf->rows[_fr].renderchars[buf->offset.x];
      unsigned char *hl = &buf->rows[_fr].hlchars[buf->offset.x];
      int _ptk = TK_NORMAL;

      // selection mode processing
      if(selflag){
        if(sstate.y == BEFORE){
          if(_fr>=sel.start.y) sstate.y = INBOUND;
        } else if(sstate.y == INBOUND){
          if(_fr > sel.end.y) sstate.y = AFTER;
        } // no need to process after AFTER
      }

      for (int i = 0; i < len; i++) {

        // token highlighing
        int _clr = hlTokentoColorIndex(hl[i]);
        if (hl[i] != _ptk) {
          if (_ptk == TK_MATCH || _ptk == TK_KEYWORD3)
            abAppend(ab, "\x1b[m", 3);

          char _tcstr[24];
          int _cl;
          switch (hl[i]) {
            case TK_MATCH:
              _cl = snprintf(_tcstr, sizeof(_tcstr), "\x1b[48;5;%dm\x1b[38;5;%dm", _clr, 0);
              break;
            case TK_KEYWORD3:
              _cl = snprintf(_tcstr, sizeof(_tcstr), "\x1b[1m\x1b[38;5;%dm", _clr);
              break;
            default:
              _cl = snprintf(_tcstr, sizeof(_tcstr), "\x1b[38;5;%dm", _clr);
          }

          abAppend(ab, _tcstr, _cl);
          _ptk = hl[i];
        }

        // selection mode highlighting
        if(sstate.y == INBOUND){
          if(sstate.x == BEFORE){
            if(i>=sel.start.x){
              sstate.x = INBOUND;
              abAppend(ab, "\x1b[100m", 6);
            }
          } else if(sstate.x == INBOUND){
            if(_fr == sel.end.y && i>sel.end.x){
              sstate.x = AFTER;
              abAppend(ab, "\x1b[49m", 5);
            }
          } // no need to process after AFTER

          if(i==0 && sstate.x == INBOUND){
            abAppend(ab, "\x1b[100m", 6);
          }
        }

        abAppend(ab, &ch[i], 1);
      }
    }
    abAppend(ab, "\x1b[m\x1b[K", 6); // clear the line before drawing
    abAppend(ab, "\r\n", 2);
  }
}

void bufferUpdateSelection(buffer *buf, int mode, int flags){
  if(!flags){
    buf->selection.start = buf->cursor;
    buf->selection.end = buf->cursor;
    buf->selection.dir = FORWARD;
  } else {
    int cst = checkPoint(buf->selection, buf->cursor);
    if(buf->selection.dir == FORWARD){
      if(cst == BEFORE){
        buf->selection.dir = BACKWARD;
        buf->selection.end = buf->selection.start;
        buf->selection.start = buf->cursor;
      }else buf->selection.end = buf->cursor;
    } else if(buf->selection.dir == BACKWARD){
      if(cst == AFTER){
        buf->selection.dir = FORWARD;
        buf->selection.start = buf->selection.end;
        buf->selection.end = buf->cursor;
      } else buf->selection.start = buf->cursor;
    }
  }

  if(mode == VISUAL_LINE){
    buf->selection.start.x = 0;
    buf->selection.end.x = buf->rows[buf->selection.end.y].size-1;
  }
}

void bufferSwapSelCursor(buffer *buf){
  if(buf->selection.dir == FORWARD){
    buf->cursor = buf->selection.start;
    buf->selection.dir = BACKWARD;
  }else{
    buf->cursor = buf->selection.end;
    buf->selection.dir = FORWARD;
  }
}

void bufferDeleteSelection(buffer *buf){
  // delete from last to first to avoid memory misplace issues
  int y = buf->selection.end.y, x;
  erow *row;
  for(;y>=buf->selection.start.y; y--){
    row = &buf->rows[y];
    if(y == buf->selection.end.y){
      x = y == buf->selection.start.y?buf->selection.start.x:0;
      rowDeleteCharacters(row, x, buf->selection.end.x-x+1);
      if(row->size == 0) bufferDeleteRows(buf, y, 1);
      else bufferUpdateRow(buf, row);
    } else if (y == buf->selection.start.y){
      x = buf->selection.start.x;
      rowDeleteCharacters(row, x, row->size-x+1);
      if(row->size == 0) bufferDeleteRows(buf, y, 1);
      else bufferUpdateRow(buf, row);
    } else {
      bufferDeleteRows(buf, y, 1);
    }
  }
  buf->cursor = buf->selection.start;
  if(buf->cursor.y>=buf->row_size){
    buf->cursor.y = buf->row_size-1;
    buf->cursor.x = buf->rows[buf->cursor.y].size-1;
  } else buf->cursor.x = clamp(buf->cursor.x, 0, buf->rows[buf->cursor.y].size-1);
  buf->st.cursx = buf->cursor.x;
}

void bufferReplaceSelection(buffer *buf, char c){
  int y = buf->selection.end.y, x;
  erow *row;
  for(;y>=buf->selection.start.y; y--){
    row = &buf->rows[y];
    if(y == buf->selection.end.y){
      x = y == buf->selection.start.y?buf->selection.start.x:0;
      rowReplaceCharacter(row, c, x, buf->selection.end.x-x+1);
      bufferUpdateRow(buf, row);
    } else if (y == buf->selection.start.y){
      x = buf->selection.start.x;
      rowReplaceCharacter(row, c, x, row->size-x+1);
      bufferUpdateRow(buf, row);
    } else {
      rowReplaceCharacter(row, c, 0, row->size);
      bufferUpdateRow(buf, row);
    }
  }
}

void bufferDrawStatusBar(buffer *buf, abuf *ab) {
  abAppend(ab, "\x1b[48;5;237m", 11);

  char lstatus[80], rstatus[80];
  int len = snprintf(lstatus, sizeof(lstatus), "%.20s [%s] %.3s",
                     buf->filename ? buf->filename : "[No name]",
                     buf->syntax ? buf->syntax->filetype : "nt",
                     buf->dirty ? "[+]" : "");
  if (len > buf->view_size.x)
    len = buf->view_size.x;

  int rlen = snprintf(rstatus, sizeof(rstatus), "%d|%d, %d", buf->cursor.y,
                      buf->row_size, buf->cursor.x);

  abAppend(ab, lstatus, len);
  for (; len < buf->view_size.x - rlen; len++)
    abAppend(ab, " ", 1);
  abAppend(ab, rstatus, rlen);

  abAppend(ab, "\r\n", 2);
  abAppend(ab, "\x1b[m", 3);
}

void bufferShowCursor(buffer *buf) {
  showCursor(
      buf->cursor.y - buf->offset.y + 1,
      buf->render_x - buf->offset.x + buf->linenumcol_sz + 1);
}

void bufferMoveCursor(buffer *buf, int key, int mode, int repx) {
  if(buf->cursor.y>=buf->row_size) buf->cursor.y = buf->row_size-1;
  erow *row = &buf->rows[buf->cursor.y];

  int _rsz = row->size;
  if(mode != INSERT) _rsz--;

  switch (key) {
    case 'h':
    case ARROW_LEFT:
      buf->cursor.x -= repx;
      if(buf->cursor.x<0) buf->cursor.x=0;
      buf->st.cursx = buf->cursor.x;
      return;
    case 'l':
    case ARROW_RIGHT:
      buf->cursor.x += repx;
      if(buf->cursor.x>_rsz) buf->cursor.x=_rsz;
      buf->st.cursx = buf->cursor.x;
      return;
    case 'k':
    case ARROW_UP:
      buf->cursor.y -= repx;
      if(buf->cursor.y<0) buf->cursor.y=0;
      break;
    case 'j':
    case ARROW_DOWN:
      buf->cursor.y += repx;
      if(buf->cursor.y>=buf->row_size) buf->cursor.y=buf->row_size-1;
      break;
  }

  // cursor state preservation
  if (buf->cursor.y < buf->row_size) {
    row = &buf->rows[buf->cursor.y];
    _rsz = row->size;
    if(mode != INSERT) _rsz--;

    buf->cursor.x = buf->st.cursx;
    if (buf->cursor.x >= row->size)
      buf->cursor.x = row->size == 0 ? 0 : _rsz;
  }
}

void bufferAbsoluteJump(buffer *buf, int line){
  line--;
  buf->cursor.y = clamp(line, 0, buf->row_size-1);
  buf->cursor.x = clamp(buf->st.cursx, 0, buf->rows[buf->cursor.y].size-1);
}

void bufferScroll(buffer *buf) {
  int _cy = buf->cursor.y - buf->offset.y;

  // vertical scroll handler
  if (_cy > (buf->view_size.y - 1 - settings.scrollpadding))
    buf->offset.y = buf->cursor.y - buf->view_size.y + settings.scrollpadding + 1;
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
  if (_cx >= buf->view_size.x - buf->linenumcol_sz - 1)
    buf->offset.x = buf->render_x - buf->view_size.x + buf->linenumcol_sz + 1;
  else if (_cx < 0)
    buf->offset.x = buf->render_x;
}

void bufferGotoEnd(buffer* buf, int mode, int posflg){
  if(posflg&JMP_BACK) buf->cursor.x=0;
  else{
    if(buf->cursor.y>=buf->row_size) buf->cursor.y = buf->row_size-1;
    buf->cursor.x = buf->rows[buf->cursor.y].size;
    if(mode != INSERT) buf->cursor.x--;
  }
  buf->st.cursx = buf->cursor.x;
}

int bufferWordJump(buffer *buf, int flags){
  int dir = flags & JMP_BACK? -1: 1;
  vec2 crs = buf->cursor, lnsp = buf->cursor;

  int _fnd = 0;
  int _lj = 0;
  while(crs.y >= 0 && crs.y<buf->row_size){
    erow *row = &buf->rows[crs.y];
    if(_lj) crs.x = dir>0?0:(row->size-1);

    while(crs.x>=0 && crs.x<row->size){
      char c = row->chars[crs.x];
      int _sep = flags&JMP_PUNC?c==' ':isSeparator(c);

      if(flags&JMP_END){
        if(_sep){
          if(!vec2areSame(lnsp, buf->cursor)) {
            crs = lnsp;
            _fnd++;
            break;
          } else if(c != ' ' && !vec2areSame(crs, buf->cursor)) {
            _fnd++;
            break;
          } } else lnsp = crs;
      } else {
        if(_sep) _fnd++;
        if((_lj || _fnd) && c != ' ' && !vec2areSame(buf->cursor, crs)) break;
      }

      crs.x += dir;
    }
    crs.x = clamp(crs.x, 0, row->size-1);

    if(_lj || (_fnd && !vec2areSame(buf->cursor, crs))) break;
    else _lj++;
    if((flags&JMP_END) && !vec2areSame(crs, buf->cursor)) break;

    crs.y += dir;
  }

  int _nomore = 1; // if we went out of bounds, not a successful cmd invoc.
  if(crs.y<0) crs.y=0;
  else if(crs.y>=buf->row_size) crs.y=buf->row_size-1;
  else _nomore = 0;

  crs.x = clamp(crs.x, 0, buf->rows[crs.y].size-1);
  buf->cursor = crs;
  buf->st.cursx = crs.x;
  return !_nomore;
}

int bufferFindChar(buffer *buf, char char_, int dirflg){
  int dir = dirflg&JMP_BACK?-1:1;
  vec2 crs = buf->cursor;
  if(crs.y<0 || crs.y>=buf->row_size) return 0;

  erow *row = &buf->rows[crs.y];
  crs.x += dir; // start  from next

  char c;
  for(;crs.x<row->size && crs.x>=0; crs.x+=dir){
    c = row->chars[crs.x];
    if(c == char_){
      buf->cursor = crs;
      return 1;
    }
  }
  return 0;
}

int bufferParaNav(buffer *buf, int dirflag){
  int dir = dirflag&JMP_BACK?-1:1;
  int y=buf->cursor.y+dir;
  for(; y>0 && y<buf->row_size;y+=dir){
    if(buf->rows[y].size == 0){
      buf->cursor.x=0;
      buf->cursor.y=y;
      return 1;
    }
  }
  buf->cursor.y = clamp(y, 0, buf->row_size-1);
  buf->cursor.x = dir>0?(buf->rows[buf->cursor.y].size-1):0;
  return 0;
}

void bufferReplaceChar(buffer *buf, char char_, int repx){
  if(buf->cursor.y<0 || buf->cursor.y>=buf->row_size) return;
  erow *row = &buf->rows[buf->cursor.y];
  if(rowReplaceCharacter(row, char_, buf->cursor.x, repx))
    bufferUpdateRow(buf, row);
}

void bufferPageScroll(buffer *buf, int key){
  if (key == PAGE_UP)
    buf->cursor.y = buf->offset.y + settings.scrollpadding;
  else {
    buf->cursor.y =
      buf->offset.y + buf->size.y - 1 - settings.scrollpadding;
    if (buf->cursor.y >= buf->row_size)
      buf->cursor.y = buf->row_size - 1;
  }
  int _times = buf->size.y;
  bufferMoveCursor(buf, key == PAGE_UP ? ARROW_UP : ARROW_DOWN, NORMAL, _times);
}

void bufferUpdateRow(buffer *buf, erow *row) {
  rowUpdate(row);
  rowUpdateSyntax(row, buf->syntax);
}

void bufferInsertRow(buffer *buf, int index, char *s, size_t len) {
  if (index < 0 || index > buf->row_size)
    return;

  buf->rows = realloc(buf->rows, sizeof(erow) * (buf->row_size + 1));
  memmove(&buf->rows[index + 1], &buf->rows[index],
          (buf->row_size - index) * sizeof(erow));

  erow *_rw = &buf->rows[index];
  _rw->size = len;
  _rw->chars = malloc(len + 1);
  memcpy(_rw->chars, s, len);
  _rw->chars[len] = '\0';
  _rw->rsize = 0;
  _rw->renderchars = NULL;
  _rw->hlchars = NULL;
  bufferUpdateRow(buf, _rw);
  buf->row_size++;
  buf->dirty++;
}

void bufferDeleteRows(buffer *buf, int index, int len) {
  if (index < 0 || index >= buf->row_size)
    return;
  if(index+len > buf->row_size) len = buf->row_size-index;
  if(len == 0) return;

  for(int i=index; i<index+len; i++) rowFree(&buf->rows[i]);

  memmove(&buf->rows[index], &buf->rows[index + len],
          (buf->row_size - index - len) * sizeof(erow));
  buf->row_size-=len;
  buf->dirty++;
  bufferUpdateLineColSz(buf);
}

void bufferOpenFile(buffer *buf, char *filename) {
  free(buf->filename);
  buf->filename = strdup(filename);

  FILE *fp = fopen(buf->filename, "r");
  if (!fp)
    die("failed to open file");

  bufferSelectSyntax(buf);

  char *line = NULL;
  size_t lcap = 0;
  ssize_t llen;

  while ((llen = getline(&line, &lcap, fp)) != -1) {
    while (llen > 0 && (line[llen - 1] == '\n' || line[llen - 1] == '\r'))
      llen--;
    bufferInsertRow(buf, buf->row_size, line, llen);
  }
  bufferUpdateLineColSz(buf);
  free(line);
  fclose(fp);
  buf->dirty = 0;
}

void bufferInsertChar(buffer *buf, int ch) {
  if (buf->cursor.y == buf->row_size) {
    bufferInsertRow(buf, buf->row_size, "", 0);
  }
  rowInsertCharacter(&buf->rows[buf->cursor.y], buf->cursor.x, ch);
  bufferUpdateRow(buf, &buf->rows[buf->cursor.y]);
  buf->cursor.x++;
  buf->st.cursx = buf->cursor.x;
  buf->dirty++;
}

void bufferInsertNewLine(buffer *buf) {
  if (buf->cursor.x == 0){
    bufferInsertRow(buf, buf->cursor.y++, "", 0);
    buf->cursor.x = 0;
  } else {
    erow *row = &buf->rows[buf->cursor.y];
    bufferInsertRow(buf, buf->cursor.y+1, &row->chars[buf->cursor.x],
                    row->size - buf->cursor.x);
    row = &buf->rows[buf->cursor.y++];
    row->size = buf->cursor.x;
    row->chars[row->size] = '\0';
    bufferUpdateRow(buf, row);

    int _tcount = countTabs(row->chars);
    row = &buf->rows[buf->cursor.y];
    buf->cursor.x = 0;
    while(_tcount--) bufferInsertChar(buf, '\t');
    bufferUpdateRow(buf, row);
  }
  buf->st.cursx = buf->cursor.x;
  buf->dirty++;
  bufferUpdateLineColSz(buf);
}

void bufferSwapRow(buffer *buf, int index1, int index2){
  if(index1 == index2) return;
  if(index1<0 || index1>=buf->row_size) return;
  if(index2<0 || index2>=buf->row_size) return;

  erow _tr = buf->rows[index1];
  buf->rows[index1] = buf->rows[index2];
  buf->rows[index2] = _tr;
}

void bufferDelChar(buffer *buf, int dir) {
  if (buf->cursor.y >= buf->row_size)
    return;

  erow *row = &buf->rows[buf->cursor.y];
  int _ps = buf->cursor.x + dir;
  if (_ps >= 0 && _ps<row->size) {
    rowDeleteCharacters(row, _ps, 1);
    if (dir < 0) buf->cursor.x--;
  } else if(_ps == row->size){
    if(buf->cursor.y+1 == buf->row_size) return;
    erow *_nrow = &buf->rows[buf->cursor.y+1];
    rowAppendString(row, _nrow->chars, _nrow->size);
    bufferDeleteRows(buf, buf->cursor.y+1, 1);
  }else {
    if (buf->cursor.x == 0 && buf->cursor.y == 0)
      return;
    buf->cursor.x = buf->rows[buf->cursor.y - 1].size;
    rowAppendString(&buf->rows[buf->cursor.y - 1], row->chars, row->size);
    bufferDeleteRows(buf, buf->cursor.y, 1);
    buf->cursor.y--;
  }
  row = &buf->rows[buf->cursor.y];
  bufferUpdateRow(buf, row);
  buf->st.cursx = buf->cursor.x;
  buf->dirty++;
  bufferUpdateLineColSz(buf);
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

int bufferSave(buffer *buf) {
  // buffer should have filename
  if (!buf->filename)
    return -1;

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

int bufferFind(buffer *buf, char *query, int dir) {

  int _d = dir == 0 ? 1 : dir;
  for (int i = 0, crs = buf->cursor.y; i <= buf->row_size; i++, crs += _d) {

    if (crs == -1)
      crs += buf->row_size;
    else if (crs == buf->row_size)
      crs = 0;

    erow *row = &buf->rows[crs];
    char *match = strstr(row->renderchars, query);
    if (match) {
      int _cx = rowRendertoCursorX(row, match - row->renderchars);
      if (i == 0) {
        if (dir == 1 && _cx <= buf->cursor.x)
          continue;
        else if (dir == 0 && _cx < buf->cursor.x)
          continue;
        else if (dir == -1 && _cx >= buf->cursor.x)
          continue;
      }

      buf->cursor.y = crs;
      buf->cursor.x = _cx;
      return 0;
    }
  }
  return -1;
}

void bufferSelectSyntax(buffer *buf) {
  buf->syntax = NULL;
  if (buf->filename == NULL)
    return;

  char *_extn = strchr(buf->filename, '.');
  for (int j = 0; j < HLDB_SIZE; j++) {
    syntaxhl *s = &HLDB[j];
    int i = 0;
    char *_mtch;
    while ((_mtch = s->filematch[i]) != NULL) {
      int isext = _mtch[0] == '.';
      if ((isext && _extn && !strcmp(_extn, _mtch)) ||
          (!isext && strstr(buf->filename, _mtch))) {
        buf->syntax = s;

        for (int fr = 0; fr < buf->row_size; fr++) {
          rowUpdateSyntax(&buf->rows[fr], buf->syntax);
        }
        return;
      }
      i++;
    }
  }
}
