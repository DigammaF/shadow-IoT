
#ifndef CHAMPION_H
#define CHAMPION_H

#include <stdint.h>
#include <stdbool.h>
#include <stdbool.h>

#include "ability.h"
#include "constants.h"
#include "hashmap.h"

struct pawn_t;
struct server_t;

#define MAKE_STAT(n) (stat_value_t){ .value = 0, .maxValue = (n), .minValue = 0 }
#define MAKE_STATS(atk, dfe, matk, mdfe, intel, health) \
	{ .value = 0, .maxValue = atk, .minValue = 0 }, \
	{ .value = 0, .maxValue = dfe, .minValue = 0 }, \
	{ .value = 0, .maxValue = matk, .minValue = 0 }, \
	{ .value = 0, .maxValue = mdfe, .minValue = 0 }, \
	{ .value = 0, .maxValue = intel, .minValue = 0 }, \
	{ .value = 0, .maxValue = health, .minValue = 0 }

typedef struct {
    int value;
    int maxValue;
    int minValue;
} stat_value_t;

typedef enum { 
    ATTACK,
    DEFENSE,
    MAGIC_ATTACK,
    MAGIC_DEFENSE, 
    INTELLIGENCE, /** définit dans quel ordre les personnages agissent */
    HEALTH, 
    STAT_COUNT /** STAT_COUNT will be the number of elements stored in this enum, not counting itself */
} stat_t;

/** Structure représentant une entité qui possède des capacités et est utilisée pour le combat */
typedef struct champion_t {
	char* name;
	struct pawn_t* pawn; // peut être NULL
	unsigned pawnKey;
    champion_type_t type;
    int effects[EFFECT_COUNT];
    stat_value_t stats[STAT_COUNT]; /** dictionnaire qui prends des STAT en clés et a des stats_t comme valeurs */
    hashmap_t abilities; // ability_t* possède la valeur
	unsigned powerBudget; // le budget utilisé pour générer le champion
	bool fleeing; // indique si le champion est en train de fuir le combat
	bool hasInitiative; // indique si le champion est celui qui doit jouer son tour
	bool playedTurn; // indique si le champion a joué son tour
	unsigned fightKey;
} champion_t;

void initChampion(champion_t* champion);
void dropChampion(champion_t* champion);

void setStats(
	champion_t* champion,
	unsigned attack, unsigned defense, unsigned magicAttack, unsigned magicDefense, unsigned intelligence,
	unsigned health
);

void addAbilityToChampion(champion_t* champion, ability_t* ability);
void removeAbilityToChampion(champion_t* champion, ability_t* ability);

/** règle la valeur courante de la stat en respectant min et max */
void setStat(stat_value_t* stat, int value);
/** vérifie si le champion est mort */
bool isDead(champion_t* champion);
/** met toutes les stats à leur niveau max */
void replenishStats(champion_t* champion);

/**
 * génère un champion aléatoirement à partir d'une seed et d'un budget de puissance
 * le budget de puissance utilisé est inscrit dans le champ correspondant
 * */
void generateChampion(unsigned seed, unsigned powerBudget, champion_t* champion);
void applyAbility(struct server_t* server, champion_t* source, champion_t* destination, ability_t* ability);

/** créé le message (terminé par '\0') qui permet à un client de connaître le champion */
void formatChampion(champion_t* champion, char* header, char* buffer, unsigned bufferSize);

void makeSpider(champion_t* champion);

#endif
