#include "editor.h"

int main(int argc, char* argv[]) {
  enableRawMode();
  initEditor();
  if(argc >= 2) editorOpen(argv[1]);

  editorSetStatusMsg("Help: Ctrl-Z=quit | CTRL-S=Save");
  while (1) {
    editorRefreshScreen();
    editorProcessKeyPress();
  }

  return 0;
}
