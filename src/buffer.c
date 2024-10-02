#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "editor.h"
#include "settings.h"

extern settings_t settings;
extern syntaxhl HLDB[];
extern int HLDB_SIZE;

void initBuffer(buffer_t *buf) {
  *buf = (buffer_t){
      .row_size = 0,
      .dirty = 0,
      .rows = NULL,
      .filename = NULL,
      .syntax = NULL,
      .selection = {DEF_VEC2, DEF_VEC2, FORWARD},
  };
}

void freeBuffer(buffer_t *buf){
  for(int i=0; i<buf->row_size; i++) rowFree(&buf->rows[i]);
  free(buf->filename);
}

void bufferUpdateSelection(buffer_t *buf, vec2 cursor, int mode, int flags){
  if(!flags){
    buf->selection.start = cursor;
    buf->selection.end = cursor;
    buf->selection.dir = FORWARD;
  } else {
    int cst = checkPoint(buf->selection, cursor);
    if(buf->selection.dir == FORWARD){
      if(cst == BEFORE){
        buf->selection.dir = BACKWARD;
        buf->selection.end = buf->selection.start;
        buf->selection.start = cursor;
      }else buf->selection.end = cursor;
    } else if(buf->selection.dir == BACKWARD){
      if(cst == AFTER){
        buf->selection.dir = FORWARD;
        buf->selection.start = buf->selection.end;
        buf->selection.end = cursor;
      } else buf->selection.start = cursor;
    }
  }
  if(mode == VISUAL_LINE){
    buf->selection.start.x = 0;
    buf->selection.end.x = buf->rows[buf->selection.end.y].size-1;
  }
}

void bufferSwapSelCursor(buffer_t *buf, vec2 *cursor){
  if(buf->selection.dir == FORWARD){
    *cursor = buf->selection.start;
    buf->selection.dir = BACKWARD;
  }else{
    *cursor = buf->selection.end;
    buf->selection.dir = FORWARD;
  }
}

void bufferDeleteSelection(buffer_t *buf, vec2 *cursor){
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
  *cursor = buf->selection.start;
  if(cursor->y>=buf->row_size){
    cursor->y = buf->row_size-1;
    cursor->x = buf->rows[cursor->y].size-1;
  } else cursor->x = clamp(cursor->x, 0, buf->rows[cursor->y].size-1);
}

void bufferReplaceSelection(buffer_t *buf, char c){
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

void bufferCommentSelection(buffer_t *buf){
  char *cmnt;
  if(buf->syntax) cmnt = buf->syntax->cmnt_1ls;
  else cmnt = editorPrompt("Comment syntax > %s", NULL);
  if(!cmnt) return;

  int _nc = strlen(cmnt);
  for(int i=buf->selection.start.y; i<=buf->selection.end.y && i<buf->row_size; i++){
    erow *row = &buf->rows[i];
    for(int j=0; j<_nc; j++){
      rowInsertCharacter(row, j, cmnt[j]);
    }
    bufferUpdateRow(buf, row);
  }
}

int bufferWordJump(buffer_t *buf, vec2 *cursor, int flags){
  int dir = flags & JMP_BACK? -1: 1;
  vec2 crs = *cursor, lnsp = *cursor;

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
          if(!vec2areSame(lnsp, *cursor)) {
            crs = lnsp;
            _fnd++;
            break;
          } else if(c != ' ' && !vec2areSame(crs, *cursor)) {
            _fnd++;
            break;
          } } else lnsp = crs;
      } else {
        if(_sep) _fnd++;
        if((_lj || _fnd) && c != ' ' && !vec2areSame(*cursor, crs)) break;
      }

      crs.x += dir;
    }
    crs.x = clamp(crs.x, 0, row->size-1);

    if(_lj || (_fnd && !vec2areSame(*cursor, crs))) break;
    else _lj++;
    if((flags&JMP_END) && !vec2areSame(crs, *cursor)) break;

    crs.y += dir;
  }

  int _nomore = 1; // if we went out of bounds, not a successful cmd invoc.
  if(crs.y<0) crs.y=0;
  else if(crs.y>=buf->row_size) crs.y=buf->row_size-1;
  else _nomore = 0;

  crs.x = clamp(crs.x, 0, buf->rows[crs.y].size-1);
  *cursor = crs;
  return !_nomore;
}

int bufferFindChar(buffer_t *buf, vec2 *cursor, char char_, int dirflg){
  int dir = dirflg&JMP_BACK?-1:1;
  vec2 crs = *cursor;
  if(crs.y<0 || crs.y>=buf->row_size) return 0;

  erow *row = &buf->rows[crs.y];
  crs.x += dir; // start  from next

  char c;
  for(;crs.x<row->size && crs.x>=0; crs.x+=dir){
    c = row->chars[crs.x];
    if(c == char_){
      *cursor = crs;
      return 1;
    }
  }
  return 0;
}

int bufferParaNav(buffer_t *buf, vec2 *cursor, int times, int dirflag){
  int dir = dirflag&JMP_BACK?-1:1;
  int y=cursor->y+dir;
  for(; y>0 && y<buf->row_size;y+=dir){
    if(buf->rows[y].size == 0 && !(--times)){
      cursor->x=0;
      cursor->y=y;
      return 1;
    }
  }
  cursor->y = clamp(y, 0, buf->row_size-1);
  cursor->x = dir>0?(buf->rows[cursor->y].size-1):0;
  return 0;
}

void bufferReplaceChar(buffer_t *buf, vec2 *cursor, char char_, int repx){
  if(cursor->y<0 || cursor->y>=buf->row_size) return;
  erow *row = &buf->rows[cursor->y];
  if(rowReplaceCharacter(row, char_, cursor->x, repx))
    bufferUpdateRow(buf, row);
}

void bufferUpdateRow(buffer_t *buf, erow *row) {
  rowUpdate(row);
  rowUpdateSyntax(row, buf->syntax);
}

void bufferInsertRow(buffer_t *buf, int index, char *s, size_t len) {
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

void bufferDeleteRows(buffer_t *buf, int index, int len) {
  if (index < 0 || index >= buf->row_size)
    return;
  if(index+len > buf->row_size) len = buf->row_size-index;
  if(len == 0) return;

  for(int i=index; i<index+len; i++) rowFree(&buf->rows[i]);

  memmove(&buf->rows[index], &buf->rows[index + len],
          (buf->row_size - index - len) * sizeof(erow));
  buf->row_size-=len;
  buf->dirty++;
}

void bufferInsertText(buffer_t *buf, vec2 *cursor, char *text){
  if(!text || !buf || !cursor) return;

  if(!buf->row_size) bufferInsertRow(buf, 0, "", 0);
  cursor->y = clamp(cursor->y, 0, buf->row_size-1);
  cursor->x = clamp(cursor->x, 0, buf->rows[cursor->y].size-1);

  char *line = text, *lp = text;
  erow* _row;
  for(;*lp;lp++){
    if(*lp == '\n'){
      bufferInsertNewLine(buf, cursor);
      _row = &buf->rows[cursor->y-1];
      rowAppendString(_row, line, lp-line);
      bufferUpdateRow(buf, _row);
      line = lp+1;
    }
  }
  if(line<lp) {
    _row = &buf->rows[cursor->y];
    rowInsertString(_row, cursor->x, line, lp-line);
    bufferUpdateRow(buf, _row);
  }
}

int bufferOpenFile(buffer_t *buf, char *filename) {
  free(buf->filename);

  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fp = fopen(filename, "w");
    if(!fp) return 0;
  }

  buf->filename = strdup(filename);
  bufferSelectSyntax(buf);

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
  return 1;
}

void bufferInsertChar(buffer_t *buf, vec2 *cursor, int ch) {
  if (cursor->y == buf->row_size) {
    bufferInsertRow(buf, buf->row_size, "", 0);
  }
  rowInsertCharacter(&buf->rows[cursor->y], cursor->x, ch);
  bufferUpdateRow(buf, &buf->rows[cursor->y]);
  cursor->x++;
  buf->dirty++;
}

void bufferInsertNewLine(buffer_t *buf, vec2 *cursor) {
  if (cursor->x == 0){
    bufferInsertRow(buf, cursor->y++, "", 0);
    cursor->x = 0;
  } else {
    erow *row = &buf->rows[cursor->y];
    bufferInsertRow(buf, cursor->y+1, &row->chars[cursor->x],
                    row->size - cursor->x);
    row = &buf->rows[cursor->y++];
    row->size = cursor->x;
    row->chars[row->size] = '\0';
    bufferUpdateRow(buf, row);

    int _tcount = countTabs(row->chars);
    row = &buf->rows[cursor->y];
    cursor->x = 0;
    while(_tcount--) bufferInsertChar(buf, cursor, '\t');
    bufferUpdateRow(buf, row);
  }
  buf->dirty++;
}

void bufferSwapRow(buffer_t *buf, int index1, int index2){
  if(index1 == index2) return;
  if(index1<0 || index1>=buf->row_size) return;
  if(index2<0 || index2>=buf->row_size) return;

  erow _tr = buf->rows[index1];
  buf->rows[index1] = buf->rows[index2];
  buf->rows[index2] = _tr;
}

void bufferDelChar(buffer_t *buf, vec2 *cursor, int dir){
  if (cursor->y >= buf->row_size)
    return;

  erow *row = &buf->rows[cursor->y];
  int _ps = cursor->x + dir;
  if (_ps >= 0 && _ps<row->size) {
    rowDeleteCharacters(row, _ps, 1);
    if (dir < 0) cursor->x--;
  } else if(_ps == row->size){
    if(cursor->y+1 == buf->row_size) return;
    erow *_nrow = &buf->rows[cursor->y+1];
    rowAppendString(row, _nrow->chars, _nrow->size);
    bufferDeleteRows(buf, cursor->y+1, 1);
  }else {
    if (cursor->x == 0 && cursor->y == 0)
      return;
    cursor->x = buf->rows[cursor->y - 1].size;
    rowAppendString(&buf->rows[cursor->y - 1], row->chars, row->size);
    bufferDeleteRows(buf, cursor->y, 1);
    cursor->y--;
  }
  row = &buf->rows[cursor->y];
  bufferUpdateRow(buf, row);
  buf->dirty++;
}

char *bufferRowtoStr(buffer_t *buf, int *buflen) {
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

int bufferSave(buffer_t *buf) {
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

int bufferFind(buffer_t *buf, char *query, vec2 *cursor, int dir) {
  if(!query || !strlen(query)) return 0;
  int _d = dir == 0 ? 1 : dir;
  for (int i = 0, crs = cursor->y; i <= buf->row_size; i++, crs += _d) {
    if (crs == -1)
      crs += buf->row_size;
    else if (crs == buf->row_size)
      crs = 0;

    erow *row = &buf->rows[crs];
    char *match = strstr(row->renderchars, query);
    if (match) {
      int _cx = rowRendertoCursorX(row, match - row->renderchars);
      if (i == 0) {
        if (dir == 1 && _cx <= cursor->x)
          continue;
        else if (dir == 0 && _cx < cursor->x)
          continue;
        else if (dir == -1 && _cx >= cursor->x)
          continue;
      }

      cursor->y = crs;
      cursor->x = _cx;
      return 1;
    }
  }
  return 0;
}

void bufferSelectSyntax(buffer_t *buf) {
  buf->syntax = NULL;
  if (buf->filename == NULL)
    return;

  char *_extn = strrchr(buf->filename, '.');

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
