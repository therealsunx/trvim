#pragma once

#include "utils.h"
#include "abuf.h"
#include "row.h"
#include "highlight.h"

#define DEF_STATE (state_t) {0, NULL}

typedef struct {
  int cursx;
  char* query;
} state_t;

typedef struct {
  vec2 cursor, view_size, size, offset;
  int row_size, dirty, render_x, linenumcol_sz;
  state_t st;
  erow *rows;
  char *filename;
  syntaxhl *syntax;
} buffer;

void initBuffer(buffer *buf);
void bufferUpdateSize(buffer *buf, int sx, int sy);
void addColumn(buffer *buf, abuf *ab, int linenum);
void addWelcomeMsg(buffer *buf, abuf *ab);
void bufferDrawRows(buffer *buf, abuf *ab);
void bufferDrawStatusBar(buffer *buf, abuf *ab);
void bufferShowCursor(buffer *buf, abuf *ab);
void bufferMoveCursor(buffer *buf, int key);
void bufferScroll(buffer *buf);
void bufferUpdateRow(buffer *buf, erow *row);
void bufferInsertRow(buffer *buf, int index, char *s, size_t len);
void bufferDeleteRow(buffer *buf, int index);
void bufferOpenFile(buffer *buf, char *filename);
void bufferInsertChar(buffer* buf, int ch);
void bufferInsertNewLine(buffer *buf);
void bufferDelChar(buffer *buf, int dir);
int bufferSave(buffer *buf);
char *bufferRowtoStr(buffer *buf, int *buflen);
int bufferFind(buffer *buf, char *query, int dir);
void bufferSelectSyntax(buffer *buf);
