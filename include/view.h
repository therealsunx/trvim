#pragma once

#include "buffer.h"
#include "abuf.h"
#include "utils.h"
#include "cmdstack.h"

#define STATUSBAR_SZ 1
#define SIGNCOL_SZ 1
#define LCOL_GAP 1

typedef struct {
  int cursx;
  char *query;
} state_t;

typedef struct {
  int buf_i;
  vec2 position, size;
  vec2 cursor, offset;
  int render_cx, lcols;
  state_t st;
} view_t;

void initView(view_t *view, int buf_i, vec2 size, vec2 position);

void viewSetDims(view_t *view, vec2 size, vec2 position);
void viewSetBuffer(view_t *view, int buf_i);

void viewDraw(view_t *view, int selflag);
void viewShowCursor(view_t *view, int mode);
void viewScrollCursor(view_t *view);

// in any mode
int viewMasterKeys(view_t *view, int key, int mode);

// in normal mode
int viewMvmtCmdHandle(view_t *view, int key, int times, int mode);
int viewInsCmdHandle(view_t *view, int key);
int viewVisCmdHandle(view_t *view, int key);
int viewMiscCmdHandle(view_t *view, parsedcmd_t *cmd);

// in insert mode
int viewInsertEdit(view_t *view, int key);
int viewBack2Normal(view_t *view, int key, int mode);

// in visual mode
int viewVisualOp(view_t *view, parsedcmd_t *cmd, int mode);

// other functions for buffer edit
void viewUpdateSelection(view_t *view, int mode, int flags);
void viewSwapSelCursor(view_t *view);
void viewDeleteSelection(view_t *view);

void _addWelcomeMsg(view_t *view, abuf *ab);
void _drawStatusBar(view_t *view, abuf *ab);
void _arrMvmnt(view_t *view, int key, int times, int mode);
void _pageMvmnt(view_t *view, int key);
void _absJmp(view_t *view, int line);
void _inlineMvmt(view_t *view, int key, int times);
void _wrdJmp(view_t *view, int flags, int times);
void _miscJmp(view_t *view, int key);

