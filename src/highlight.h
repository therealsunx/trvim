#pragma once

#define HL_NUMBERS (1<<0)
#define HL_KEYWORD (1<<1)
#define HL_PUNCTUATION (1<<2)
#define HL_COMMENT (1<<3)
#define HL_STRING (1<<4)

enum hlTokens {
  TK_IGNORE = 0,
  TK_NORMAL,
  TK_NUMBER,
  TK_KEYWORD,
  TK_COMMENT,
  TK_STRING,
  TK_PUNCTUATION,
  TK_MATCH,
};

typedef struct {
  char *filetype;
  char **extensions;
  int flags;
} syntaxhl;

int  hlTokentoColorIndex(unsigned char hl);
