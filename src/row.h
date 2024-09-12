typedef struct {
  int size, rsize;
  char *chars, *renderch;
} erow;

void rowUpdate(erow *row);
int rowCursorToRenderX(erow *row, int cursor_x);
