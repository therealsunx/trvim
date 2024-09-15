#pragma once

#define HL_NUMBERS (1<<0)
#define HL_PREPROC (1<<1)
#define HL_KEYWORD (1<<2)
#define HL_PUNCTUATION (1<<3)
#define HL_COMMENT (1<<4)
#define HL_STRING (1<<5)

enum hlTokens {
  TK_IGNORE = 0,
  TK_NORMAL,
  TK_NUMBER,
  TK_KEYWORD1,
  TK_KEYWORD2,
  TK_KEYWORD3, // functions
  TK_KEYWORD4, // members
  TK_PREPROC,
  TK_COMMENT,
  TK_STRING,
  TK_PUNCTUATION,
  TK_MATCH,
};

typedef struct {
  char *filetype;
  char **filematch;
  char **keywords;
  char **preprocs;
  char preprocbeg;
  char *cmnt_1ls;
  int flags;
} syntaxhl;

int  hlTokentoColorIndex(unsigned char hl);
