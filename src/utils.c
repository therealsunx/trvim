#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "settings.h"
#include "utils.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

extern settings_t settings;

#ifdef _WIN32
int getWindowSize(int *rows, int *cols) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut == INVALID_HANDLE_VALUE) {
        return -1;
    }

    if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
        return -1;
    }

    *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    return 0;
}
#else
int getWindowSize(int *rows, int *cols) {
  static struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0)
    return -1;
  *rows = ws.ws_row;
  *cols = ws.ws_col;
  return 0;
}
#endif

int getCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R')
      break;
    i++;
  }
  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[')
    return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
    return -1;
  return 0;
}

void die(const char *s) { // because func name:kill was already used
  clearTerminal();
  perror(s);
  exit(1);
}

int max(int a, int b){
  return a>b?a:b;
}

int min(int a, int b){
  return a<b?a:b;
}

int clamp(int value, int min, int max) {
  if (max < min) {
    int _t = min;
    min = max;
    max = _t;
  }
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

int vec2areSame(vec2 v1, vec2 v2) { return v1.x == v2.x && v1.y == v2.y; }

int isSeparator(char c) {
  return c == ' ' || isPunct(c) || c == '\n' || c == '\0';
}

int isPunct(char c) {
  return strchr("!$%&*+-./:<>=?@^|~#(){}[],;\"\\'", c) != NULL;
}

int isNumber(char *ch) {
  int s = strlen(ch);
  for (int i = 0; i < s; i++) {
    if (!isdigit(ch[i]))
      return 0;
  }
  return 1;
}

int firstCharIndex(char *str) {
  int x = 0, _len = strlen(str);
  for (; x < _len; x++){
    if (str[x] != ' ' && str[x] != '\t')
      break;
  }

  return x;
}

int countTabs(char *str) {
  int _count = 0, _sz = strlen(str);
  for (int i = 0; i < _sz; i++) {
    if (str[i] == ' ')
      _count++;
    else if (str[i] == '\t')
      _count += settings.tabwidth;
    else
      break;
  }
  return _count / settings.tabwidth;
}

void showCursor(int y, int x) {
  moveCursor(y, x);
  write(STDOUT_FILENO, "\x1b[?25h", 6);
}

void hideCursor(void){
  write(STDOUT_FILENO, "\x1b[?25l", 6);
}

void moveCursor(int y, int x){
  char _cbuf[32];
  int l = snprintf(_cbuf, sizeof(_cbuf), "\x1b[%d;%dH", y+1, x+1);
  if (l == 0) die("cursor positioning failed");
  write(STDOUT_FILENO, _cbuf, l);
}

void resetCursor(void){
  write(STDOUT_FILENO, "\x1b[H", 3);
}

void thinCursor(void){
  write(STDOUT_FILENO, "\x1b[6 q", 5);
}
void thickCursor(void){
  write(STDOUT_FILENO, "\x1b[2 q", 5);
}

int checkPoint(boundstype bounds, vec2 point){
  if(point.y<bounds.start.y || 
      (point.y==bounds.start.y && point.x<bounds.start.x)) return BEFORE;

  if(point.y>bounds.end.y ||
      (point.y==bounds.end.y && point.x>bounds.end.x)) return AFTER;

  return INBOUND;
}

void clearTerminal(void){
  write(STDIN_FILENO, "\x1b[2J", 4);
  write(STDIN_FILENO, "\x1b[H", 3);
}

