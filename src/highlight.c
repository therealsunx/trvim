#include "highlight.h"
#include <time.h>

char *C_EXTNS[] = {".c", ".h", ".cpp", ".hpp", NULL};

syntaxhl HLDB[] ={
  {
    "c",
    C_EXTNS,
    HL_NUMBERS|HL_KEYWORD|HL_PUNCTUATION|HL_STRING|HL_COMMENT
  },
};

int hlTokentoColorIndex(unsigned char hl) {
  switch (hl) {
    case TK_NUMBER:
      return 35;
    case TK_COMMENT:
      return 37;
    case TK_KEYWORD:
      return 93;
    case TK_PUNCTUATION:
      return 91;
    case TK_STRING:
      return 32;
    case TK_MATCH:
      return 43;
    case TK_NORMAL:
    default:
      return 39;
  }
}
