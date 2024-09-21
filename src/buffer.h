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
void freeBuffer(buffer *buf);
void bufferUpdateSize(buffer *buf, int sx, int sy);
void addColumn(buffer *buf, abuf *ab, int linenum);
void addWelcomeMsg(buffer *buf, abuf *ab);
void bufferDrawRows(buffer *buf, abuf *ab);
void bufferDrawStatusBar(buffer *buf, abuf *ab);

void bufferShowCursor(buffer *buf, abuf *ab);
void bufferMoveCursor(buffer *buf, int key, int mode, int repeatx);
void bufferAbsoluteJump(buffer *buf, int line);
void bufferGotoEnd(buffer *buf, int mode, int posflg);
void bufferScroll(buffer *buf);
void bufferPageScroll(buffer *buf, int key);
int bufferWordJump(buffer *buf, int flags);
int bufferFindChar(buffer *buf, char char_, int dirflg);
int bufferParaNav(buffer *buf, int dirflag);
void bufferReplaceChar(buffer *buf, char char_, int repx);

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
