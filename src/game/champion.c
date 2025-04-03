
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "constants.h"
#include "ability.h"
#include "champion.h"
#include "random_utils.h"
#include "hashmap.h"
#include "server.h"
#include "pawn.h"
#include "fight.h"

#define UNUSED(x) (void)(x)
#define CHECK(status, message) { if ((status) == -1) { perror(message); exit(EXIT_FAILURE); } }
#define CHECKM(status, message) { if ((status) == NULL) { perror(message); exit(EXIT_FAILURE); } }

void initChampion(champion_t* champion) {
	champion->type = 0;
	champion->powerBudget = 0;
	for (unsigned n = 0; n < EFFECT_COUNT; n++) { champion->effects[n] = 0; }
	for (unsigned n = 0; n < STAT_COUNT; n++) { champion->stats[n] = MAKE_STAT(0); }
	initHashmap(&champion->abilities);
	champion->hasInitiative = false;
	champion->fleeing = false;
}

void dropChampion(champion_t* champion) {
	for (unsigned n = 0; n < champion->abilities.capacity; n++) {
		ability_t* ability = champion->abilities.elements[n];
		if (ability == NULL) { continue; }
		dropAbility(ability);
		free(ability);
	}

	dropHashmap(&champion->abilities);
}

void setStats(
	champion_t* champion,
	unsigned attack, unsigned defense, unsigned magicAttack, unsigned magicDefense, unsigned intelligence,
	unsigned health
) {
	champion->stats[ATTACK] = MAKE_STAT(attack);
	champion->stats[DEFENSE] = MAKE_STAT(defense);
	champion->stats[MAGIC_ATTACK] = MAKE_STAT(magicAttack);
	champion->stats[MAGIC_DEFENSE] = MAKE_STAT(magicDefense);
	champion->stats[INTELLIGENCE] = MAKE_STAT(intelligence);
	champion->stats[HEALTH] = MAKE_STAT(health);
}

void replenishStats(champion_t* champion) {
	for (unsigned n = 0; n < STAT_COUNT; n++) {
		stat_value_t* stat = &champion->stats[n];
		stat->value = stat->maxValue;
	}
}

void addAbilityToChampion(champion_t* champion, ability_t* ability) {
	ability->championKey = hashmapLocateUnusedKey(&champion->abilities);
	hashmapSet(&champion->abilities, ability->championKey, ability);
	ability->champion = champion;
}

void removeAbilityToChampion(champion_t* champion, ability_t* ability) {
	hashmapSet(&champion->abilities, ability->championKey, NULL);
	ability->champion = NULL;
}

/** Crée des personnages définis. */
int setupExamples() {
    champion_t character1 = {
        .type = TYPE_COSMIC,
		.effects = { 0 },
		.stats = { MAKE_STATS(5, 6, 5, 3, 8, 10) },
		.abilities = BLANK_HASHMAP
    };
	initHashmap(&character1.abilities);

    champion_t character2 = {
        .type = TYPE_BLOOD,
		.effects = { 0 },
		.stats = { MAKE_STATS(7, 1, 9, 4, 5, 21) },
		.abilities = BLANK_HASHMAP
    };
	initHashmap(&character2.abilities);

    champion_t character3 = {
        .type = TYPE_DARK,
		.effects = { 0 },
		.stats = { MAKE_STATS(7, 4, 3, 8, 8, 11) },
		.abilities = BLANK_HASHMAP
    };
	initHashmap(&character3.abilities);

	UNUSED(character1); UNUSED(character2); UNUSED(character3);
    printf("setupExamples : Trois personnages créés.\n");
    return 0;
}

int compare(const void* a, const void* b) { return (*(int*)a - *(int*)b); }

void generateChampion(unsigned seed, unsigned powerBudget, champion_t* champion) {
	if (powerBudget < STAT_COUNT) { fprintf(stderr, "generateChampion: powerBudget lower than stat count"); exit(EXIT_FAILURE); }
	srand(seed);
	unsigned budgets[STAT_COUNT - 1];

	for (unsigned n = 0; n < STAT_COUNT - 1; n++) {
		budgets[n] = getRandomInt(0, powerBudget);
	}

	qsort(budgets, STAT_COUNT - 1, sizeof(unsigned), compare);
	champion->stats[0] = MAKE_STAT(budgets[0]);

	for (unsigned n = 1; n < STAT_COUNT - 1; n++) {
		champion->stats[n] = MAKE_STAT(budgets[n] - budgets[n - 1]);
	}

	champion->stats[STAT_COUNT - 1] = MAKE_STAT(powerBudget - budgets[STAT_COUNT - 2]);
	champion->powerBudget = powerBudget;
}

void setStat(stat_value_t* stat, int value) {
    if (value < stat->minValue) { stat->value = stat->minValue; return; }
    if (value > stat->maxValue) { stat->value = stat->maxValue; return; }
    stat->value = value;
}

bool isDead(champion_t* champion) {
	return champion->stats[HEALTH].value == 0;
}

void applyAbility(server_t* server, champion_t* source, champion_t* destination, ability_t* ability) {
	pawn_t* pawn = source->pawn;
	fight_t* fight = pawn->fight;

	if (fight != NULL) {
		champion_ability_event_args_t args = { .champion = source, .ability = ability };
		event_t event = MAKE_EVENT(EVENT_CHAMPION_ABILITY_USE, &args);
		notifyFight(server, fight, &event);
	}

    unsigned reader = 0;
    champion_t* target = destination;

    while (reader < MAX_ABILITY_OP_CODES) {
        unsigned opCode = ability->opCodes[reader];
        unsigned arg = 0;
        if (reader + 1 < MAX_ABILITY_OP_CODES) { arg = ability->opCodes[reader + 1]; }

        switch (opCode) {
            case NOOP:
                reader++;
                break;

            case SET_TARGET:
                if (arg == 0) { target = source; }
                if (arg == 1) { target = destination; }
                reader += 2;
                break;

            case SET_HEALTH:
                setStat(&target->stats[HEALTH], target->stats[HEALTH].value + arg);
                reader += 2;
                break;

            case ADD_EFFECT:
                target->effects[arg] = 1;
                reader += 2;
                break;

            case REM_EFFECT:
                target->effects[arg] = 0;
                reader += 2;
                break;

            case IF_EFFECT:
                if (!(target->effects[arg])) { return; }
                reader++;
                break;

            case IF_TYPE:
                if (target->type != arg) { return; }
                reader++;
                break;
        }
    }
}

void formatChampion(champion_t* champion, char* header, char* buffer, unsigned bufferSize) {
	char effectText[EFFECT_COUNT + 1];
	for (unsigned k = 0; k < EFFECT_COUNT; k++) { effectText[k] = champion->effects[k] ? 'Y':'N'; }
	unsigned recquiredSize = snprintf(
		buffer,
		bufferSize,
		"%s %i %s %i %i %i %i %s %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
		header,
		champion->pawnKey, champion->name, champion->type,
		champion->fleeing, champion->hasInitiative, champion->playedTurn,
		effectText,
		champion->stats[HEALTH].minValue, champion->stats[HEALTH].value, champion->stats[HEALTH].maxValue,
		champion->stats[INTELLIGENCE].minValue, champion->stats[INTELLIGENCE].value, champion->stats[INTELLIGENCE].maxValue,
		champion->stats[MAGIC_DEFENSE].minValue, champion->stats[MAGIC_DEFENSE].value, champion->stats[MAGIC_DEFENSE].maxValue,
		champion->stats[MAGIC_ATTACK].minValue, champion->stats[MAGIC_ATTACK].value, champion->stats[MAGIC_ATTACK].maxValue,
		champion->stats[DEFENSE].minValue, champion->stats[DEFENSE].value, champion->stats[DEFENSE].maxValue,
		champion->stats[ATTACK].minValue, champion->stats[ATTACK].value, champion->stats[ATTACK].maxValue,
		champion->powerBudget
	);

	if (recquiredSize > bufferSize) {
		fprintf(stderr, "buffer too small to handle champion description: %i, required: %i\n", bufferSize, recquiredSize);
		exit(EXIT_FAILURE);
	}
}

void makeSpider(champion_t* champion) {
	champion->name = "Araignee";
	champion->type = TYPE_BUG;
	setStats(champion, 5, 5, 5, 5, 10, 20);
}
