#include <ctype.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "utils.h"
#include "settings.h"

extern settingsType settings;

int getWindowSize_(int *rows, int *cols) {
  static struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
      return -1;
    return getCursorPosition(rows, cols);
  }
  *cols = ws.ws_col;
  *rows = ws.ws_row;
  return 0;
}

int getWindowSize(int *rows, int *cols){
  static struct winsize ws;

  if( ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0) return -1;
  *rows = ws.ws_row;
  *cols = ws.ws_col;
  return 0;
}

int getCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[')
    return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
    return -1;
  return 0;
}

void clearTerminal() {
  write(STDIN_FILENO, "\x1b[2J", 4);
  write(STDIN_FILENO, "\x1b[H", 3);
}

void die(const char *s) { // because func name:kill was already used
  clearTerminal();
  perror(s);
  exit(1);
}

int clamp(int value, int min, int max){
  if(max<min){
    int _t = min;
    min = max;
    max = _t;
  }
  if(value<min) return min;
  if(value>max) return max;
  return value;
}

int vec2areSame(vec2 v1, vec2 v2){
  return v1.x == v2.x && v1.y == v2.y;
}

int isSeparator(char c){
  return c==' ' || isPunct(c) || c == '\n' || c == '\0';
}

int isPunct(char c){
  return strchr("!$%&*+-./:<>=?@^|~#(){}[],;\"\\'", c) != NULL;
}

int isNumber(char *ch){
  int s = strlen(ch);
  for(int i=0; i<s; i++){
    if(!isdigit(ch[i])) return 0;
  }
  return 1;
}

int countTabs(char *str){
  int _count = 0, _sz=strlen(str);
  for(int i=0; i<_sz; i++){
    if(str[i]==' ') _count++;
    else if(str[i] == '\t') _count+=settings.tabwidth;
    else break;
  }
  return _count/settings.tabwidth;
}
