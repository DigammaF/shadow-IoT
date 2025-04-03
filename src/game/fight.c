
#include <stdbool.h>

#include "random_utils.h"
#include "hashmap.h"
#include "fight.h"
#include "pawn.h"
#include "champion.h"
#include "server.h"
#include "user.h"

#define UNUSED(x) (void)(x)

void initFight(fight_t* fight) {
	initHashmap(&fight->pawns);
	initHashmap(&fight->champions);
}

void dropFight(fight_t* fight) {
	dropHashmap(&fight->pawns);
	dropHashmap(&fight->champions);
}

void addChampionToFight(fight_t* fight, struct champion_t* champion) {
	champion->fightKey = hashmapLocateUnusedKey(&fight->champions);
	hashmapSet(&fight->champions, champion->fightKey, champion);
}

void removeChampionFromFight(fight_t* fight, struct champion_t* champion) {
	hashmapSet(&fight->champions, champion->fightKey, NULL);
}

void addPawnToFight(fight_t* fight, struct pawn_t* pawn) {
	pawn->fightKey = hashmapLocateUnusedKey(&fight->pawns);
	hashmapSet(&fight->pawns, pawn->fightKey, pawn);
	pawn->fight = fight;
}

void removePawnFromFight(fight_t* fight, struct pawn_t* pawn) {
	hashmapSet(&fight->pawns, pawn->fightKey, NULL);
	pawn->fight = NULL;
}

bool turnEnded(fight_t* fight) {
	for (unsigned n = 0; n < fight->champions.capacity; n++) {
		champion_t* champion = fight->champions.elements[n];
		if (champion == NULL) { continue; }
		if (!champion->playedTurn) { return false; }
	}

	return true;
}

void makeFight(fight_t* fight, pawn_t* pawnA, pawn_t* pawnB) {
	addPawnToFight(fight, pawnA);
	addPawnToFight(fight, pawnB);

	for (unsigned n = 0; n < TEAM_SIZE; n++) {
		champion_t* championA = pawnA->team[n];
		champion_t* championB = pawnB->team[n];
		if (championA != NULL) { addChampionToFight(fight, championA); }
		if (championB != NULL) { addChampionToFight(fight, championB); }
	}

	for (unsigned n = 0; n < fight->champions.capacity; n++) {
		champion_t* champion = fight->champions.elements[n];
		if (champion == NULL) { continue; }
		champion->hasInitiative = false;
		champion->fleeing = false;
		champion->playedTurn = false;
	}
}

bool fightContainsChampion(fight_t* fight, champion_t* champion) {
	for (unsigned n = 0; n < fight->champions.capacity; n++) {
		champion_t* localChampion = fight->champions.elements[n];
		if (localChampion == NULL) { continue; }
		if (localChampion == champion) { return true; }
	}

	return false;
}

bool pawnCanOrderChampion(pawn_t* pawn, champion_t* champion) {
	return (champion->pawn == pawn);
}

void disband(fight_t* fight) {
	for (unsigned n = 0; n < fight->pawns.capacity; n++) {
		pawn_t* pawn = fight->pawns.elements[n];
		if (pawn == NULL) { continue; }
		removePawnFromFight(fight, pawn);
	}

	for (unsigned n = 0; n < fight->champions.capacity; n++) {
		champion_t* champion = fight->champions.elements[n];
		if (champion == NULL) { continue; }
		removeChampionFromFight(fight, champion);
	}
}

champion_t* giveInitiative(fight_t* fight) {
	champion_t* quickest = NULL;

	for (unsigned n = 0; n < fight->champions.capacity; n++) {
		champion_t* champion = fight->champions.elements[n];
		if (champion == NULL) { continue; }
		if (isDead(champion)) { continue; }
		if (quickest == NULL) { quickest = champion; continue; }
		if (champion->stats[INTELLIGENCE].value > quickest->stats[INTELLIGENCE].value) {
			quickest = champion;
		}
	}

	quickest->hasInitiative = true;
	return quickest;
}

bool anyChampionHasYetToPlay(fight_t* fight) {
	for (unsigned n = 0; n < fight->champions.capacity; n++) {
		champion_t* champion = fight->champions.elements[n];
		if (champion == NULL) { continue; }
		if (isDead(champion)) { continue; }
		if (!champion->playedTurn) { return true; }
	}

	return false;
}

bool initiativeChampionHasYetToPlay(fight_t* fight) {
	for (unsigned n = 0; n < fight->champions.capacity; n++) {
		champion_t* champion = fight->champions.elements[n];
		if (champion == NULL) { continue; }
		if (isDead(champion)) { continue; }
		if (!champion->playedTurn && champion->hasInitiative) { return true; }
	}

	return false;
}

void applyPawnRunaway(server_t* server, pawn_t* pawn) {
	pawn_event_args_t args = { .pawn = pawn };
	event_t event = MAKE_EVENT(EVENT_PAWN_RUNAWAY, &args);
	notifyFight(server, pawn->fight, &event);

	for (unsigned n = 0; n < TEAM_SIZE; n++) {
		champion_t* champion = pawn->team[n];
		if (champion == NULL) { continue; }
		if (isDead(champion)) {
			removeChampionFromPawn(pawn, champion);
			removeChampionFromTeam(pawn, champion);
			notifyChampionRemoved(server, pawn, champion, "mort au combat");
		}

		if (!champion->fleeing) {
			removeChampionFromPawn(pawn, champion);
			removeChampionFromTeam(pawn, champion);
			notifyChampionRemoved(server, pawn, champion, "abandonné au combat");
		}
	}
}

void applyTurn(struct server_t* server, fight_t* fight) {
	UNUSED(server);

	for (unsigned n = 0; n < fight->champions.capacity; n++) {
		champion_t* champion = fight->champions.elements[n];
		if (champion == NULL) { continue; }
		if (isDead(champion)) { continue; }
		champion->playedTurn = false;
	}
}

unsigned pawnValidChampionCount(fight_t* fight, struct pawn_t* pawn) {
	unsigned count = 0;

	for (unsigned n = 0; n < fight->champions.capacity; n++) {
		champion_t* champion = fight->champions.elements[n];
		if (champion == NULL) { continue; }
		if (!isDead(champion) && champion->pawn == pawn) { count++; }
	}

	return count;
}

void notifyFight(server_t* server, fight_t* fight, event_t* event) {
	for (unsigned n = 0; n < fight->pawns.capacity; n++) {
		pawn_t* pawn = fight->pawns.elements[n];
		if (pawn == NULL) { continue; }
		sendPawnEvent(server, pawn, event);
	}
}

void updateFight(struct server_t* server, fight_t* fight) {
	if (!initiativeChampionHasYetToPlay(fight)) {
		champion_t* champion = giveInitiative(fight);
		champion_event_args_t args = { .champion = champion, .reason = "meilleur intelligence" };
		event_t event = MAKE_EVENT(EVENT_CHAMPION_GET_INITIATIVE, &args);
		notifyFight(server, fight, &event);
	}

	if (!anyChampionHasYetToPlay(fight)) {
		applyTurn(server, fight);
	}

	for (unsigned n = 0; n < fight->pawns.capacity; n++) {
		pawn_t* pawn = fight->pawns.elements[n];
		if (pawn == NULL) { continue; }
		if (pawnValidChampionCount(fight, pawn) == 0) {
			applyPawnRunaway(server, pawn);
			removePawnFromFight(fight, pawn);
			notifyPopContext(server, pawn);
		}
	}

	if (hashmapValueCount(&fight->pawns) == 1) {
		disband(fight);
		removeFightFromWorld(&server->world, fight);
		dropFight(fight);
		free(fight);
	}
}
