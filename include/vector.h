#pragma once

#include <stdlib.h>

typedef struct {
  unsigned char *vals;
  size_t _size;
  unsigned int len;
} vector;

void initVec(vector *vec);
void vecFree(vector *vec);
void vecPush(vector *vec, unsigned char val);
unsigned char vecPop(vector *vec);
