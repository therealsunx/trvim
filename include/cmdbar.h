#pragma once

#include <time.h>

typedef struct {
  char msg[64];
  time_t msg_t;
} cmdbar_t;

void initCmdBar(cmdbar_t *cmdbar);
