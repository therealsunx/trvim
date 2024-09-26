#pragma once

#include "utils.h"
#include "view.h"
#include "cmdbar.h"
#include "cmdstack.h"

#define CMDBAR_SZ 1

typedef struct {
  vec2 size;

  buffer_t *bufs;
  int bufcount;

  view_t *views;
  int vcount, active_i;
} window_t;

void initWindow(window_t *window);
void freeWindow(window_t *window);

// TODO : maybe think hard on which buffer to delete, if function is required
void windowAddBuffer(window_t *window);

buffer_t *windowGetCurBuffer(window_t *window);
view_t *windowGetCurView(window_t *window);

void windowAddView(window_t *window);
void windowRemoveView(window_t *window);

void windowSizeUpdate(window_t *window);
void windowDrawViews(window_t *window, int mode);
void windowDrawCmdBar(window_t *window, cmdbar_t *cmdbar, cmdstack_t *cmdstk);
void windowShowCursor(window_t *window, int mode);

void windowOpenFile(window_t *window, char *filename);
int windowDirtyBufCheck(window_t *window);

int _winUpdateCheck(window_t *window);
void _onSizeUpdate(window_t *window);

