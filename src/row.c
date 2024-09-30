#include <stdlib.h>
#include <string.h>

#include "row.h"
#include "settings.h"
#include "highlight.h"
#include "utils.h"

extern settings_t settings;

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

int setType(unsigned char* hlch, char *str, int len, syntaxhl *syn) {
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
    for(int j=0; syn->keywords[j]; j++){
      int _klen = strlen(syn->keywords[j]);
      int _k2 = syn->keywords[j][_klen-1] == '|';
      if(_k2) _klen--;

      if(len==_klen && !strncmp(_sstr, syn->keywords[j], _klen)){
        _tk = _k2?TK_KEYWORD2:TK_KEYWORD1;
        break;
      }
    }
    if(_tk == TK_NORMAL) return 0;
  }

  memset(hlch, _tk, len);
  return 1;
}

void rowUpdateSyntax(erow *row, syntaxhl *syntax){

  free(row->hlchars);
  row->hlchars = malloc(row->rsize);
  memset(row->hlchars, TK_NORMAL, row->rsize);

  if(!syntax) return;

  int in_str = 0;
  for (int i = 0, ci = 0; i < row->rsize; i++) {

    char c = row->renderchars[i];
    if(isSeparator(c) || i+1==row->size){
      if(in_str){
        if(c == in_str){
          in_str = 0;
          memset(&row->hlchars[ci], TK_STRING, i-ci+1);
          ci = i+1;
          continue;
        } else if(c == '\\') { // escape chars check
          int _esclen = 1;
          int _tp = NONE;

          if(i+1 == row->rsize){
            memset(&row->hlchars[ci], TK_STRING, i-ci);
            memset(&row->hlchars[i], TK_PUNCTUATION, 1);
          }else{
            char _nc;
            for(int _j=1; _j<=3 && _j+i<row->rsize; _j++){
              _nc = row->renderchars[_j+i];
              if(_tp==NONE){
                  _esclen++;
                  if(_nc>='0' && _nc<='9') _tp = DECIMAL;
                  else if(_nc == 'x') _tp = HEX;
                  else break;
              }else if(_nc == '"' || _nc == '\''){
                in_str = 0;
                memset(&row->hlchars[ci], TK_STRING, i-ci);
                memset(&row->hlchars[i], TK_PUNCTUATION, _esclen);
                memset(&row->hlchars[i+_esclen], TK_STRING, 1);
                i += _esclen;
                ci = i+1;
                _tp = TERMINATE;
                break;
              }else if(_tp==DECIMAL){
                if(_nc>='0' && _nc<='9') _esclen++;
                else break;
              } else if(_tp == HEX){
                if(_nc>='0' && _nc<='9') _esclen++;
                else if(_nc>='a' && _nc<='f') _esclen++;
                else break;
              }
            }
          }
          if(_tp == TERMINATE) continue;
          memset(&row->hlchars[ci], TK_STRING, i-ci);
          memset(&row->hlchars[i], TK_PUNCTUATION, _esclen);
          i += _esclen-1;
          ci = i+1;
          continue;
        } if(i+1==row->rsize){
          memset(&row->hlchars[ci], TK_STRING, i-ci);
        }else continue;
      } else if(ci != i){
        if(syntax->flags & HL_PREPROC && ci>0 && row->renderchars[ci-1] == syntax->preprocbeg){
          int __hl = 0, _len = i-ci;
          for(int j=0; syntax->preprocs[j]; j++){
            int _klen  = strlen(syntax->preprocs[j]);
            if(_klen == _len &&  !strncmp(&row->renderchars[ci], syntax->preprocs[j], _klen)){
              memset(&row->hlchars[ci-1], TK_PREPROC, row->size-ci+1);
              __hl++;
              break;
            }
          }
          if(!__hl) memset(&row->hlchars[ci], TK_NORMAL, i-ci);
          else break;
        } else if(!setType(&row->hlchars[ci], &row->renderchars[ci], i-ci, syntax)
            && (syntax->flags & HL_KEYWORD)){
          if(c=='(')
            memset(&row->hlchars[ci], TK_KEYWORD3, i-ci);
          else if(ci>0 && row->renderchars[ci-1]=='.')
            memset(&row->hlchars[ci], TK_KEYWORD4, i-ci);
          else 
            memset(&row->hlchars[ci], TK_NORMAL, i-ci);
        };
      }

      if(syntax->flags & HL_COMMENT
          && !strncmp(syntax->cmnt_1ls, &row->renderchars[i], strlen(syntax->cmnt_1ls))){
        memset(&row->hlchars[i], TK_COMMENT, row->rsize-i);
        break;
      } else if((syntax->flags & HL_STRING)
          && (c == '"' || c == '\'' )){
        in_str = c;
        ci = i;
        continue;;
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

int rowReplaceCharacter(erow *row, char ch_, int start, int len){
  len = clamp(len, 0, row->size-start);
  if(!len) return 0;
  memset(&row->chars[start], ch_, len);
  return 1;
}

void rowFree(erow *row){
  free(row->chars);
  free(row->renderchars);
  free(row->hlchars);
}

void rowDeleteCharacters(erow *row, int start, int len){
  if(start<0 || start >= row->size) return;
  if(start+len >= row->size) len = row->size-start;
  memmove(&row->chars[start], &row->chars[start+len], row->size-start-len+1);
  row->size-=len;
  rowUpdate(row);
}

void rowAppendString(erow *row, char *s, size_t len){
  row->chars = realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = '\0';
  rowUpdate(row);
}

