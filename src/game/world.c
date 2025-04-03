
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "world.h"
#include "random_utils.h"
#include "hashmap.h"
#include "server.h"
#include "place.h"
#include "pawn.h"
#include "deal.h"
#include "fight.h"

#define UNUSED(x) (void)(x)
#define CHECKM(status, message) { if ((status) == NULL) { perror(message); exit(EXIT_FAILURE); } }

void initWorld(world_t* world) {
	for (unsigned x = 0; x < WORLD_SIZE; x++) {
		for (unsigned y = 0; y < WORLD_SIZE; y++) {
			world->places[x][y] = NULL;
		}
	}

	initHashmap(&world->pawns);
	initHashmap(&world->championDeals);
	initHashmap(&world->itemDeals);
	initHashmap(&world->fights);
}

void dropWorld(world_t* world) {
	for (unsigned x = 0; x < WORLD_SIZE; x++) {
		for (unsigned y = 0; y < WORLD_SIZE; y++) {
			place_t* place = world->places[x][y];
			if (place == NULL) { continue; }
			dropPlace(place);
			free(place);
		}
	}

	for (unsigned n = 0; n < world->pawns.capacity; n++) {
		pawn_t* pawn = world->pawns.elements[n];
		if (pawn == NULL) { continue; }
		dropPawn(pawn);
		free(pawn);
	}

	for (unsigned n = 0; n < world->championDeals.capacity; n++) {
		champion_deal_t* championDeal = world->championDeals.elements[n];
		if (championDeal == NULL) { continue; }
		dropChampionDeal(championDeal);
		free(championDeal);
	}

	for (unsigned n = 0; n < world->itemDeals.capacity; n++) {
		item_deal_t* itemDeal = world->itemDeals.elements[n];
		if (itemDeal == NULL) { continue; }
		dropItemDeal(itemDeal);
		free(itemDeal);
	}

	for (unsigned n = 0; n < world->fights.capacity; n++) {
		fight_t* fight = world->fights.elements[n];
		if (fight == NULL) { continue; }
		dropFight(fight);
		free(fight);
	}

	dropHashmap(&world->pawns);
	dropHashmap(&world->championDeals);
	dropHashmap(&world->itemDeals);
	dropHashmap(&world->fights);
}

void connectToNeighbors(world_t* world, unsigned x, unsigned y) {
	place_t* place = world->places[x][y];

	for (int dx = -1; dx <= 1; dx++) {
		for (int dy = -1; dy <= 1; dy++) {
			if (dx == 0 && dy == 0) { continue; }
			//if (getRandomIntInclusive(1, 3) == 1) { continue; }
			int twx = ((int)x + dx);
			int twy = ((int)y + dy);
			if (twx < 0) { twx += WORLD_SIZE; }
			if (twy < 0) { twy += WORLD_SIZE; }
			if (twx >= WORLD_SIZE) { twx -= WORLD_SIZE; }
			if (twy >= WORLD_SIZE) { twy -= WORLD_SIZE; }
			unsigned wx = twx;
			unsigned wy = twy;
			char* name;
			if (dx == 0 && dy > 0) {
				name = "Nord";
			} else if (dx == 0 && dy < 0) {
				name = "Sud";
			} else if (dx > 0 && dy == 0) {
				name = "Est";
			} else if (dx < 0 && dy == 0) {
				name = "Ouest";
			} else if (dx > 0 && dy > 0) {
				name = "Nord-Est";
			} else if (dx > 0 && dy < 0) {
				name = "Sud-Est";
			} else if (dx < 0 && dy > 0) {
				name = "Nord-Ouest";
			} else if (dx < 0 && dy < 0) {
				name = "Sud-Ouest";
			}
			place_t* destination = world->places[wx][wy];
			createLink(place, destination, name);
		}
	}
}

void addPawnToWorld(world_t* world, pawn_t* pawn) {
	pawn->worldKey = hashmapLocateUnusedKey(&world->pawns);
	hashmapSet(&world->pawns, pawn->worldKey, pawn);
}

void removePawnFromWorld(world_t* world, pawn_t* pawn) {
	hashmapSet(&world->pawns, pawn->worldKey, NULL);
}

void generateWorld(unsigned seed, world_t* world) {
	srand(seed);
	printf("(...) Generating world\n");

	for (unsigned x = 0; x < WORLD_SIZE; x++) {
		printf(".");
		for (unsigned y = 0; y < WORLD_SIZE; y++) {
			place_t* place = malloc(sizeof(place_t));
			CHECKM(place, "malloc place_t");
			initPlace(place);

			if (getRandomIntInclusive(0, 1)) {
				makePlain(place);
			} else {
				makeDesert(place);
			}

			world->places[x][y] = place;
			char name[200];
			sprintf(name, "%s(%i;%i)", place->name, x, y);
			place->name = strdup(name);
		}
	}

	printf("\n");

	for (unsigned x = 0; x < WORLD_SIZE; x++) {
		for (unsigned y = 0; y < WORLD_SIZE; y++) {
			connectToNeighbors(world, x, y);
		}
	}
}

void spawnPawn(server_t* server, place_t* place, pawn_t* pawn) {
	addPawnToWorld(&server->world, pawn);
	addPawnToPlace(place, pawn);
	notifyPawnSpawned(server, place, pawn);
}

void addChampionDealToWorld(world_t* world, champion_deal_t* deal) {
	deal->key = hashmapLocateUnusedKey(&world->championDeals);
	hashmapSet(&world->championDeals, deal->key, deal);
}

void removeChampionDealFromWorld(world_t* world, champion_deal_t* deal) {
	hashmapSet(&world->championDeals, deal->key, NULL);
}

void addItemDealToWorld(world_t* world, item_deal_t* deal) {
	deal->key = hashmapLocateUnusedKey(&world->itemDeals);
	hashmapSet(&world->itemDeals, deal->key, deal);
}

void removeItemDealFromWorld(world_t* world, item_deal_t* deal) {
	hashmapSet(&world->itemDeals, deal->key, NULL);
}

void addFightToWorld(world_t* world, fight_t* fight) {
	fight->key = hashmapLocateUnusedKey(&world->fights);
	hashmapSet(&world->fights, fight->key, fight);
}

void removeFightFromWorld(world_t* world, fight_t* fight) {
	hashmapSet(&world->fights, fight->key, NULL);
}

void updateWorld(struct server_t* server, world_t* world) {
	for (unsigned n = 0; n < world->pawns.capacity; n++) {
		pawn_t* pawn = world->pawns.elements[n];
		if (pawn == NULL) { continue; }
		updatePawn(server, pawn);
	}

	for (unsigned n = 0; n < world->fights.capacity; n++) {
		fight_t* fight = world->fights.elements[n];
		if (fight == NULL) { continue; }
		updateFight(server, fight);
	}
}
