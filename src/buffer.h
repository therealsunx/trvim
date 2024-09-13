#pragma once

#include "utils.h"
#include "abuf.h"
#include "row.h"

typedef struct {
  vec2 cursor, size, offset;
  int row_size, dirty, st_cx, render_x;
  erow *rows;
  char *filename;
} buffer;

void initBuffer(buffer *buf);
void addWelcomeMsg(buffer *buf, abuf *ab);
void bufferDrawRows(buffer *buf, abuf *ab);
void bufferDrawStatusBar(buffer *buf, abuf *ab);
void bufferShowCursor(buffer *buf, abuf *ab);
void bufferMoveCursor(buffer *buf, int key);
void bufferScroll(buffer *buf);
void bufferInsertRow(buffer *buf, int index, char *s, size_t len);
void bufferDeleteRow(buffer *buf, int index);
void bufferOpenFile(buffer *buf, char *filename);
void bufferInsertChar(buffer* buf, int ch);
void bufferInsertNewLine(buffer *buf);
void bufferDelChar(buffer *buf, int dir);
int bufferSave(buffer *buf);
char *bufferRowtoStr(buffer *buf, int *buflen);
void bufferFind(buffer *buf, char *query);

