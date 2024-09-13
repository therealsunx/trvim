#pragma once

#define EDITOR_VERSION "0.0.1"

#define DEF_VEC2 (vec2){0,0}

typedef struct {
  int x;
  int y;
} vec2;

int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *x, int *y);
void clearTerminal();
void die(const char *s);

