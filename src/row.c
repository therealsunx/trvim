#include <stdlib.h>

#include "row.h"
#include "settings.h"

extern settingsType settings;

void rowUpdate(erow *row){
  //count tabs
  int tabs = 0;
  for(int j=0; j<row->size; j++){
    if(row->chars[j] == '\t') tabs++;
  }

  free(row->renderch);
  row->renderch = malloc(row->size+(settings.tabwidth-1)*tabs+1);

  int idx = 0;
  for(int j=0; j<row->size; j++){
    if(row->chars[j] == '\t'){
      row->renderch[idx++] = ' ';
      while(idx % settings.tabwidth != 0) row->renderch[idx++] = ' ';
    }else{
      row->renderch[idx++] = row->chars[j];
    }
  }

  row->renderch[idx] = '\0';
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

