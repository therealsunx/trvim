#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "window.h"
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
  if(window->views) viewSetBuffer(&window->views[window->active_i], buf);
}

buffer_t *windowGetCurBuffer(window_t *window){
  return window->views[window->active_i].buf;
}

view_t *windowGetCurView(window_t *window){
  return &window->views[window->active_i];
}

void windowAddView(window_t *window){
  // there should be at least one buffer
  if(window->bufcount == 0) die("no buffer found");

  window->vcount++;
  window->active_i = window->vcount-1;
  window->views = window->views?
    realloc(window->views, window->vcount*sizeof(view_t)):
    malloc(window->vcount*sizeof(view_t));

  buffer_t *buf = window->vcount==1?
    &window->bufs[0]:
    window->views[window->active_i].buf;
  initView(&window->views[window->vcount-1], buf, window->size, DEF_VEC2);
  _onSizeUpdate(window);
}

void windowRemoveView(window_t *window){
  window->vcount--;
  memmove(&window->views[window->active_i],
      &window->views[window->active_i+1],
      (window->vcount-window->active_i-1)*sizeof(view_t));
  window->active_i =clamp(window->active_i, 0, window->vcount-1);
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
      viewSetBuffer(view, buf);
      return;
    }
  }
  // open new file into new buffer
  windowAddBuffer(window);
  buffer_t *buf = &window->bufs[window->bufcount-1];
  bufferOpenFile(buf, filename);
  viewSetBuffer(view, buf);
}

int windowDirtyBufCheck(window_t *window){
  if(!window->bufs) return 0;
  for(int i=0; i<window->bufcount; i++){
    if(window->bufs[i].dirty) return 1;
  }
  return 0;
}
