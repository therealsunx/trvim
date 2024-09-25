#pragma once

#include <stdio.h>
#include "highlight.h"

typedef struct {
  int size, rsize;
  char *chars, *renderchars;
  unsigned char *hlchars;
} erow;

void rowUpdate(erow *row);
void rowUpdateSyntax(erow *row, syntaxhl *syntax);
void rowInsertCharacter(erow *row, int index, int ch);
int rowReplaceCharacter(erow *row, char ch_, int start, int len);
void rowDeleteCharacters(erow *row, int start, int len);
int rowCursorToRenderX(erow *row, int cursor_x);
int rowRendertoCursorX(erow *row, int render_x);
void rowFree(erow *row);
void rowAppendString(erow *row, char *s, size_t len);
