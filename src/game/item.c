
#include <stdlib.h>
#include <stdio.h>

#include "lantern.h"
#include "constants.h"
#include "item.h"
#include "pawn.h"
#include "place.h"
#include "user.h"
#include "champion.h"

#define CHECKM(status, message) { if ((status) == NULL) { perror(message); exit(EXIT_FAILURE); } }

void initItem(item_t* item) {
	item->params = NULL;
	item->useHandler = noHandler;
}

void dropItem(item_t* item) {
	if (item->params != NULL) { free(item->params); }
}

void makeSqueaker(item_t* item) {
	item->name = "Boite-a-couinement";
	item->params = malloc(sizeof(no_item_params_t));
	CHECKM(item->params, "malloc params");
	item->useHandler = squeakerUseHandler;
}

void triggerItemUse(struct server_t* server, item_t* item, use_t* use) {
	use->server = server;
	use->item = item;
	item->useHandler((void*)use);
	pawn_t* pawn = item->pawn;
	if (pawn == NULL) { return; }
	place_t* place = pawn->place;
	if (place == NULL) { return; }
	item_use_event_args_t args = { .use = use };
	event_t event = MAKE_EVENT(EVENT_ITEM_USE, &args);
	notifyPlace(server, place, &event);
}

void* squeakerUseHandler(void* _) {
	use_t* use = (use_t*)_;
	item_t* item = use->item;
	pawn_t* pawn = item->pawn;
	if (pawn == NULL) { return NULL; }
	user_t* user = pawn->user;
	if (user == NULL) { return NULL; }
	sendData(&user->socket, "OUTPUT *couinement*");

	if (use->type == USE_ON_CHAMPION) {
		champion_use_args_t* args = use->args;
		champion_t* champion = args->champion;
		char message[COMMUNICATION_SIZE];
		sprintf(message, "OUTPUT champion: %s", champion->name);
		sendData(&user->socket, message);
	}

	return NULL;
}
