
#ifndef USER_H
#define USER_H

#include <time.h>

#include "lantern.h"
#include "function_stack.h"
#include "vector.h"
#include "stack.h"
#include "constants.h"

struct server_t;
struct account_t;

typedef struct user_t {
	socket_t socket;
	struct account_t* account; // peut être NULL
	time_t lastActivity;
	unsigned id;
	function_stack_t commandHandlers;
	stack_t contexts;
	int running;
} user_t;

void initUser(user_t* user);
void dropUser(user_t* user);

void setUserContext(user_t* user, context_t context, function_t handler);
void popUserContext(user_t* user);

#endif
