#pragma once

#define ABUF_INIT {NULL, 0}

typedef struct {
  char *buf;
  int len;
} abuf;

// -- append Buffer methods --
void abAppend(abuf *appendbuffer, const char *string, int length);
void abFree(abuf *appendbuffer);
void writeBuf(abuf *ab);
void abPutCursor(abuf *ab, int x, int y);
