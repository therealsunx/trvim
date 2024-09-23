#include <stdlib.h>

#include "cmdstack.h"
#include "utils.h"

void initStack(cmdstack *stk){
  stk->_size=4;
  stk->vals = malloc(stk->_size * sizeof(int));
  stk->top = 0;
}

void freeStack(cmdstack *stk){
  free(stk->vals);
  stk->_size = 0;
  stk->top = 0;
}

void emptyStack(cmdstack *stk){
  stk->top = 0;
}

int isStackEmpty(cmdstack *stk){
  return stk->top == 0;
}

void push(cmdstack *stk, int value){
  if(stk->top == stk->_size){
    stk->_size *= 2;
    stk->vals = realloc(stk->vals, stk->_size);
  }
  stk->vals[stk->top++] = value;
}

int pop(cmdstack *stk){
  if(stk->top == 0){
    die("cmdstack underflow");
    return -1;
  }
  return stk->vals[--stk->top];
}

int getNumber(cmdstack *stk){
  int n = 0;
  for(int i=0; i<stk->top; i++){
    int c = stk->vals[i];
    if(c>='0' && c <= '9') n = n*10+(c-'0');
    else break;
  }
  return n;
}

parsedcmd_t parseCommand(cmdstack *stk){
  parsedcmd_t pc = {0,0,0,0};

  int i=0, c=0;

  //special commands
  if(i<stk->top && stk->vals[i]=='0') {
    pc.cmd ='0';
    return pc;
  }

  // repx iteration
  for(;i<stk->top; i++){
    c = stk->vals[i];
    if(c>='0' && c <= '9') pc.repx = pc.repx*10+(c-'0');
    else break;
  }
  if(i<stk->top) pc.cmd = stk->vals[i++];
  if(i<stk->top) pc.arg1 = stk->vals[i++];
  if(i<stk->top) pc.arg2 = stk->vals[i++];

  return pc;
}
