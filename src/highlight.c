#include "highlight.h"
#include <time.h>

char *C_FMATCH[] = {".c", ".h", ".cpp", ".hpp", NULL};
char *C_KEYWORDS[] = {
  "switch", "if", "while", "for", "break", "continue", "return", "else",
  "struct", "union", "typedef", "static", "enum", "class", "case", "extern", "default",
  "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
  "void|", 
  NULL
};
char *C_PREPROC[] = {
  "include", "define", "undef", "ifdef", "if", "else", "elif", "endif",
  "error", "pragma", NULL
};

syntaxhl HLDB[] ={
  {
    "c",
    C_FMATCH,
    C_KEYWORDS,
    C_PREPROC,
    '#',
    "//",
    HL_NUMBERS|HL_KEYWORD|HL_PUNCTUATION|HL_STRING|HL_COMMENT|HL_PREPROC
  },
};

int HLDB_SIZE = sizeof(HLDB)/sizeof(HLDB[0]);

int hlTokentoColorIndex(unsigned char hl) {
  switch (hl) {
    case TK_NUMBER:
      return 206; //95;
    case TK_COMMENT:
      return 244; //37;
    case TK_KEYWORD1:
      return 202; //91;
    case TK_KEYWORD2:
      return 220; //93;
    case TK_KEYWORD3:
      return 142; //92;
    case TK_KEYWORD4:
      return 64; //94;
    case TK_PREPROC:
      return 71; //36;
    case TK_PUNCTUATION:
      return 166; //33;
    case TK_STRING:
      return 106; //32;
    case TK_MATCH:
      return 227; //43;
    case TK_NORMAL:
    default:
      return 250; //39;
  }
}
