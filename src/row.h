#include <stdio.h>

typedef struct {
  int size, rsize;
  char *chars, *renderchars;
} erow;

void rowUpdate(erow *row);
void rowInsertCharacter(erow *row, int index, int ch);
void rowDeleteCharacter(erow *row, int index);
int rowCursorToRenderX(erow *row, int cursor_x);
void rowFree(erow *row);
void rowAppendString(erow *row, char *s, size_t len);
