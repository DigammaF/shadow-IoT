
#ifndef STACK_H
#define STACK_H

typedef struct {
	struct stack_node_t* previous;
	unsigned value;
} stack_node_t;

typedef struct {
	stack_node_t* node;
} stack_t;

void initStack(stack_t* stack);
void dropStack(stack_t* stack);
int stackEmpty(const stack_t* stack);
void push(stack_t* stack, const unsigned value);
unsigned pop(stack_t* stack);
unsigned peek(const stack_t* stack);

#endif
