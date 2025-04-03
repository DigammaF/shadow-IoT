
#include <stdlib.h>
#include <stdio.h>

#include "lantern.h"
#include "event_handlers.h"
#include "pawn.h"
#include "user.h"
#include "account.h"
#include "champion.h"

#define UNUSED(x) (void)(x)

/**
 * 
 * 	Gestionnaire d'évènements associé à un pawn controlé par un joueur
 *  En particulier, ce pawn est lié à un account
 * 
 */
void* playerEventHandler(void* _) {
	event_t* event = (event_t*)_;
	pawn_t* pawn = event->pawn;
	user_t* user = pawn->user;

	if (user == NULL) { return NULL; }

	char message[COMMUNICATION_SIZE];
	pawn_event_args_t* pawnEventArgs = NULL;
	message_event_args_t* messageEventArgs = NULL;
	gold_changed_event_args_t* goldChangedEventArgs = NULL;
	champion_event_args_t* championEventArgs = NULL;
	item_event_args_t* itemEventArgs = NULL;
	setcontext_event_args_t* setContextEventArgs = NULL;
	champion_ability_event_args_t* championAbilityArgs = NULL;

	switch (event->type) {
		case EVENT_PAWN_ARRIVED:
			pawnEventArgs = event->args;
			sprintf(message, "PAWN-ARRIVED %i %s", pawnEventArgs->pawn->placeKey, pawnEventArgs->pawn->name);
			sendData(&user->socket, message);
			break;

		case EVENT_PAWN_LEFT:
			pawnEventArgs = event->args;
			sprintf(message, "PAWN-LEFT %s", pawnEventArgs->pawn->name);
			sendData(&user->socket, message);
			break;

		case EVENT_PAWN_SPAWNED:
			pawnEventArgs = event->args;
			sprintf(message, "PAWN-SPAWN %i %s", pawnEventArgs->pawn->placeKey, pawnEventArgs->pawn->name);
			sendData(&user->socket, message);
			break;

		case EVENT_MESSAGE:
			messageEventArgs = event->args;
			sprintf(message, "MESSAGE %s", messageEventArgs->message);
			sendData(&user->socket, message);
			break;

		case EVENT_CHAMPION_GAINED:
			championEventArgs = event->args;
			sprintf(message, "GAINED-CHAMPION %i %s %s", championEventArgs->champion->pawnKey, championEventArgs->champion->name, championEventArgs->reason);
			sendData(&user->socket, message);
			break;

		case EVENT_CHAMPION_LOST:
			championEventArgs = event->args;
			sprintf(message, "LOST-CHAMPION %s %s", championEventArgs->champion->name, championEventArgs->reason);
			sendData(&user->socket, message);
			break;

		case EVENT_GOLD_CHANGED:
			goldChangedEventArgs = event->args;
			sprintf(message, "YOUR-GOLD-CHANGED %s %s", goldChangedEventArgs->delta, goldChangedEventArgs->reason);
			sendData(&user->socket, message);
			break;

		case EVENT_ITEM_GAINED:
			itemEventArgs = event->args;
			sprintf(message, "GAINED-ITEM %i %s %s", itemEventArgs->item->pawnKey, itemEventArgs->item->name, itemEventArgs->reason);
			sendData(&user->socket, message);
			break;

		case EVENT_ITEM_LOST:
			itemEventArgs = event->args;
			sprintf(message, "LOST-ITEM %i %s %s", itemEventArgs->item->pawnKey, itemEventArgs->item->name, itemEventArgs->reason);
			sendData(&user->socket, message);
			break;

		case EVENT_SETCONTEXT:
			setContextEventArgs = event->args;
			setUserContext(user, setContextEventArgs->context, setContextEventArgs->handler);
			break;

		case EVENT_POPCONTEXT:
			popUserContext(user);
			break;

		case EVENT_CHAMPION_ABILITY_USE:
			championAbilityArgs = event->args;
			sprintf(
				message,
				"CHAMPION-USE-ABILITY %i %s %i %s",
				championAbilityArgs->champion->fightKey, championAbilityArgs->champion->name,
				championAbilityArgs->ability->championKey, championAbilityArgs->ability->name
			);
			sendData(&user->socket, message);
			break;

		case EVENT_CHAMPION_FLEE:
			championEventArgs = event->args;
			sprintf(
				message,
				"CHAMPION-FLEE %i %s %s",
				championEventArgs->champion->fightKey, championEventArgs->champion->name,
				championEventArgs->reason
			);
			sendData(&user->socket, message);
			break;

		case EVENT_CHAMPION_GET_INITIATIVE:
			championEventArgs = event->args;
			sprintf(
				message,
				"CHAMPION-GET-INITIATIVE %i %s %s",
				championEventArgs->champion->fightKey, championEventArgs->champion->name,
				championEventArgs->reason
			);
			sendData(&user->socket, message);
			break;

		default:
			return NULL;
	}

	return NULL;
}
