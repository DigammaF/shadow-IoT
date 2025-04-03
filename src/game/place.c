
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "link.h"
#include "place.h"
#include "hashmap.h"
#include "vector.h"
#include "pawn.h"
#include "random_utils.h"

struct server_t;
struct event_t;

#define CHECKM(status, message) { if ((status) == NULL) { perror(message); exit(EXIT_FAILURE); } }
#define UNUSED(x) (void)(x)

void initPlace(place_t* place) {
    initHashmap(&place->pawns);
    initHashmap(&place->features);
	initHashmap(&place->links);
}

void dropPlace(place_t* place) {
	for (unsigned n = 0; n < place->links.capacity; n++) {
		link_t* link = place->links.elements[n];
		if (link == NULL) { continue; }
		dropLink(link);
		free(link);
	}

    dropHashmap(&place->pawns);
    dropHashmap(&place->features);
	dropHashmap(&place->links);
}

void initLink(link_t* link) {
	link->target = NULL;
}

void dropLink(link_t* link) { UNUSED(link); }

link_t* createLink(place_t* source, place_t* destination, char* name) {
	link_t* link = malloc(sizeof(link_t));
	CHECKM(link, "malloc link");
	link->target = destination;
	link->name = strdup(name);
	link->id = hashmapLocateUnusedKey(&source->links);
	hashmapSet(&source->links, link->id, link);
	return link;
}

void deleteLink(place_t* place, unsigned key) {
	hashmapSet(&place->links, key, NULL);
}

void makePlain(place_t* place) {
    place->name = "Plaine";
	
	unsigned bushCount = getRandomInt(2, 6);

	for (unsigned n = 0; n < bushCount; n++) {
		feature_t* feature = malloc(sizeof(feature_t));
		CHECKM(feature, "malloc feature");
		initFeature(feature);
		makeBush(feature);
		addFeatureToPlace(place, feature);
	}
}

void makeDesert(place_t* place) {
    place->name = "Désert";

	unsigned rockCount = getRandomInt(2, 6);

	for (unsigned n = 0; n < rockCount; n++) {
		feature_t* feature = malloc(sizeof(feature_t));
		CHECKM(feature, "malloc feature");
		initFeature(feature);
		makeRock(feature);
		addFeatureToPlace(place, feature);
	}
}

void addPawnToPlace(place_t* place, pawn_t* pawn) {
	pawn->placeKey = hashmapLocateUnusedKey(&place->pawns);
	hashmapSet(&place->pawns, pawn->placeKey, pawn);
	pawn->place = place;
}

void removePawnFromPlace(place_t* place, pawn_t* pawn) {
	hashmapSet(&place->pawns, pawn->placeKey, NULL);
	pawn->place = NULL;
}

void addFeatureToPlace(place_t* place, struct feature_t* feature) {
	feature->placeKey = hashmapLocateUnusedKey(&place->features);
	hashmapSet(&place->features, feature->placeKey, feature);
	feature->place = place;
}

void removeFeatureFromPlace(place_t* place, struct feature_t* feature) {
	hashmapSet(&place->features, feature->placeKey, NULL);
	feature->place = NULL;
}

void notifyPlace(struct server_t* server, place_t* place, struct event_t* event) {
	for (unsigned n = 0; n < place->pawns.capacity; n++) {
		pawn_t* localPawn = place->pawns.elements[n];
		if (localPawn == NULL) { continue; }
		sendPawnEvent(server, localPawn, event);
	}
}

void notifyPawnLeft(struct server_t* server, place_t* place, pawn_t* pawn) {
	pawn_event_args_t args = { .pawn = pawn };
	event_t event = MAKE_EVENT(EVENT_PAWN_LEFT, &args);
	notifyPlace(server, place, &event);
}

void notifyPawnArrived(struct server_t* server, place_t* place, pawn_t* pawn) {
	pawn_event_args_t args = { .pawn = pawn };
	event_t event = MAKE_EVENT(EVENT_PAWN_ARRIVED, &args);
	notifyPlace(server, place, &event);
}

void notifyPawnSpawned(struct server_t* server, place_t* place, pawn_t* pawn) {
	pawn_event_args_t args = { .pawn = pawn };
	event_t event = MAKE_EVENT(EVENT_PAWN_SPAWNED, &args);
	notifyPlace(server, place, &event);
}

void movePawn(struct server_t* server, pawn_t* pawn, link_t* link) {
	place_t* place = pawn->place;

	if (place != NULL) {
		removePawnFromPlace(place, pawn);
		notifyPawnLeft(server, place, pawn);
	}

	place = link->target;
	addPawnToPlace(place, pawn);
	notifyPawnArrived(server, place, pawn);
}
