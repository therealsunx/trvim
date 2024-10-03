#include <stdlib.h>
#include <string.h>

#include "editor.h"
#include "clipboard.h"
#include "view.h"
#include "keybinds.h"
#include "settings.h"

extern editorconf editor;
extern settings_t settings;

void update(view_t *view) {
  if (!editor.window.bufs) return;
  int _sz = windowGetBufOfView(&editor.window, view)->row_size;
  view->lcols = SIGNCOL_SZ + LCOL_GAP; // 1 minimum char
  int c = 0;
  while ((_sz /= 10)) c++; // HEHE
  view->lcols += max(c, 3);
}

void initView(view_t *view, int buf_i, vec2 size, vec2 position) {
  view->buf_i = buf_i;
  view->position = position;
  view->size = size;
  view->cursor = DEF_VEC2;
  view->offset = DEF_VEC2;
  update(view);
}

void viewSetDims(view_t *view, vec2 size, vec2 position) {
  view->position = position;
  view->size = size;
  update(view);
}

void viewSetBuffer(view_t *view, int buf_i) {
  view->buf_i = buf_i;
  view->cursor = DEF_VEC2;
  view->offset = DEF_VEC2;
  update(view);
}

void _addlcol(view_t *view, abuf *ab, int linenum) {
  char _lstr[32];
  int val = linenum + 1, len;
  int _nsz = view->lcols - SIGNCOL_SZ - LCOL_GAP;

  abAppend(ab, "\x1b[48;5;237m \x1b[m\x1b[90m", 20);
  if (linenum < windowGetBufOfView(&editor.window, view)->row_size) {
    if (settings.flags & REL_LINENUM && linenum != view->cursor.y) {
      val -= view->cursor.y + 1;
      if (val < 0)
        val = -val;
    }
    len = linenum == view->cursor.y
              ? snprintf(_lstr, sizeof(_lstr), "\x1b[33m%-*d ", _nsz, val)
              : snprintf(_lstr, sizeof(_lstr), "%*d ", _nsz, val);
  } else {
    len = snprintf(_lstr, sizeof(_lstr), "%*s ", _nsz, "~");
  }
  abAppend(ab, _lstr, len);
  abAppend(ab, "\x1b[m", 3);
}

void viewDraw(view_t *view, int selflag) {
  abuf ab = ABUF_INIT;
  buffer_t *buf = windowGetBufOfView(&editor.window, view);

  vec2 sstate = {BEFORE, BEFORE}; // for selection mode
  boundstype sel = buf->selection;
  if (selflag) {
    sel.end.x = rowCursorToRenderX(&buf->rows[sel.end.y], sel.end.x);
    sel.start.x =
        rowCursorToRenderX(&buf->rows[sel.start.y], sel.start.x);
  }

  for (int y = 0; y < (view->size.y-STATUSBAR_SZ); y++) {
    abPutCursor(&ab, view->position.x, view->position.y+y);

    int _fr = y + view->offset.y;
    _addlcol(view, &ab, _fr);

    if (_fr >= buf->row_size) {
      if (buf->row_size == 0 && y == view->size.y / 3)
        _addWelcomeMsg(view, &ab);
    } else {
      int len = clamp(buf->rows[_fr].rsize-view->offset.x, 0, view->size.x-view->lcols);

      char *ch = &buf->rows[_fr].renderchars[view->offset.x];
      unsigned char *hl = &buf->rows[_fr].hlchars[view->offset.x];
      int _ptk = TK_NORMAL;

      if(selflag){
        if(sstate.y == BEFORE){
          if(_fr>=sel.start.y) sstate.y = INBOUND;
        } else if(sstate.y == INBOUND){
          if(_fr > sel.end.y) sstate.y = AFTER;
        } // no need to process after AFTER
      }

      for(int i=0; i<len; i++){
        int _clr = hlTokentoColorIndex(hl[i]);
        if(hl[i] != _ptk){
          if (_ptk == TK_KEYWORD3) abAppend(&ab, "\x1b[2m", 4);
          if (_ptk == TK_MATCH) {
            if (sstate.y == INBOUND) sstate.x =BEFORE; // this will force recheck of selection bound
            abAppend(&ab, "\x1b[m", 3);
          }

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

          abAppend(&ab, _tcstr, _cl);
          _ptk = hl[i];
        }

        // selection mode highlighting
        if(sstate.y == INBOUND){
          if(sstate.x == BEFORE){
            if(i>=sel.start.x){
              sstate.x = INBOUND;
              abAppend(&ab, "\x1b[48:5:238m", 12);
            }
          } else if(sstate.x == INBOUND){
            if(_fr == sel.end.y && i>sel.end.x){
              sstate.x = AFTER;
              abAppend(&ab, "\x1b[49m", 5);
            }
          } // no need to process after AFTER
          if(i==0 && sstate.x == INBOUND){
            abAppend(&ab, "\x1b[48:5:238m", 12);
          }
        }

        abAppend(&ab, &ch[i], 1);
      }
    }
    abAppend(&ab, "\x1b[m", 3);
  }
  _drawStatusBar(view, &ab);
  writeBuf(&ab);
  abFree(&ab);
}

void viewShowCursor(view_t *view, int mode){
  //viewScrollCursor(view);
  showCursor(view->position.y+view->cursor.y-view->offset.y,
      view->position.x+view->lcols+view->render_cx-view->offset.x);
  if(mode == INSERT) thinCursor();
  else thickCursor();
}

void viewScrollCursor(view_t *view){
  int _cy = view->cursor.y - view->offset.y;
  if(_cy>(view->size.y-settings.scrollpadding-1)){
    view->offset.y = view->cursor.y - view->size.y + settings.scrollpadding + 1;
  } else if(_cy < settings.scrollpadding){
    view->offset.y = view->cursor.y - settings.scrollpadding;
    if(view->offset.y < 0) view->offset.y = 0;
  }

  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  view->render_cx = view->cursor.y < buf->row_size?
    rowCursorToRenderX(&buf->rows[view->cursor.y], view->cursor.x):
    0;
  int _cx = view->render_cx - view->offset.x;
  if(_cx >= view->size.x - view->lcols - 1)
    view->offset.x = view->render_cx - view->size.x + view->lcols + 1;
  else if(_cx < 0)
    view->offset.x = view->render_cx;
}

int viewMasterKeys(view_t *view, int key, int mode){
  switch (key) {
    case HOME_KEY:
      _arrMvmnt(view, '0', 1, mode);
      return ST_SUCCESS;
    case END_KEY:
      _arrMvmnt(view, '$', 1, mode);
      return ST_SUCCESS;
    case PAGE_UP:
    case PAGE_DOWN:
      _pageMvmnt(view, key);
      return ST_SUCCESS;
    case CTRL_Z:
    case CTRL_X:
      exit(0);
      break;
  }
  return ST_NOOP;
}

int viewMvmtCmdHandle(view_t *view, int key, int times, int mode){
  switch(key){
    case 'h':
    case ARROW_LEFT:
      _arrMvmnt(view, ARROW_LEFT, times, mode);
      return ST_SUCCESS;
    case 'j':
    case ARROW_DOWN:
      _arrMvmnt(view, ARROW_DOWN, times, mode);
      return ST_SUCCESS;
    case 'k':
    case ARROW_UP:
      _arrMvmnt(view, ARROW_UP, times, mode);
      return ST_SUCCESS;
    case 'l':
    case ARROW_RIGHT:
      _arrMvmnt(view, ARROW_RIGHT, times, mode);
      return ST_SUCCESS;
    case '$':
      _arrMvmnt(view, '$', times, mode);
      return ST_SUCCESS;
    case '0':
      _arrMvmnt(view, '0', times, mode);
      return ST_SUCCESS;
    case '_':
      _arrMvmnt(view, '_', times, mode);
      return ST_SUCCESS;
    case 'J':
      _absJmp(view, times);
      return ST_SUCCESS;
    case 'w':
      _wrdJmp(view, 0, times);
      return ST_SUCCESS;
    case 'W':
      _wrdJmp(view, JMP_PUNC, times);
      return ST_SUCCESS;
    case 'e':
      _wrdJmp(view, JMP_END, times);
      return ST_SUCCESS;
    case 'E':
      _wrdJmp(view, JMP_END|JMP_PUNC, times);
      return ST_SUCCESS;
    case 'b':
      _wrdJmp(view, JMP_BACK, times);
      return ST_SUCCESS;
    case 'B':
      _wrdJmp(view, JMP_BACK|JMP_PUNC, times);
      return ST_SUCCESS;
    case 'G':
    case 'H':
    case 'L':
      _miscJmp(view, key);
      return ST_SUCCESS;
  }
  return ST_NOOP;
}

int viewInsCmdHandle(view_t *view, int key){
  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  switch(key){
    case 'i':
      if(!buf->row_size) bufferInsertRow(buf, 0, "", 0);
      break;
    case 'I':
      if(!buf->row_size) bufferInsertRow(buf, 0, "", 0);
      else _arrMvmnt(view, '_', 1, INSERT);
      break;
    case 'a':
      if(!buf->row_size) bufferInsertRow(buf, 0, "", 0);
      else _arrMvmnt(view, ARROW_RIGHT, 1, INSERT);
      break;
    case 'A':
      if(!buf->row_size) bufferInsertRow(buf, 0, "", 0);
      else _arrMvmnt(view, '$', 1, INSERT);
      break;
    case 'o':
      if(!buf->row_size) bufferInsertRow(buf, 0, "", 0);
      _arrMvmnt(view, '$', 1, INSERT);
      bufferInsertNewLine(buf, &view->cursor);
      break;
    case 'O':
      if(!buf->row_size) bufferInsertRow(buf, 0, "", 0);
      _arrMvmnt(view, '$', 1, INSERT);
      bufferInsertNewLine(buf, &view->cursor);
      view->cursor.y--;
      bufferSwapRow(buf, view->cursor.y, view->cursor.y+1);
      break;
    default:
      return ST_NOOP;
  }
  view->st.cursx = view->cursor.x;
  return ST_SUCCESS;
}

int viewVisCmdHandle(view_t *view, int key){
  switch(key){
    case 'v': 
      viewUpdateSelection(view, VISUAL, 0);
      return VISUAL;
    case 'V':
      viewUpdateSelection(view, VISUAL_LINE, 0);
      return VISUAL_LINE;
  }
  return 0;
}

int viewMiscCmdHandle(view_t *view, parsedcmd_t *cmd){
  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  switch(cmd->cmd){
    case 'd':
      editorSwitchMode(VISUAL);
      bufferUpdateSelection(buf, view->cursor, VISUAL_LINE, 0);
      _arrMvmnt(view, ARROW_RIGHT, cmd->repx-1, NORMAL);
      bufferUpdateSelection(buf, view->cursor, VISUAL_LINE, 1);
      bufferDeleteSelection(buf, &view->cursor);
      break;
    case 'D':
      editorSwitchMode(VISUAL_LINE);
      bufferUpdateSelection(buf, view->cursor, VISUAL_LINE, 0);
      _arrMvmnt(view, ARROW_DOWN, cmd->repx-1, NORMAL);
      bufferUpdateSelection(buf, view->cursor, VISUAL_LINE, 1);
      bufferDeleteSelection(buf, &view->cursor);
      editorSwitchMode(NORMAL);
      break;
    case 'x':
      editorSwitchMode(VISUAL);
      bufferUpdateSelection(buf, view->cursor, VISUAL_LINE, 0);
      _arrMvmnt(view, ARROW_RIGHT, cmd->repx-1, NORMAL);
      bufferUpdateSelection(buf, view->cursor, VISUAL_LINE, 1);
      bufferCopySelection(buf);
      bufferDeleteSelection(buf, &view->cursor);
      break;
    case 'X':
      editorSwitchMode(VISUAL_LINE);
      bufferUpdateSelection(buf, view->cursor, VISUAL_LINE, 0);
      _arrMvmnt(view, ARROW_DOWN, cmd->repx-1, NORMAL);
      bufferUpdateSelection(buf, view->cursor, VISUAL_LINE, 1);
      bufferCopySelection(buf);
      bufferDeleteSelection(buf, &view->cursor);
      editorSwitchMode(NORMAL);
      break;
    case 'r':
      if (cmd->arg1 == 0) return ST_WAIT;
      if (cmd->arg1 >= 127) return ST_ERR;
      bufferReplaceChar(buf, &view->cursor, cmd->arg1, cmd->repx);
      break;
    case 'f':
    case 'F':
      {
        if(cmd->arg1 == 0) return ST_WAIT;
        if (cmd->arg1 >= 127) return ST_ERR;
        int flg = cmd->cmd=='F'?JMP_BACK:0;
        while(cmd->repx--
            && bufferFindChar(buf, &view->cursor, cmd->arg1, flg)){}
        break;
      }
    case '{':
      bufferParaNav(buf, &view->cursor, cmd->repx, JMP_BACK);
      break;
    case '}':
      bufferParaNav(buf, &view->cursor, cmd->repx, 0);
      break;
    case '/':
      editorFind("/%s");
      break;
    case ':':
      editorCmdPromptProc(":%s");
      break;
    case 'P':
    case 'p':
      {
        char *text = getClipBoard();
        if(!text) break;
        bufferInsertText(buf, &view->cursor, text, cmd->cmd=='P'?BACKWARD:FORWARD);
        free(text);
      }
      break;
    default:
      return ST_ERR;
  }
  view->st.cursx = view->cursor.x;
  _cursST(view, NORMAL);
  return ST_SUCCESS;
}

int viewInsertEdit(view_t *view, int key){
  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  switch(key){
    case RETURN:
      bufferInsertNewLine(buf, &view->cursor);
      break;
    case BACKSPACE:
    case CTRL_H:
      bufferDelChar(buf, &view->cursor, -1);
      break;
    case DEL_KEY:
      bufferDelChar(buf, &view->cursor, 0);
      break;
    case ARROW_LEFT:
    case ARROW_RIGHT:
    case ARROW_UP:
    case ARROW_DOWN:
      _arrMvmnt(view, key, 1, INSERT);
      break;
    default:
      bufferInsertChar(buf, &view->cursor, key);
  }
  view->st.cursx = view->cursor.x;
  return ST_SUCCESS;
}

int viewBack2Normal(view_t *view, int key, int mode){
  switch(key){
    case CTRL_C:
    case ESCAPE:
      if(mode == INSERT) _arrMvmnt(view, ARROW_LEFT, 1, mode);
      return ST_SUCCESS;
  }
  return ST_NOOP;
}

int viewVisualOp(view_t *view, parsedcmd_t *cmd, int mode){
  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  switch(cmd->cmd){
    case 'o':
      bufferSwapSelCursor(buf, &view->cursor);
      break;
    case 'x':
      bufferCopySelection(buf);
      bufferDeleteSelection(buf, &view->cursor);
      editorSwitchMode(NORMAL);
      break;
    case 'y':
      bufferCopySelection(buf);
      editorSwitchMode(NORMAL);
      break;
    case 'd':
      bufferDeleteSelection(buf, &view->cursor);
      editorSwitchMode(NORMAL);
      break;
    case 'p':
    case 'P':
      {
        char *text = getClipBoard();
        bufferCopySelection(buf);
        bufferDeleteSelection(buf, &view->cursor);
        editorSwitchMode(NORMAL);

        if(!text) break;
        bufferInsertText(buf, &view->cursor, text, BACKWARD);
        free(text);
      }
      break;
    case 'c':
      {
        int ec = view->cursor.y;
        bufferDeleteSelection(buf, &view->cursor);
        editorSwitchMode(INSERT);
        if(mode == VISUAL)
          _arrMvmnt(view, ARROW_RIGHT, 1, INSERT);
        else if (ec >= buf->row_size){
          _arrMvmnt(view, '$', 1, INSERT);
          bufferInsertNewLine(buf, &view->cursor);
        }
      }
      break;
    case 'r':
      if(cmd->arg1 == 0) return ST_WAIT;
      if(cmd->arg1 >= 127) return ST_ERR;
      bufferReplaceSelection(buf, cmd->arg1);
      break;
    case 'v':
      editorSwitchMode(mode==VISUAL?VISUAL_LINE:VISUAL);
      break;
    case 'C':
      bufferCommentSelection(buf);
      editorSwitchMode(NORMAL);
      break;
    default:
      return ST_NOOP;
  }
  view->st.cursx = view->cursor.x;
  _cursST(view, mode);
  return ST_SUCCESS;
}

void viewUpdateSelection(view_t *view, int mode, int flags){
  if(mode != VISUAL && mode != VISUAL_LINE) return;
  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  bufferUpdateSelection(buf, view->cursor, mode, flags);
}
void viewSwapSelCursor(view_t *view){
  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  bufferSwapSelCursor(buf, &view->cursor);
}
void viewDeleteSelection(view_t *view){
  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  bufferDeleteSelection(buf, &view->cursor);
  view->st.cursx = view->cursor.x;
}

void _drawStatusBar(view_t *view, abuf *ab){
  abPutCursor(ab, view->position.x, view->position.y+view->size.y-1);

  abAppend(ab, "\x1b[48;5;237m", 11);

  char lstatus[80], rstatus[80];
  buffer_t *buf = windowGetBufOfView(&editor.window, view);

  int sl = buf->filename? strlen(buf->filename) : 0;
  if(sl>16) sl-=16;
  else sl = 0;
  int len = snprintf(lstatus, sizeof(lstatus), " %s%.20s [%s] %.3s", sl?"...":"",
                     buf->filename ? &buf->filename[sl] : "[No name]",
                     buf->syntax ? buf->syntax->filetype : "nt",
                     buf->dirty ? "[+]" : "");
  if (len > view->size.x)
    len = view->size.x;

  int rlen = snprintf(rstatus, sizeof(rstatus), "%d|%d, %d ", view->cursor.y,
                      buf->row_size, view->cursor.x);

abAppend(ab, lstatus, len);
  for (; len < view->size.x - rlen; len++)
    abAppend(ab, " ", 1);
  abAppend(ab, rstatus, rlen);
  abAppend(ab, "\x1b[m", 3);
}

void _addWelcomeMsg(view_t *view, abuf *ab) {
  char wlc[80];
  int wlclen =
      snprintf(wlc, sizeof(wlc), "trvim v%s : A normal looking text editor.", EDITOR_VERSION);
  if (wlclen > view->size.x)
    wlclen = view->size.x;

  int padding = (view->size.x - wlclen) / 2;
  while (padding--)
    abAppend(ab, " ", 1);
  abAppend(ab, wlc, wlclen);
}

void _arrMvmnt(view_t *view, int key, int times, int mode){
  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  if(!buf || !buf->row_size) return;

  view->cursor.y = clamp(view->cursor.y, 0, buf->row_size-1);
  int _rsz = buf->rows[view->cursor.y].size-(mode!=INSERT);

  switch (key) {
    case ARROW_LEFT:
      view->st.cursx = max(view->cursor.x-times, 0);
      break;
    case ARROW_RIGHT:
      view->st.cursx = min(view->cursor.x+times, _rsz);
      break;
    case ARROW_UP:
      view->cursor.y = max(view->cursor.y-times, 0);
      break;
    case ARROW_DOWN:
      view->cursor.y  = min(view->cursor.y+times, buf->row_size-1);
      break;
    case '$':
      view->cursor.y  = clamp(view->cursor.y+times-1, 0, buf->row_size);
      view->st.cursx = buf->rows[view->cursor.y].size - (mode!=INSERT);
      break;
    case '0':
      view->st.cursx = 0;
      break;
    case '_':
      view->st.cursx = firstCharIndex(buf->rows[view->cursor.y].chars);
      break;
  }
  _cursST(view, mode);
}

void _pageMvmnt(view_t *view, int key){
  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  if(!buf || !buf->row_size) return;

  if (key == PAGE_UP) view->cursor.y = view->offset.y + settings.scrollpadding;
  else {
    view->cursor.y =
      view->offset.y + view->size.y - 1 - settings.scrollpadding;
    if (view->cursor.y >= buf->row_size)
      view->cursor.y = buf->row_size - 1;
  }
  int _times = view->size.y;
  _arrMvmnt(view, key == PAGE_UP ? ARROW_UP : ARROW_DOWN, _times, NORMAL);
}

void _absJmp(view_t *view, int line){
  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  if(!buf || !buf->row_size) return;
  view->cursor.y = min(line-1, buf->row_size-1);

  _cursST(view, NORMAL);
}

void _wrdJmp(view_t *view, int flags, int times){
  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  while(times-- && bufferWordJump(buf, &view->cursor, flags)){}
  view->st.cursx = view->cursor.x;
}

void _miscJmp(view_t *view, int key){
  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  if(!buf || !buf->row_size) return;
  switch(key){
    case 'G':
      view->cursor.y = buf->row_size-1;
      break;
    case 'H':
      view->cursor.y = settings.scrollpadding + view->offset.y;
      break;
    case 'L':
      view->cursor.y = view->offset.y+view->size.y
        -settings.scrollpadding-STATUSBAR_SZ;
      break;      
  }
  view->cursor.y = clamp(view->cursor.y, 0, buf->row_size-1);
  _cursST(view, NORMAL);
}

void _cursST(view_t *view, int mode){
  buffer_t *buf = windowGetBufOfView(&editor.window, view);
  if(!buf || !buf->row_size) return;

  // cursor state preservation
  int _rsz = buf->rows[view->cursor.y].size-(mode!=INSERT);
  view->cursor.x = view->st.cursx;
  view->cursor.x = clamp(view->st.cursx, 0, _rsz);
}
