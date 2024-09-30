#pragma once

#include "utils.h"
#include "view.h"
#include "buffer.h"
#include "cmdbar.h"
#include "cmdstack.h"

#define CMDBAR_SZ 1
#define MIN_WIDTH 20
#define STEP_SZ 10

typedef struct {
  vec2 size;

  buffer_t *bufs;
  int bufcount;

  view_t *views;
  int vcount, active_i;
} window_t;

void initWindow(window_t *window);
void freeWindow(window_t *window);

void windowAddBuffer(window_t *window);
void windowRemoveLastBuffer(window_t *window);

buffer_t *windowGetCurBuffer(window_t *window);
view_t *windowGetCurView(window_t *window);

buffer_t *windowGetBufOfView(window_t *window, view_t *view);

void windowAddView(window_t *window);
void windowRemoveView(window_t *window);

int windowOpCmdHandle(window_t *window, parsedcmd_t *cmd);
void windowSizeChange(window_t *window, int delta);

void windowDrawViews(window_t *window, int mode);
void windowDrawCmdBar(window_t *window, cmdbar_t *cmdbar, cmdstack_t *cmdstk);
void windowShowCursor(window_t *window, int mode);
void windowScrollCursor(window_t *window);

int windowShouldClose(window_t *window);
void windowOpenFile(window_t *window, char *filename);

void windowQuitBuffers(window_t *window, int all);
void windowSaveBuffers(window_t *window, int all);
int windowDirtyBufCheck(window_t *window);

int _winUpdateCheck(window_t *window);
void _onSizeUpdate(window_t *window);
void _saveBuffer(buffer_t *buf);

