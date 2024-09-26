#pragma once

#define EDITOR_VERSION "0.0.1"

#define DEF_VEC2 (vec2){0,0}

typedef struct {
  int x;
  int y;
} vec2;

typedef struct {
  vec2 start, end;
  int dir;
} boundstype;

enum escapechars {
  NONE=0,
  DECIMAL,
  HEX,
  TERMINATE,
};

enum MODE {
  NORMAL = 0,
  INSERT,
  VISUAL,
  VISUAL_LINE,
};

enum JUMP_FLAGS {
  JMP_END = 0x1,
  JMP_BACK = 0x2,
  JMP_PUNC = 0x4,
};

enum CMD_TYPES {
  CMD_NONE = 0x0,
  CMD_NUM = 0x1,
  CMD_WRITE = 0x2,
  CMD_QUIT = 0x4,
  CMD_ALL = 0x8,
  CMD_FORCE = 0x10,
  CMD_ERROR = 0x20,
};

enum BOUND_STATE {
  BEFORE = -1,
  INBOUND = 0,
  AFTER = 1
};

enum VISUAL_DIR {
  FORWARD = 1,
  BACKWARD,
};

int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *x, int *y);
void die(const char *s);

int clamp(int value, int min, int max);
int max(int a, int b);
int min(int a, int b);
int vec2areSame(vec2 v1, vec2 v2);

int isSeparator(char c);
int isPunct(char c);
int isNumber(char *ch);

int firstCharIndex(char *str);
int countTabs(char *str);

void showCursor(int x, int y);
void moveCursor(int y, int x);
void hideCursor(void);
void resetCursor(void);
void thinCursor(void);
void thickCursor(void);

int checkPoint(boundstype bounds, vec2 point);
void clearTerminal(void);
