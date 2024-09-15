#include "highlight.h"
#include <time.h>

char *C_FMATCH[] = {".c", ".h", ".cpp", ".hpp", NULL};
char *C_KEYWORDS[] = {
  "switch", "if", "while", "for", "break", "continue", "return", "else",
  "struct", "union", "typedef", "static", "enum", "class", "case",
  "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
  "void|", NULL
};

syntaxhl HLDB[] ={
  {
    "c",
    C_FMATCH,
    C_KEYWORDS,
    "//",
    //"/*", "*/",
    HL_NUMBERS|HL_KEYWORD|HL_PUNCTUATION|HL_STRING|HL_COMMENT
  },
};

int HLDB_SIZE = sizeof(HLDB)/sizeof(HLDB[0]);

int hlTokentoColorIndex(unsigned char hl) {
  switch (hl) {
    case TK_NUMBER:
      return 95;
    case TK_COMMENT:
      return 37;
    case TK_KEYWORD1:
      return 93;
    case TK_KEYWORD2:
      return 91;
    case TK_KEYWORD3:
      return 92;
    case TK_PUNCTUATION:
      return 33;
    case TK_STRING:
      return 32;
    case TK_MATCH:
      return 43;
    case TK_NORMAL:
    default:
      return 39;
  }
}
