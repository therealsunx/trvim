
typedef struct {
  int *vals;
  int top, _size;
} cmdstack;

typedef struct {
  int repx, cmd, arg1, arg2;
} parsedcmd_t;

void initStack(cmdstack *stk);
void freeStack(cmdstack *stk);
void emptyStack(cmdstack *stk);
int isStackEmpty(cmdstack *stk);
void push(cmdstack *stk, int value);
int pop(cmdstack *stk);

int getNumber(cmdstack *stk);
parsedcmd_t parseCommand(cmdstack *stk);
