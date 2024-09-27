#include <stdlib.h>

#include "view.h"
#include "keybinds.h"
#include "settings.h"

extern settings_t settings;

void update(view_t *view) {
  if (!view->buf) return;
  int _sz = view->buf->row_size;
  view->lcols = SIGNCOL_SZ + LCOL_GAP + 1; // 1 minimum char
  while ((_sz /= 10))
    view->lcols++;

  // buffer should not have size for draw.. all handled by view
  // view->buf->size.x = view->size.x-view->lcols;
  // view->buf->size.y = view->size.y-STATUSBAR_SZ;
}

void initView(view_t *view, buffer_t *buf, vec2 size, vec2 position) {
  view->buf = buf;
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

void viewSetBuffer(view_t *view, buffer_t *buf) {
  view->buf = buf;
  view->cursor = DEF_VEC2;
  view->offset = DEF_VEC2;
  update(view);
}

void _addlcol(view_t *view, abuf *ab, int linenum) {
  char _lstr[32];
  int val = linenum + 1, len;
  int _nsz = view->lcols - SIGNCOL_SZ - LCOL_GAP;

  abAppend(ab, "\x1b[48;5;237m \x1b[m\x1b[90m", 20);
  if (linenum < view->buf->row_size) {
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

  vec2 sstate = {BEFORE, BEFORE}; // for selection mode
  boundstype sel = view->buf->selection;
  if (selflag) {
    sel.end.x = rowCursorToRenderX(&view->buf->rows[sel.end.y], sel.end.x);
    sel.start.x =
        rowCursorToRenderX(&view->buf->rows[sel.start.y], sel.start.x);
  }

  for (int y = 0; y < (view->size.y-STATUSBAR_SZ); y++) {
    abPutCursor(&ab, view->position.x+1, view->position.y+y+1);

    int _fr = y + view->offset.y;
    _addlcol(view, &ab, _fr);

    if (_fr >= view->buf->row_size) {
      if (view->buf->row_size == 0 && y == view->size.y / 3)
        _addWelcomeMsg(view, &ab);
    } else {
      int len = clamp(view->buf->rows[_fr].rsize-view->offset.x, 0, view->size.x-view->lcols);

      char *ch = &view->buf->rows[_fr].renderchars[view->offset.x];
      unsigned char *hl = &view->buf->rows[_fr].hlchars[view->offset.x];
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
          if (_ptk == TK_MATCH || _ptk == TK_KEYWORD3)
            abAppend(&ab, "\x1b[m", 3);

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
              abAppend(&ab, "\x1b[100m", 6);
            }
          } else if(sstate.x == INBOUND){
            if(_fr == sel.end.y && i>sel.end.x){
              sstate.x = AFTER;
              abAppend(&ab, "\x1b[49m", 5);
            }
          } // no need to process after AFTER
          if(i==0 && sstate.x == INBOUND){
            abAppend(&ab, "\x1b[100m", 6);
          }
        }

        abAppend(&ab, &ch[i], 1);
      }
    }

    abAppend(&ab, "\x1b[m", 3);
    //abAppend(&ab, "\x1b[m\x1b[K", 6); // clear the line before drawing
    //abAppend(&ab, "\r\n", 2);
  }
  _drawStatusBar(view, &ab);
  writeBuf(&ab);
  abFree(&ab);
}

void viewShowCursor(view_t *view, int mode){
  viewScrollCursor(view);
  showCursor(view->position.y+view->cursor.y-view->offset.y+1,
      view->position.x+view->lcols+view->render_cx-view->offset.x+1);
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

  view->render_cx = view->cursor.y < view->buf->row_size?
    rowCursorToRenderX(&view->buf->rows[view->cursor.y], view->cursor.x):
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
      return 1;
    case END_KEY:
      _arrMvmnt(view, '$', 1, mode);
      return 1;
    case PAGE_UP:
    case PAGE_DOWN:
      _pageMvmnt(view, key);
      return 1;
    case CTRL_Z:
    case CTRL_X:
      exit(0);
      break;
  }
  return 0;
}

int viewMvmtCmdHandle(view_t *view, int key, int times, int mode){
  switch(key){
    case 'h':
    case ARROW_LEFT:
      _arrMvmnt(view, ARROW_LEFT, times, mode);
      return 1;
    case 'j':
    case ARROW_DOWN:
      _arrMvmnt(view, ARROW_DOWN, times, mode);
      return 1;
    case 'k':
    case ARROW_UP:
      _arrMvmnt(view, ARROW_UP, times, mode);
      return 1;
    case 'l':
    case ARROW_RIGHT:
      _arrMvmnt(view, ARROW_RIGHT, times, mode);
      return 1;
    case '$':
      _arrMvmnt(view, '$', times, mode);
      return 1;
    case '0':
      _arrMvmnt(view, '0', times, mode);
      return 1;
    case '_':
      _arrMvmnt(view, '_', times, mode);
      return 1;
    case 'J':
      _absJmp(view, times);
      return 1;
    case 'w':
      _wrdJmp(view, 0, times);
      return 1;
    case 'W':
      _wrdJmp(view, JMP_PUNC, times);
      return 1;
    case 'e':
      _wrdJmp(view, JMP_END, times);
      return 1;
    case 'E':
      _wrdJmp(view, JMP_END|JMP_PUNC, times);
      return 1;
    case 'b':
      _wrdJmp(view, JMP_BACK, times);
      return 1;
    case 'B':
      _wrdJmp(view, JMP_BACK|JMP_PUNC, times);
      return 1;
  }
  return 0;
}

int viewInsCmdHandle(view_t *view, int key){
  switch(key){
    case 'i':
      if(!view->buf->row_size) bufferInsertRow(view->buf, 0, "", 0);
      view->st.cursx = view->cursor.x;
      return 1;
    case 'I':
      if(!view->buf->row_size) bufferInsertRow(view->buf, 0, "", 0);
      else _arrMvmnt(view, '_', 1, INSERT);
      view->st.cursx = view->cursor.x;
      return 1;
    case 'a':
      if(!view->buf->row_size) bufferInsertRow(view->buf, 0, "", 0);
      else _arrMvmnt(view, ARROW_RIGHT, 1, INSERT);
      view->st.cursx = view->cursor.x;
      return 1;
    case 'A':
      if(!view->buf->row_size) bufferInsertRow(view->buf, 0, "", 0);
      else _arrMvmnt(view, '$', 1, INSERT);
      view->st.cursx = view->cursor.x;
      return 1;
    case 'o':
      if(!view->buf->row_size) bufferInsertRow(view->buf, 0, "", 0);
      _arrMvmnt(view, '$', 1, INSERT);
      bufferInsertNewLine(view->buf, &view->cursor);
      view->st.cursx = view->cursor.x;
      return 1;
    case 'O':
      if(!view->buf->row_size) bufferInsertRow(view->buf, 0, "", 0);
      _arrMvmnt(view, '$', 1, INSERT);
      bufferInsertNewLine(view->buf, &view->cursor); // TODO : should change the cursor
      bufferSwapRow(view->buf, view->cursor.y, view->cursor.y+1);
      view->st.cursx = view->cursor.x;
      return 1;
  }
  return 0;
}

int viewVisCmdHandle(int key){
  switch(key){
    case 'v': return VISUAL;
    case 'V': return VISUAL_LINE;
  }
  return 0;
}

int viewMiscCmdHandle(view_t *view, parsedcmd_t *cmd){
  /*
   * editorFind on /
   * editorCmdPromptProc on :
   * find char on f F
   * buffer para nav on { }
   * jumps : w W b B e E
   * G : eof
   * J : absolute jump
   * H : page start
   * L : page end
   * */
  switch(cmd->cmd){
    case 'r':
      if (cmd->arg1 == 0 || cmd->arg1>=127) return 0;
      bufferReplaceChar(view->buf, &view->cursor, cmd->arg1, cmd->repx);
      return 1;

  }
  return 0;
}

int viewInsertEdit(view_t *view, int key){
  switch(key){
    case RETURN:
      bufferInsertNewLine(view->buf, &view->cursor);
      break;
    case BACKSPACE:
    case CTRL_H:
      bufferDelChar(view->buf, &view->cursor, -1);
      break;
    case DEL_KEY:
      bufferDelChar(view->buf, &view->cursor, 0);
      break;
    case ARROW_LEFT:
    case ARROW_RIGHT:
    case ARROW_UP:
    case ARROW_DOWN:
      _arrMvmnt(view, key, 1, INSERT);
      break;
    default:
      bufferInsertChar(view->buf, &view->cursor, key);
  }
  view->st.cursx = view->cursor.x;
  return 1;
}

int viewBack2Normal(int key){
  switch(key){
    case CTRL_C:
    case ESCAPE: return 1;
  }
  return 0;
}

int viewVisualOp(view_t *view, parsedcmd_t *cmd){
  // TODO : visual mode operations
  return 0;
}

void viewUpdateSelection(view_t *view, int mode, int flags){
  bufferUpdateSelection(view->buf, view->cursor, mode, flags);
}
void viewSwapSelCursor(view_t *view){
  bufferSwapSelCursor(view->buf, &view->cursor);
}
void viewDeleteSelection(view_t *view){
  bufferDeleteSelection(view->buf, &view->cursor);
  view->st.cursx = view->cursor.x;
}

void _drawStatusBar(view_t *view, abuf *ab){
  abPutCursor(ab, view->position.x+1, view->position.y+view->size.y);

  abAppend(ab, "\x1b[48;5;237m", 11);

  char lstatus[80], rstatus[80];
  int len = snprintf(lstatus, sizeof(lstatus), "%.20s [%s] %.3s",
                     view->buf->filename ? view->buf->filename : "[No name]",
                     view->buf->syntax ? view->buf->syntax->filetype : "nt",
                     view->buf->dirty ? "[+]" : "");
  if (len > view->size.x)
    len = view->size.x;

  int rlen = snprintf(rstatus, sizeof(rstatus), "%d|%d, %d", view->cursor.y,
                      view->buf->row_size, view->cursor.x);

  abAppend(ab, lstatus, len);
  for (; len < view->size.x - rlen; len++)
    abAppend(ab, " ", 1);
  abAppend(ab, rstatus, rlen);
  abAppend(ab, "\x1b[m", 3);
  //abAppend(ab, "\r\n", 2);
}

void _addWelcomeMsg(view_t *view, abuf *ab) {
  // TODO: better welcome msg
  char wlc[80];
  int wlclen =
      snprintf(wlc, sizeof(wlc), "therealtxt editor v%s", EDITOR_VERSION);
  if (wlclen > view->size.x)
    wlclen = view->size.x;

  int padding = (view->size.x - wlclen) / 2;
  while (padding--)
    abAppend(ab, " ", 1);
  abAppend(ab, wlc, wlclen);
}

void _arrMvmnt(view_t *view, int key, int times, int mode){
  if(!view->buf || !view->buf->row_size) return;

  view->cursor.y = clamp(view->cursor.y, 0, view->buf->row_size-1);
  int _rsz = view->buf->rows[view->cursor.y].size-(mode!=INSERT);
  switch (key) {
    case ARROW_LEFT:
      view->st.cursx = max(view->cursor.x-times, 0);
      return;
    case ARROW_RIGHT:
      view->st.cursx = min(view->cursor.x+times, _rsz);
      return;
    case ARROW_UP:
      view->cursor.y = max(view->cursor.y-times, 0);
      break;
    case ARROW_DOWN:
      view->cursor.y  = min(view->cursor.y+times, view->buf->row_size-1);
      break;
    case '$':
      view->cursor.y  = clamp(view->cursor.y+times-1, 0, view->buf->row_size);
      view->st.cursx = view->buf->rows[view->cursor.y].size - (mode!=INSERT);
      break;
    case '0':
      view->st.cursx = 0;
      break;
    case '_':
      view->st.cursx = firstCharIndex(view->buf->rows[view->cursor.y].chars);
      break;
  }
  // cursor state preservation
  _rsz = view->buf->rows[view->cursor.y].size-(mode!=INSERT);
  view->cursor.x = view->st.cursx;
  view->cursor.x = clamp(view->st.cursx, 0, _rsz);
}

void _pageMvmnt(view_t *view, int key){
  if(!view->buf || !view->buf->row_size) return;

  if (key == PAGE_UP) view->cursor.y = view->offset.y + settings.scrollpadding;
  else {
    view->cursor.y =
      view->offset.y + view->size.y - 1 - settings.scrollpadding;
    if (view->cursor.y >= view->buf->row_size)
      view->cursor.y = view->buf->row_size - 1;
  }
  int _times = view->size.y;
  _arrMvmnt(view, key == PAGE_UP ? ARROW_UP : ARROW_DOWN, _times, NORMAL);
}

void _absJmp(view_t *view, int line){
  if(!view->buf || !view->buf->row_size) return;
  view->cursor.y = min(line-1, view->buf->row_size-1);
}

void _wrdJmp(view_t *view, int flags, int times){
  while(times-- && bufferWordJump(view->buf, &view->cursor, flags)){}
  view->st.cursx = view->cursor.x;
}
