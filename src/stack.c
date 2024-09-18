#include <stdlib.h>

#include "stack.h"
#include "utils.h"

void initStack(stack *stk){
  stk->_size=4;
  stk->vals = malloc(stk->_size * sizeof(int));
  stk->top = 0;
}

void freeStack(stack *stk){
  free(stk->vals);
  stk->_size = 0;
  stk->top = 0;
}

void emptyStack(stack *stk){
  stk->top = 0;
}

int isStackEmpty(stack *stk){
  return stk->top == 0;
}

void push(stack *stk, int value){
  if(stk->top == stk->_size){
    stk->_size *= 2;
    stk->vals = realloc(stk->vals, stk->_size);
  }
  stk->vals[stk->top++] = value;
}

int pop(stack *stk){
  if(stk->top == 0){
    die("stack underflow");
    return -1;
  }
  return stk->vals[--stk->top];
}
