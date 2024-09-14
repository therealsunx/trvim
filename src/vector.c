#include "vector.h"

void initVec(vector *vec){
  *vec = (vector){
    .vals = NULL,
    ._size = 0,
    .len = 0
  };
}
void vecFree(vector *vec){
  free(vec->vals);
}
void vecPush(vector *vec, unsigned char val){
  if(vec->len == vec->_size){
    vec->_size = vec->_size? vec->_size*2:4;
    vec->vals = realloc(vec->vals, vec->_size);
  }
  vec->vals[vec->len++] = val;
}
unsigned char vecPop(vector *vec){
  return vec->vals[--vec->len];
}
