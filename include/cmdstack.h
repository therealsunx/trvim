#pragma once

enum CMD_STATE{
  ST_NOOP = 0,
  ST_ERR,
  ST_WAIT,
  ST_SUCCESS
};

typedef struct {
  int *vals;
  int top, _size;
} cmdstack_t;

typedef struct {
  int repx, cmd, arg1, arg2;
} parsedcmd_t;

void initStack(cmdstack_t *stk);
void freeStack(cmdstack_t *stk);
void emptyStack(cmdstack_t *stk);
int isStackEmpty(cmdstack_t *stk);
void push(cmdstack_t *stk, int value);
int pop(cmdstack_t *stk);

int getNumber(cmdstack_t *stk);
parsedcmd_t parseCommand(cmdstack_t *stk);
