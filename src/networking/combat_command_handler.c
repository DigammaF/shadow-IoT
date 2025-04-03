
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "combat_command_handler.h"
#include "server.h"
#include "user.h"
#include "fight.h"
#include "pawn.h"
#include "account.h"
#include "ability.h"
#include "champion.h"
#include "string_utils.h"

void handleCombatAttack(command_context_t* context) {
	unsigned sourceChampionKey;
	unsigned destinationChampionKey;
	unsigned abilityKey;

	if (!safeStrToUnsigned(context->args[2], &sourceChampionKey)) { fprintf(stderr, "(!) Failed to convert '%s' to an unsigned\n", context->args[2]); return; }
	if (!safeStrToUnsigned(context->args[3], &destinationChampionKey)) { fprintf(stderr, "(!) Failed to convert '%s' to an unsigned\n", context->args[3]); return; }
	if (!safeStrToUnsigned(context->args[4], &abilityKey)) { fprintf(stderr, "(!) Failed to convert '%s' to an unsigned\n", context->args[4]); return; }

	user_t* user = context->user;
	account_t* account = user->account;
	pawn_t* pawn = account->pawn;
	fight_t* fight = pawn->fight;

	champion_t* sourceChampion = hashmapGet(&fight->champions, sourceChampionKey);
	champion_t* destinationChampion = hashmapGet(&fight->champions, destinationChampionKey);

	if (sourceChampion == NULL) { fprintf(stderr, "(!) could not fetch champion %i\n", sourceChampionKey); return; }
	if (destinationChampion == NULL) { fprintf(stderr, "(!) could not fetch champion %i\n", destinationChampionKey); return; }

	if (!pawnCanOrderChampion(pawn, sourceChampion)) {
		fprintf(stderr, "(!) can't order champion\n"); return;
	}

	if (isDead(sourceChampion)) {
		fprintf(stderr, "(!) champion is dead\n"); return;
	}

	if (!sourceChampion->hasInitiative) {
		fprintf(stderr, "(!) champion doesn't have initiative\n"); return;
	}

	if (sourceChampion->fleeing) {
		fprintf(stderr, "(!) champion is fleeing\n"); return;
	}

	ability_t* ability = hashmapGet(&sourceChampion->abilities, abilityKey);
	applyAbility(context->server, sourceChampion, destinationChampion, ability);
}

void handleRunaway(command_context_t* context) {
	user_t* user = context->user;
	account_t* account = user->account;
	pawn_t* pawn = account->pawn;
	fight_t* fight = pawn->fight;

	applyPawnRunaway(context->server, pawn);
	removePawnFromFight(fight, pawn);
	popUserContext(user);
}

void handleCombatSee(command_context_t* context) {
	user_t* user = context->user;
	account_t* account = user->account;
	pawn_t* pawn = account->pawn;
	fight_t* fight = pawn->fight;
	char message[COMMUNICATION_SIZE];

	for (unsigned n = 0; n < fight->champions.capacity; n++) {
		champion_t* champion = fight->champions.elements[n];
		if (champion == NULL) { continue; }
		formatChampion(champion, "LIST-CHAMPION", message, COMMUNICATION_SIZE);
		sendData(&user->socket, message);
	}

	sendData(&user->socket, "END-LIST");
}

void handleFlee(command_context_t* context) {
	unsigned championKey;

	if (!safeStrToUnsigned(context->args[2], &championKey)) { fprintf(stderr, "(!) Failed to convert '%s' to an unsigned\n", context->args[2]); return; }

	user_t* user = context->user;
	account_t* account = user->account;
	pawn_t* pawn = account->pawn;
	fight_t* fight = pawn->fight;

	champion_t* champion = hashmapGet(&fight->champions, championKey);

	if (champion == NULL) { fprintf(stderr, "(!) could not fetch champion %i\n", championKey); return; }

	if (!pawnCanOrderChampion(pawn, champion)) {
		fprintf(stderr, "(!) can't order champion\n"); return;
	}

	if (isDead(champion)) {
		fprintf(stderr, "(!) champion is dead\n"); return;
	}

	if (!champion->hasInitiative) {
		fprintf(stderr, "(!) champion doesn't have initiative\n"); return;
	}

	if (champion->fleeing) {
		fprintf(stderr, "(!) champion is fleeing\n"); return;
	}

	champion->fleeing = true;
	char reason[200];
	sprintf(reason, "ordre de %s", pawn->name);
	champion_event_args_t args = { .champion = champion, .reason = reason };
	event_t event = MAKE_EVENT(EVENT_CHAMPION_FLEE, &args);
	notifyFight(context->server, fight, &event);
}

void* combatCommandHandler(void* arg) {
	command_context_t* context = (command_context_t*)arg;

	if (context->count == 2) {
		if (strcmp(context->args[1], "RUNAWAY") == 0) {
			handleRunaway(context);
			return NULL;
		}

		if (strcmp(context->args[1], "SEE") == 0) {
			handleCombatSee(context);
			return NULL;
		}
	}

	if (context->count == 3) {
		if (strcmp(context->args[1], "FLEE") == 0) {
			handleFlee(context);
			return NULL;
		}
	}

	if (context->count == 5) {
		if (strcmp(context->args[1], "ATTACK") == 0) {
			handleCombatAttack(context);
			return NULL;
		}
	}

	printf("(!) unable to parse command\n");
	return NULL;
}
