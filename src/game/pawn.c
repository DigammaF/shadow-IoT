
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "hashmap.h"
#include "server.h"
#include "pawn.h"
#include "constants.h"
#include "champion.h"
#include "item.h"

#define UNUSED(x) (void)(x)

void initPawn(pawn_t* pawn) {
    pawn->exhaustion = 0;
	pawn->gold = 0;
    pawn->place = NULL;
    pawn->eventHandler = noHandler;
    pawn->account = NULL;
    pawn->user = NULL;
	pawn->fight = NULL;
	initHashmap(&pawn->champions);
	initHashmap(&pawn->items);
}

void dropPawn(pawn_t* pawn) {
	for (unsigned n = 0; n < pawn->champions.capacity; n++) {
		champion_t* champion = pawn->champions.elements[n];
		if (champion == NULL) { continue; }
		dropChampion(champion);
		free(champion);
	}

	for (unsigned n = 0; n < pawn->items.capacity; n++) {
		item_t* item = pawn->items.elements[n];
		if (item == NULL) { continue; }
		dropItem(item);
		free(item);
	}

	dropHashmap(&pawn->champions);
	dropHashmap(&pawn->items);
}

void addItemToPawn(pawn_t* pawn, item_t* item) {
	item->pawnKey = hashmapLocateUnusedKey(&pawn->items);
	hashmapSet(&pawn->items, item->pawnKey, item);
	item->pawn = pawn;
}

void removeItemFromPawn(pawn_t* pawn, item_t* item) {
	hashmapSet(&pawn->items, item->pawnKey, NULL);
	item->pawn = NULL;
}

void addChampionToPawn(pawn_t* pawn, champion_t* champion) {
	champion->pawnKey = hashmapLocateUnusedKey(&pawn->champions);
	hashmapSet(&pawn->champions, champion->pawnKey, champion);
	champion->pawn = pawn;
}

void removeChampionFromTeam(pawn_t* pawn, champion_t* champion) {
	for (unsigned n = 0; n < TEAM_SIZE; n++) {
		champion_t* localChampion = pawn->team[n];
		if (localChampion == NULL) { continue; }
		if (localChampion == champion) { pawn->team[n] = NULL; }
	}
}

void removeChampionFromPawn(pawn_t* pawn, champion_t* champion) {
	removeChampionFromTeam(pawn, champion);
	hashmapSet(&pawn->champions, champion->pawnKey, NULL);
	champion->pawn = NULL;
}

void notifySetContext(
	struct server_t* server, pawn_t* pawn,
	context_t context, function_t handler
) {
	setcontext_event_args_t args = { .context = context, .handler = handler };
	event_t event = MAKE_EVENT(EVENT_SETCONTEXT, &args);
	sendPawnEvent(server, pawn, &event);
}

void notifyPopContext(struct server_t* server, pawn_t* pawn) {
	popcontext_event_args_t args = { };
	event_t event = MAKE_EVENT(EVENT_POPCONTEXT, &args);
	sendPawnEvent(server, pawn, &event);
}

void notifyPawnUpdate(struct server_t* server, pawn_t* pawn) {
	update_event_args_t args = { };
	event_t event = MAKE_EVENT(EVENT_UPDATE, &args);
	sendPawnEvent(server, pawn, &event);
}

void notifyChampionAdded(server_t* server, pawn_t* pawn, champion_t* champion, char* reason) {
	champion_event_args_t args = { .champion = champion, .reason = reason };
	event_t event = MAKE_EVENT(EVENT_CHAMPION_GAINED, &args);
	sendPawnEvent(server, pawn, &event);
}

void notifyChampionRemoved(server_t* server, pawn_t* pawn, champion_t* champion, char* reason) {
	champion_event_args_t args = { .champion = champion, .reason = reason };
	event_t event = MAKE_EVENT(EVENT_CHAMPION_LOST, &args);
	sendPawnEvent(server, pawn, &event);
}

void notifyItemAdded(struct server_t* server, pawn_t* pawn, struct item_t* item, char* reason) {
	item_event_args_t args = { .item = item, .reason = reason };
	event_t event = MAKE_EVENT(EVENT_ITEM_GAINED, &args);
	sendPawnEvent(server, pawn, &event);
}

void notifyItemRemoved(struct server_t* server, pawn_t* pawn, struct item_t* item, char* reason) {
	item_event_args_t args = { .item = item, .reason = reason };
	event_t event = MAKE_EVENT(EVENT_ITEM_LOST, &args);
	sendPawnEvent(server, pawn, &event);
}

void sendPawnEvent(server_t* server, pawn_t* pawn, event_t* event) {
    event->server = server;
    event->pawn = pawn;
    pawn->eventHandler((void*)event);
}

void changePawnGold(server_t* server, pawn_t* pawn, int delta, char* reason) {
	pawn->gold += delta;
	char deltaRepr[1024];
	char sign = (delta >= 0) ? '+' : '-';
	sprintf(deltaRepr, "%c%i", sign, abs(delta));
	gold_changed_event_args_t args = { .delta = deltaRepr, .reason = reason };
	event_t event = MAKE_EVENT(EVENT_GOLD_CHANGED, &args);
	sendPawnEvent(server, pawn, &event);
}

void updatePawn(struct server_t* server, pawn_t* pawn) {
	int newExhaustion = pawn->exhaustion - EXHAUSTION_REFILL_RATE*server->deltaTime;
	pawn->exhaustion = newExhaustion < 0 ? 0 : newExhaustion;
	int newGracePeriod = pawn->gracePeriod - GRACE_PERIOD_DEPLETE_RATE*server->deltaTime;
	pawn->gracePeriod = newGracePeriod < 0 ? 0 : newGracePeriod;

	notifyPawnUpdate(server, pawn);
}
