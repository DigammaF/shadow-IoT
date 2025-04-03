
#include <stdlib.h>

#include <stdbool.h>

#include "ability.h"

#define UNUSED(x) (void)(x)

void initAbility(ability_t* ability) {
	ability->champion = NULL;

	for (unsigned n = 0; n < MAX_ABILITY_OP_CODES; n++) {
		ability->opCodes[n] = NOOP;
	}
}

void dropAbility(ability_t* ability) { UNUSED(ability); }

void makeVenom(ability_t* ability) {
	ability->name = "Venin";
	ability->type = 10;
	ability->monoTarget = true;
	ability->opCodes[0] = SET_TARGET;
	ability->opCodes[1] = 1;
	ability->opCodes[2] = SET_HEALTH;
	ability->opCodes[3] = -3;
}
