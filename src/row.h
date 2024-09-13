typedef struct {
  int size, rsize;
  char *chars, *renderch;
} erow;

void rowUpdate(erow *row);
void rowInsertCharacter(erow *row, int index, int ch);
int rowCursorToRenderX(erow *row, int cursor_x);
