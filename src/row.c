#include <stdlib.h>
#include <string.h>

#include "row.h"
#include "settings.h"

extern settingsType settings;

void rowUpdate(erow *row){
  //count tabs
  int tabs = 0;
  for(int j=0; j<row->size; j++){
    if(row->chars[j] == '\t') tabs++;
  }

  free(row->renderchars);
  row->renderchars = malloc(row->size+(settings.tabwidth-1)*tabs+1);

  int idx = 0;
  for(int j=0; j<row->size; j++){
    if(row->chars[j] == '\t'){
      row->renderchars[idx++] = ' ';
      while(idx % settings.tabwidth != 0) row->renderchars[idx++] = ' ';
    }else{
      row->renderchars[idx++] = row->chars[j];
    }
  }

  row->renderchars[idx] = '\0';
  row->rsize = idx;
}

int rowCursorToRenderX(erow *row, int cursor_x){
  int rx = 0;
  for(int j=0; j<cursor_x; j++){
    if(row->chars[j] == '\t')
      rx += (settings.tabwidth-1) - (rx % settings.tabwidth);
    rx++;
  }
  return rx;
}

void rowInsertCharacter(erow *row, int index, int ch){
  if(index < 0 || index > row->size) index = row->size;
  row->chars = realloc(row->chars, row->size+2);
  memmove(&row->chars[index+1], &row->chars[index], row->size-index+1);
  row->size++;
  row->chars[index] = ch;
  rowUpdate(row);
}

void rowDeleteCharacter(erow *row, int index){
  if(index < 0 || index >= row->size) return;
  memmove(&row->chars[index], &row->chars[index+1], row->size-index);
  row->size--;
  rowUpdate(row);
}

void rowFree(erow *row){
  free(row->chars);
  free(row->renderchars);
}

void rowAppendString(erow *row, char *s, size_t len){
  row->chars = realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = '\0';
  rowUpdate(row);
}
