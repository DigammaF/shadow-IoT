
#include <stdlib.h>
#include <stdio.h>

#include "feature.h"
#include "constants.h"
#include "pawn.h"
#include "place.h"
#include "user.h"

#define CHECKM(status, message) { if ((status) == NULL) { perror(message); exit(EXIT_FAILURE); } }

#define UNUSED(x) (void)(x)

void initFeature(feature_t* feature) {
    feature->interactionHandler = noHandler;
}

void dropFeature(feature_t* feature) {
	UNUSED(feature);
}

void triggerFeatureInteraction(struct server_t* server, feature_t* feature, interaction_t* interaction) {
	interaction->feature = feature;
	interaction->server = server;
	feature->interactionHandler((void*)interaction);
	place_t* place = feature->place;
	if (place == NULL) { return; }
	interaction_event_args_t args = { .interaction = interaction };
	event_t event = MAKE_EVENT(EVENT_INTERACTION, &args);
	notifyPlace(server, place, &event);
}

void makeRock(feature_t* feature) {
    feature->name = "Rocher";
	feature->interactionHandler = debuginteractionHandler;
}

void makeBush(feature_t* feature) {
    feature->name = "Buisson";
	feature->interactionHandler = debuginteractionHandler;
}

void* debuginteractionHandler(void* _) {
	interaction_t* interaction = (interaction_t*)_;
	pawn_interaction_args_t* pawnInteractionArgs = NULL;
	printf("in handler\n");

	if (interaction->type == INTERACTION_HELP) {
		pawnInteractionArgs = interaction->args;
		pawn_t* pawn = pawnInteractionArgs->pawn;
		user_t* user = pawn->user;
		if (user == NULL) { return NULL; }
		sendData(&user->socket, "Successfully triggered feature");
	}

	return NULL;
}
