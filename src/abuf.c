#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "abuf.h"
#include "utils.h"

// -- definitions --
void abAppend(abuf *ab, const char *s, int len) {
  char *new = realloc(ab->buf, ab->len + len);

  if (new == NULL) return;
  memcpy(&new[ab->len], s, len);
  ab->buf = new;
  ab->len += len;
}

void abFree(abuf *ab) { free(ab->buf); }

void writeBuf(abuf *ab){
  write(STDOUT_FILENO, ab->buf, ab->len);
}

void abPutCursor(abuf *ab, int x, int y){
  char _cbuf[32];
  int l = snprintf(_cbuf, sizeof(_cbuf), "\x1b[%d;%dH", y, x);
  if (l == 0) die("cursor positioning failed");
  abAppend(ab, _cbuf, l);
}
