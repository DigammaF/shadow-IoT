
#ifndef ABILITY_H
#define ABILITY_H

#include <stdbool.h>

struct champion_t;

#define MAX_ABILITY_OP_CODES 10

/**
 * 
 * Z: entiers relatifs
 * N: entiers positifs
 * 
 * SET-TARGET 0|1 choisit la cible entre source et destination
 * SET-HEALTH Z change les points de vie de la cible
 * ADD-EFFECT N ajoute un effet à la cible
 * REM-EFFECT N enlève l'effet à la cible
 * IF-EFFECT N si la cible n'a pas cet effet, arrêter
 * IF-TYPE N si la cible n'est pas de ce type, arrêter
 * NOOP pas d'action
 * 
*/

#define SET_TARGET 0
#define SET_HEALTH 1
#define ADD_EFFECT 2
#define REM_EFFECT 3
#define IF_EFFECT 4
#define IF_TYPE 5
#define NOOP 6

/**
 * une capacité que possède un champion
 */
typedef struct ability_t {
	char* name;
	struct champion_t* champion; // peut être NULL
	unsigned championKey;
    unsigned type;
    int opCodes[MAX_ABILITY_OP_CODES];
    bool monoTarget; // indique si la compétence est lancée sur toute l'équipe adverse
} ability_t;

void initAbility(ability_t* ability);
void dropAbility(ability_t* ability);

void makeVenom(ability_t* ability);

#endif
