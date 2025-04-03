
#ifndef FUNCTION_STACK_H
#define FUNCTION_STACK_H

typedef void* (*function_t)(void*);

typedef struct {
	struct function_stack_node_t* previous;
	function_t value;
} function_stack_node_t;

typedef struct {
	function_stack_node_t* node; //topNode
	unsigned size;
} function_stack_t;

void initFunctionStack(function_stack_t* stack);
void dropFunctionStack(function_stack_t* stack);
int functionStackEmpty(function_stack_t* stack);
void pushFunction(function_stack_t* stack, function_t value);
function_t popFunction(function_stack_t* stack);
function_t peekFunction(const function_stack_t* stack);
function_t peekNFunction(const function_stack_t* stack, unsigned offset);

#endif
