
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "stack.h"

#define CHECKM(status, message) { if ((status) == NULL) { perror(message); exit(EXIT_FAILURE); } }

void initStack(stack_t* stack) {
	stack->node = NULL;
}

void dropStack(stack_t* stack) {
	while (!stackEmpty(stack)) { pop(stack); }
}

int stackEmpty(const stack_t* stack) {
	return stack->node == NULL;
}

void push(stack_t* stack, const unsigned value) {
	stack_node_t* node = malloc(sizeof(stack_node_t));
	CHECKM(node, "malloc node");
	node->value = value;
	node->previous = (struct stack_node_t*)stack->node;
	stack->node = node;
}

unsigned pop(stack_t* stack) {
	stack_node_t* node = stack->node;
	stack->node = (stack_node_t*)node->previous;
	unsigned value = node->value;
	free(node);
	return value;
}

unsigned peek(const stack_t* stack) {
	return stack->node->value;
}
