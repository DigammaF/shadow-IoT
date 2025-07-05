
#ifndef INTERACTIVE_CLIENT_H
#define INTERACTIVE_CLIENT_H

#include <stdbool.h>

typedef struct {
	bool up; bool down; bool left; bool right;
	bool validate;
} input_t;

void scanUserInput(input_t* input);

#endif
