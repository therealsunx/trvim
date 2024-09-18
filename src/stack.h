
typedef struct {
  int *vals;
  int top, _size;
} stack;

void initStack(stack *stk);
void freeStack(stack *stk);
void emptyStack(stack *stk);
int isStackEmpty(stack *stk);
void push(stack *stk, int value);
int pop(stack *stk);
