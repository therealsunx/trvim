#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "window.h"
#include "editor.h"
#include "view.h"

int _winUpdateCheck(window_t *window){
  vec2 _sz;
  if (getWindowSize(&_sz.y, &_sz.x) == -1)
    die("invalid window size");

  if(vec2areSame(_sz, window->size)) return 0;
  window->size = _sz;
  return 1;
}

void _onSizeUpdate(window_t *window){
  if(window->vcount == 0) die("nothing to view");

  vec2 _s = {window->size.x / window->vcount, window->size.y-CMDBAR_SZ};
  int rem = window->size.x % window->vcount;
  vec2 _o = DEF_VEC2;
  view_t *view;
  for(int i=0; i<window->vcount; i++){
    view = &window->views[i];
    if(rem) {
      viewSetDims(view, (vec2){_s.x+rem, _s.y}, _o);
      _o.x += _s.x+rem;
      rem = 0;
    } else {
      viewSetDims(view, _s, _o);
      _o.x += _s.x;
    }
  }
}

void initWindow(window_t *window){
  window->bufcount = 0;
  window->vcount = 0;
  _winUpdateCheck(window);
  windowAddBuffer(window);
  windowAddView(window);
  _onSizeUpdate(window);
}

void freeWindow(window_t *window){
  for(int i=0; i<window->bufcount; i++) freeBuffer(&window->bufs[i]);
  free(window->bufs);
  free(window->views);
}

void windowAddBuffer(window_t *window){
  window->bufcount++;
  window->bufs = window->bufs?
    realloc(window->bufs, window->bufcount*sizeof(buffer_t)):
    malloc(window->bufcount*sizeof(buffer_t));
  buffer_t *buf = &window->bufs[window->bufcount-1];
  initBuffer(buf);
  if(window->views)
    viewSetBuffer(&window->views[window->active_i], window->bufcount-1);
}

void windowRemoveLastBuffer(window_t *window){
  window->bufcount--;
  freeBuffer(&window->bufs[window->bufcount]);
  window->bufs = realloc(window->bufs, window->bufcount*sizeof(buffer_t));
}

buffer_t *windowGetCurBuffer(window_t *window){
  if(!window->bufs) return NULL;
  view_t *view = windowGetCurView(window);
  if(!view) return &window->bufs[0];
  view->buf_i = clamp(view->buf_i, 0, window->bufcount-1);
  return &window->bufs[view->buf_i];
}

view_t *windowGetCurView(window_t *window){
  return &window->views[window->active_i];
}

buffer_t *windowGetBufOfView(window_t *window, view_t *view){
  view->buf_i = clamp(view->buf_i, 0, window->bufcount-1);
  return &window->bufs[view->buf_i];
}
void windowAddView(window_t *window){
  // there should be at least one buffer
  if(window->bufcount == 0) die("no buffer found");

  window->vcount++;
  window->views = window->views?
    realloc(window->views, window->vcount*sizeof(view_t)):
    malloc(window->vcount*sizeof(view_t));

  int bi = window->vcount==1?0:windowGetCurView(window)->buf_i;
  window->active_i = window->vcount-1;
  initView(&window->views[window->active_i], bi, window->size, DEF_VEC2);
  _onSizeUpdate(window);
}

void windowRemoveView(window_t *window){
  if(window->vcount == 1) exit(0);
  window->vcount--;
  memmove(&window->views[window->active_i],
      &window->views[window->active_i+1],
      (window->vcount-window->active_i-1)*sizeof(view_t));
  window->active_i =clamp(window->active_i, 0, window->vcount-1);
  _onSizeUpdate(window);
}

int windowOpCmdHandle(window_t *window, parsedcmd_t *cmd){
  switch(cmd->cmd){
    case '[':
      cmd->repx %= window->vcount;
      window->active_i = (window->active_i+window->vcount-cmd->repx)%window->vcount;
      return ST_SUCCESS;
    case ']':
      cmd->repx %= window->vcount;
      window->active_i = (window->active_i+cmd->repx)%window->vcount;
      return ST_SUCCESS;
    case ',':
      if(cmd->repx == 1) cmd->repx = STEP_SZ;
      windowSizeChange(window, -cmd->repx);
      return ST_SUCCESS;
    case '.':
      if(cmd->repx == 1) cmd->repx = STEP_SZ;
      windowSizeChange(window, cmd->repx);
      return ST_SUCCESS;
  }
  return ST_NOOP;
}

void windowSizeChange(window_t *window, int delta){
  if(window->vcount < 2) return;
  window->active_i = clamp(window->active_i, 0, window->vcount-1);
  int _fi = window->active_i;
  int _ni = _fi+1;
  if(_ni == window->vcount){
    _ni--;
    _fi--;
  }
  view_t *fv = &window->views[_fi];
  view_t *nv = &window->views[_ni];

  int tw = fv->size.x + nv->size.x;
  fv->size.x = max(MIN_WIDTH, fv->size.x+delta);
  nv->size.x = tw - fv->size.x;
  nv->position.x = fv->position.x + fv->size.x;
}

void windowDrawViews(window_t *window, int mode){
  int _selflag = mode == VISUAL || mode == VISUAL_LINE;
  for(int i=0; i<window->vcount;i++){
    viewDraw(&window->views[i], i==window->active_i?_selflag:0);
  }
}

void windowDrawCmdBar(window_t *window, cmdbar_t *cmdbar, cmdstack_t *cmdstk){
  abuf ab = ABUF_INIT;

  abPutCursor(&ab, 0, window->size.y-CMDBAR_SZ);
  char cmdsts[cmdstk->top];
  for (int i = 0; i < cmdstk->top; i++)
    cmdsts[i] = cmdstk->vals[i];


  int _rmsgPos = window->size.x - cmdstk->top;
  if(time(NULL) - cmdbar->msg_t<5){
    int _slen = strlen(cmdbar->msg);
    int _mlen = clamp(_slen, 0, _rmsgPos);
    if(_mlen) abAppend(&ab, &cmdbar->msg[_slen-_mlen], _mlen);
  }
  abPutCursor(&ab, _rmsgPos, window->size.y-CMDBAR_SZ);
  abAppend(&ab, cmdsts, cmdstk->top);
  writeBuf(&ab);
  abFree(&ab);
}

void windowShowCursor(window_t *window, int mode){
  if(!window->views || window->vcount==0) die("no views");
  window->active_i = clamp(window->active_i, 0, window->vcount);
  viewShowCursor(&window->views[window->active_i], mode);
}
void windowScrollCursor(window_t *window){
  if(!window->views || window->vcount==0) die("no views");
  window->active_i = clamp(window->active_i, 0, window->vcount);
  viewScrollCursor(&window->views[window->active_i]);
}

void windowOpenFile(window_t *window, char *filename){
  if(!window->views || window->vcount==0) die("no views");
  view_t *view = &window->views[window->active_i];

  // check if any of the open buffers have the file open
  for(int i=0; i<window->bufcount; i++){
    buffer_t *buf = &window->bufs[i];
    if(buf->filename && strcmp(filename, buf->filename)==0){ // same
      viewSetBuffer(view, i);
      return;
    }
  }
  // open new file into new buffer
  windowAddBuffer(window);
  buffer_t *buf = &window->bufs[window->bufcount-1];
  if(!bufferOpenFile(buf, filename)) {
    windowRemoveLastBuffer(window);
    editorSetStatusMsg("\x1b[31mCannot Open file. Enter valid path\x1b[m");
    return;
  }
  viewSetBuffer(view, window->bufcount-1);
}

void windowSaveBuffers(window_t *window, int all){
  if(!all){
    _saveBuffer(windowGetCurBuffer(window));
    return;
  }
  view_t *view = windowGetCurView(window);
  for(int i=0; i<window->bufcount; i++){
    view->buf_i = i;
    _saveBuffer(&window->bufs[i]);
  }
}
void windowQuitBuffers(window_t *window, int all){
  if(all) exit(0);
  windowRemoveView(window);
}

void _saveBuffer(buffer_t *buf){
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

int windowShouldClose(window_t *window){
  return window->vcount == 0;
}

int windowDirtyBufCheck(window_t *window){
  if(!window->bufs) return 0;
  for(int i=0; i<window->bufcount; i++){
    if(window->bufs[i].dirty) return 1;
  }
  return 0;
}
