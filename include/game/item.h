
#ifndef ITEM_H
#define ITEM_H

#include "function_stack.h"

struct pawn_t;
struct server_t;
struct champion_t;

/** un objet que possède un pion */
typedef struct item_t {
	char* name;
	struct pawn_t* pawn; // peut être NULL
	unsigned pawnKey;
	void* params; // possède la valeur
	function_t useHandler; // appelé avec (use_t*), indique l'effet de l'item à l'initialisation
} item_t;

/** types d'utilisation d'un item, avec leurs types d'arguments en commentaire */
typedef enum {
	USE_ON_CHAMPION, // champion_use_args_t
	USE_SOLO, // solo_use_args_t
} use_type_t;

/** évènement d'utilisation d'un item */
typedef struct use_t {
    use_type_t type;
    struct server_t* server;
    item_t* item;
    void* args;
} use_t;

typedef struct no_item_params_t { } no_item_params_t;

void initItem(item_t* item);
void dropItem(item_t* item);

#define MAKE_USE(t, a) (use_t) { .type = t, .server = NULL, .item = NULL, .args = a }

void makeSqueaker(item_t* item);

void triggerItemUse(struct server_t* server, item_t* item, use_t* use);

typedef struct solo_use_args_t { } solo_use_args_t;

typedef struct champion_use_args_t {
	struct champion_t* champion;
} champion_use_args_t;

void* squeakerUseHandler(void* _);

#endif
