#include "editor.h"

// test comment
    // test comment
int main(int argc, char* argv[]) { // test comment
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
