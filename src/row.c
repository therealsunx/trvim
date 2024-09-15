#include <stdlib.h>
#include <string.h>

#include "row.h"
#include "settings.h"
#include "highlight.h"
#include "utils.h"

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

void setType(unsigned char* hlch, char *str, int len, syntaxhl *syn) {
  char _sstr[len+1];
  memcpy(_sstr, str, len);
  _sstr[len] = '\0';

  // get the token type from string
  unsigned char _tk = TK_NORMAL;
  if(syn->flags & HL_PUNCTUATION
      && isPunct(_sstr[0])) _tk = TK_PUNCTUATION;
  else if(syn->flags & HL_NUMBERS
      && isNumber(_sstr)) _tk = TK_NUMBER;
  else if(syn->flags & HL_KEYWORD){
  }

  memset(hlch, _tk, len);
}

void rowUpdateSyntax(erow *row, syntaxhl *syntax){
  if(!syntax) return; // TODO: actually use patterns in syntax


  free(row->hlchars);
  row->hlchars = malloc(row->rsize);
  memset(row->hlchars, TK_IGNORE, row->rsize);

  int in_str = 0;
  for (int i = 0, ci = 0; i < row->rsize; i++) {

    char c = row->renderchars[i];
    if(isSeparator(c)){
      if(in_str){
        if(c == '"' || c == '\'' || i==row->size-1){
          in_str = 0;
          memset(&row->hlchars[ci], TK_STRING, i-ci);
          ci = i+1;
          continue;
        } else {
          // escape chars check
          continue;
        }
      } else if(ci != i){
        setType(&row->hlchars[ci], &row->renderchars[ci], i-ci, syntax);
      }

      if((syntax->flags & HL_STRING)
          && (c == '"' || c == '\'' )){
        in_str++;
        ci = i;
        continue;;
      } else if(syntax->flags & HL_COMMENT
          && !strncmp(syntax->cmnt_1ls, &row->renderchars[i], strlen(syntax->cmnt_1ls))){
        memset(&row->hlchars[i], TK_COMMENT, row->size-i);
        break;
      //} else if(syntax->flags & HL_COMMENT
          //&& !strncmp(syntax->cmnt_blks, &row->renderchars[i], strlen(syntax->cmnt_blks))){
        // block comment processing
      } else if(c != ' '){
        setType(&row->hlchars[i], &row->renderchars[i], 1, syntax);
      }
      ci = i+1;
    }
  }
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

int rowRendertoCursorX(erow *row, int render_x){
  int rx = 0;
  for(int j=0; j<render_x; j++){
    if(row->chars[j] == '\t')
      rx += (settings.tabwidth-1) - (rx % settings.tabwidth);
    rx++;
    if(rx > render_x) return j;
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
  free(row->hlchars);
}

void rowAppendString(erow *row, char *s, size_t len){
  row->chars = realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = '\0';
  rowUpdate(row);
}
