
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lantern.h"
#include "command_handlers.h"
#include "user.h"
#include "server.h"
#include "account.h"
#include "constants.h"
#include "function_stack.h"
#include "string_utils.h"
#include "pawn.h"
#include "place.h"
#include "event_handlers.h"
#include "feature.h"
#include "item.h"
#include "champion.h"
#include "deal.h"
#include "fight.h"

#define UNUSED(x) (void)(x)

void* adminCommandhandler(void* arg) {
	UNUSED(arg);
	return NULL;
}

void* debugEchoCommandHandler(void* arg) {
	command_context_t* context = (command_context_t*)arg;
	user_t* user = context->user;
	char message[COMMUNICATION_SIZE];
	char* line = joinString(context->args, " ");
	printf("(received from %i) '%s'\n", user->id, line);
	sprintf(message, "OUTPUT '%s'", line);
	sendData(&user->socket, message);
	return NULL;
}
