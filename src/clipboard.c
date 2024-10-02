#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clipboard.h"

#ifdef _WIN32

#include <windows.h>

void setClipBoard(const char *text) {
  if (OpenClipboard(NULL)) {
    EmptyClipboard();
    size_t len = strlen(text) + 1;
    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, len);
    if (hGlob) {
      memcpy(GlobalLock(hGlob), text, len);
      GlobalUnlock(hGlob);
      SetClipboardData(CF_TEXT, hGlob);
    }
    CloseClipboard();
  }
}

char *getClipBoard(void) {
  if (!OpenClipboard(NULL)) {
    return NULL;
  }
  HANDLE hData = GetClipboardData(CF_TEXT);
  if (hData == NULL) {
    CloseClipboard();
    return NULL;
  }
  char *clipboardText = GlobalLock(hData);
  char *result = strdup(clipboardText);
  GlobalUnlock(hData);
  CloseClipboard();
  return result;
}

#elif defined(__APPLE__)

void setClipBoard(const char *text) {
  FILE *pipe = popen("pbcopy", "w");
  if (pipe) {
    fwrite(text, sizeof(char), strlen(text), pipe);
    pclose(pipe);
  }
}

char *getClipBoard(void) {
  char *clipboardText = NULL;
  char buffer[128];
  FILE *pipe = popen("pbpaste", "r");
  if (pipe) {
    size_t len = 0;
    while (fgets(buffer, sizeof(buffer), pipe)) {
      size_t blen = strlen(buffer);
      clipboardText = realloc(clipboardText, len + blen + 1);

      if (!clipboardText) {
        pclose(pipe);
        return NULL;
      }
      memcpy(clipboardText + len, buffer, blen);
      len += blen;
      clipboardText[len] = '\0';
    }
    pclose(pipe);
  }
  return clipboardText;
}

#else
void setClipBoard(const char *text) {
  FILE *pipe = popen("xclip -selection clipboard", "w");
  if (pipe) {
    fwrite(text, sizeof(char), strlen(text), pipe);
    pclose(pipe);
  }
}

char *getClipBoard(void) {
  char *clipboardText = NULL;
  char buffer[128];
  FILE *pipe = popen("xclip -selection clipboard -o", "r");
  if (!pipe)
    return NULL;

  size_t len = 0;
  while (fgets(buffer, sizeof(buffer), pipe)) {
    size_t blen = strlen(buffer);
    clipboardText = realloc(clipboardText, len + blen + 1);

    if (!clipboardText) {
      pclose(pipe);
      return NULL;
    }
    memcpy(clipboardText + len, buffer, blen);
    len += blen;
    clipboardText[len] = '\0';
  }
  pclose(pipe);
  return clipboardText;
}
#endif
