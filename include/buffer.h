#pragma once

#include "utils.h"
#include "row.h"
#include "highlight.h"

#define DEF_STATE (state_t) {0}

typedef struct {
  int row_size, dirty;
  erow *rows;
  char *filename;
  syntaxhl *syntax;
  boundstype selection;
} buffer_t;

void initBuffer(buffer_t *buf);
void freeBuffer(buffer_t *buf);

int bufferWordJump(buffer_t *buf, vec2 *cursor, int flags);
int bufferFindChar(buffer_t *buf, vec2 *cursor, char char_, int dirflg);
int bufferParaNav(buffer_t *buf, vec2 *cursor, int times, int dirflag);
void bufferReplaceChar(buffer_t *buf, vec2 *cursor, char char_, int repx);

char *bufferSel2Text(buffer_t *buf);
void bufferCopySelection(buffer_t *buf);
void bufferUpdateSelection(buffer_t *buf, vec2 cursor, int mode, int flags);
void bufferSwapSelCursor(buffer_t *buf, vec2 *cursor);
void bufferDeleteSelection(buffer_t *buf, vec2 *cursor);
void bufferReplaceSelection(buffer_t *buf, char c);
void bufferCommentSelection(buffer_t *buf);

void bufferUpdateRow(buffer_t *buf, erow *row);
void bufferInsertRow(buffer_t *buf, int index, char *s, size_t len);
void bufferDeleteRows(buffer_t *buf, int index, int len);
void bufferSwapRow(buffer_t *buf, int index1, int index2);

int bufferOpenFile(buffer_t *buf, char *filename);
void bufferInsertChar(buffer_t *buf, vec2 *cursor, int ch);
void bufferInsertNewLine(buffer_t *buf, vec2 *cursor);
void bufferInsertText(buffer_t *buf, vec2 *cursor, char *text, int dir);
void bufferDelChar(buffer_t *buf, vec2 *cursor, int dir);

int bufferSave(buffer_t *buf);
char *bufferRowtoStr(buffer_t *buf, int *buflen);
int bufferFind(buffer_t *buf, char *query, vec2 *cursor, int dir);
void bufferSelectSyntax(buffer_t *buf);
