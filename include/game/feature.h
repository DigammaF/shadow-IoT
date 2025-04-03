
#ifndef FEATURE_H
#define FEATURE_H

#include <stdint.h>

#include "function_stack.h"

struct server_t;
struct pawn_t;
struct place_t;

/** une feature est n'importe quel objet statique se trouvant dans une région */
typedef struct feature_t {
    char* name;
    function_t interactionHandler; // appelé avec (interaction_t*), indique la réaction d'un pion aux évènements de jeu
	void* properties;
	unsigned placeKey;
	struct place_t* place; // can be NULL
	/**
	 * drapeaux (1 << interaction_type_t), indique à l'extérieur quels types d'évènements sont traités
	 * note: cela ne restreint en rien le type des évènements qui seront envoyés à l'interactionHandler
	*/
	uint16_t interactionFlags;
} feature_t;

void initFeature(feature_t* feature);
void dropFeature(feature_t* feature);

void makeRock(feature_t* feature);
void makeBush(feature_t* feature);

/**
 * types d'interaction ainsi que les structures d'arguments associés indiqués en commentaire
 */
typedef enum {
	INTERACTION_HELP, INTERACTION_HIRE, INTERACTION_MINE, // pawn_interaction_args_t
	INTERACTION_COUNT
} interaction_type_t;

/** évènement d'interaction avec une feature */
typedef struct interaction_t {
    interaction_type_t type;
    struct server_t* server;
    feature_t* feature;
    void* args;
} interaction_t;

#define MAKE_INTERACTION(t, a) (interaction_t) { .type = t, .server = NULL, .feature = NULL, .args = a }

void triggerFeatureInteraction(struct server_t* server, feature_t* feature, interaction_t* interaction);

/** interaction entre un pion et une feature */
typedef struct pawn_interaction_args_t {
	struct pawn_t* pawn;
} pawn_interaction_args_t;

void* debuginteractionHandler(void* _);

#endif
