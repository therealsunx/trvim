#include "editor.h"

int main(int argc, char* argv[]) {
  enableRawMode();
  initEditor();
  if(argc >= 2) editorOpen(argv[1]);

  while (1) {
    editorRefreshScreen();
    editorProcessKeyPress();
  }

  return 0;
}
