
#ifndef PAWN_H
#define PAWN_H

#include <stdbool.h>

#include "hashmap.h"
#include "feature.h"
#include "function_stack.h"
#include "item.h"
#include "constants.h"

#define TEAM_SIZE 3

struct account_t;
struct user_t;
struct server_t;
struct place_t;
struct interaction_t;
struct item_t;
struct champion_t;
struct use_t;
struct fight_t;
struct ability_t;

typedef struct pawn_t {
    char* name;
    unsigned exhaustion; // un nombre non nul représente la présence de fatigue
	unsigned gracePeriod; // un nombre non nul représente la présence d'une période de grâce
    struct place_t* place;
    function_t eventHandler; // void* eventHandler(event_t*)
    struct account_t* account; // possiblement NULL si entité non joueur
    struct user_t* user; // possiblement NULL si le joueur n'est pas connecté
	unsigned placeKey; // la clé de l'entité dans le dictionnaire .pawns de la place_t dans laquelle elle se situe
	unsigned worldKey; // la clé de l'entité dans le dictionnaire .pawns de world_t
	unsigned fightKey;
	unsigned gold;
	hashmap_t champions; // collection de tous les champions possédés, champion_t*, possède la valeur
	hashmap_t items; // item_t*, possède la valeur
	struct champion_t* team[TEAM_SIZE]; // champions utilisés pour le combat, peut être NULL
	struct fight_t* fight; // peut être NULL
	void* properties; // structure qui regroupe les données nécessaires à la différenciation de fonction
} pawn_t;

/** si le verbe se termine par ED, l'évènement est envoyé après l'effet. sinon, il est envoyé avant */
typedef enum {
	EVENT_SETCONTEXT, // setcontext_event_args_t
	EVENT_POPCONTEXT, // popcontext_event_args_t
	EVENT_UPDATE, // update_event_args_t
    EVENT_PAWN_ARRIVED, // pawn_event_args_t
    EVENT_PAWN_LEFT, // pawn_event_args_t
	EVENT_PAWN_SPAWNED, // pawn_event_args_t
	EVENT_MESSAGE, // message_event_args_t
	EVENT_INTERACTION, // interaction_event_args_t
	EVENT_ITEM_USE, // item_use_event_args_t
	EVENT_GOLD_CHANGED, // gold_changed_event_args_t
	EVENT_CHAMPION_GAINED, // champion_event_args_t
	EVENT_CHAMPION_LOST, // champion_event_args_t
	EVENT_ITEM_GAINED, // item_event_args_t
	EVENT_ITEM_LOST, // item_event_args_t
	EVENT_CHAMPION_ABILITY_USE, // champion_ability_event_args_t
	EVENT_CHAMPION_FLEE, // champion_event_args_t
	EVENT_CHAMPION_GET_INITIATIVE, // champion_event_args_t
	EVENT_PAWN_RUNAWAY, // pawn_event_args_t
} event_type_t;

typedef struct event_t {
    event_type_t type;
    struct server_t* server;
    pawn_t* pawn;
    void* args;
} event_t;

#define MAKE_EVENT(t, a) (event_t) { .type = t, .server = NULL, .pawn = NULL, .args = a }

void initPawn(pawn_t* pawn);
void dropPawn(pawn_t* pawn);

void addItemToPawn(pawn_t* pawn, struct item_t* item);
void removeItemFromPawn(pawn_t* pawn, struct item_t* item);

void addChampionToPawn(pawn_t* pawn, struct champion_t* champion);
void removeChampionFromPawn(pawn_t* pawn, struct champion_t* champion);

void removeChampionFromTeam(pawn_t* pawn, struct champion_t* champion);

void addItemToPawn(pawn_t* pawn, struct item_t* item);
void removeItemFromPawn(pawn_t* pawn, struct item_t* item);

void notifyChampionAdded(struct server_t* server, pawn_t* pawn, struct champion_t* champion, char* reason);
void notifyChampionRemoved(struct server_t* server, pawn_t* pawn, struct champion_t* champion, char* reason);

void notifyItemAdded(struct server_t* server, pawn_t* pawn, struct item_t* item, char* reason);
void notifyItemRemoved(struct server_t* server, pawn_t* pawn, struct item_t* item, char* reason);

/** donne la possibilité à l'eventHandler de faire une update. principalement utilisé pour l'IA */
void notifyPawnUpdate(struct server_t* server, pawn_t* pawn);
void notifySetContext(struct server_t* server, pawn_t* pawn, context_t context, function_t newHandler);
void notifyPopContext(struct server_t* server, pawn_t* pawn);

/** opérations routinières */
void updatePawn(struct server_t* server, pawn_t* pawn);

/**
 * 
 *  Envoie un évènement à l'entité
 *  il suffit de renseigner event.type et event.args
 * 
 */
void sendPawnEvent(struct server_t* server, pawn_t* pawn, event_t* event);

void changePawnGold(struct server_t* server, pawn_t* pawn, int delta, char* reason);

// ---------------------------------------------------------------------------------------------------------------------------------------

typedef struct champion_ability_event_args_t {
	struct champion_t* champion;
	struct ability_t* ability;
} champion_ability_event_args_t;

typedef struct update_event_args_t { } update_event_args_t;

typedef struct setcontext_event_args_t {
	context_t context;
	function_t handler;
} setcontext_event_args_t;

typedef struct popcontext_event_args_t { } popcontext_event_args_t;

typedef struct pawn_event_args_t {
	pawn_t* pawn;
} pawn_event_args_t;

typedef struct message_event_args_t {
	char* message;
} message_event_args_t;

typedef struct interaction_event_args_t {
	struct interaction_t* interaction;
} interaction_event_args_t;

typedef struct item_use_event_args_t {
	struct use_t* use;
} item_use_event_args_t;

typedef struct gold_changed_event_args_t {
	char* delta;
	char* reason;
} gold_changed_event_args_t;

typedef struct champion_event_args_t {
	struct champion_t* champion;
	char* reason;
} champion_event_args_t;

typedef struct item_event_args_t {
	struct item_t* item;
	char* reason;
} item_event_args_t;

#endif
