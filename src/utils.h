#pragma once

#define EDITOR_VERSION "0.0.1"

#define DEF_VEC2 (vec2){0,0}

typedef struct {
  int x;
  int y;
} vec2;

enum escapechars {
  NONE=0,
  DECIMAL,
  HEX,
  TERMINATE,
};

enum MODE {
  NORMAL = 0,
  INSERT,
  VISUAL
};

enum JUMP_FLAGS {
  JMP_END = 0x1,
  JMP_BACK = 0x2,
  JMP_PUNC = 0x4,
};

int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *x, int *y);
void clearTerminal();
void die(const char *s);

int clamp(int value, int min, int max);
int vec2areSame(vec2 v1, vec2 v2);

int isSeparator(char c);
int isPunct(char c);
int isNumber(char *ch);

int countTabs(char *str);
