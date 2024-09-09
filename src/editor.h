#include <termios.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT {NULL, 0}
#define EDITOR_VERSION "0.0.1"

typedef struct {
  int screenRows, screenCols;
  struct termios org_termios;
} editorconf ;

typedef struct {
  char *buf;
  int len;
} abuf;

// -- append Buffer methods --
void abAppend(abuf *, const char *, int);

// --editor init functions --
void die(const char *s);
void disableRawMode();
void enableRawMode();
void initEditor();

// -- utils: getters --
int getWindowSize(int *, int *);
int getCursorPosition(int *, int *);

// -- editor prcs --
void editorClearScreen();
char editorReadKey();
void editorProcessKeyPress();
void editorDrawRows(abuf *);
void editorRefreshScreen();
