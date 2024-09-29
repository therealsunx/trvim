#include "editor.h"

int main(int argc, char* argv[]) {
  enableRawMode();
  initEditor();
  if(argc >= 2) editorOpen(argv[1]);

  editorSetStatusMsg("Welcome to TRVIM");
  while (1) {
    editorStatusBarUpdate();
    editorRefreshScreen();
    editorProcessKeyPress();
  }
  return 0;
}
