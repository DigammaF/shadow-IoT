
#include <string.h>
#include <stdio.h>

#include "initial_command_handler.h"
#include "gameworld_command_handler.h"
#include "event_handlers.h"
#include "command_handlers.h"
#include "user.h"
#include "account.h"
#include "server.h"
#include "constants.h"
#include "pawn.h"
#include "champion.h"
#include "place.h"

#define CHECKM(status, message) { if ((status) == NULL) { perror(message); exit(EXIT_FAILURE); } }
#define UNUSED(x) (void)(x)

int checkArgCount(user_t* user, int count, int expected) {
	if (count == expected) { return 1; }
	printf("(!) [%i] user sent %i args (expected: %i)", user->id, count, expected);
	return 0;
}

void handleLogin(server_t* server, user_t* user, command_context_t* context) {
	account_t* account = locateAccount(server, context->args[2]);

	if (account == NULL) {
		printf("\t[cannot find]\n");
		sendData(&user->socket, "ERROR ce compte n'existe pas");
		return;
	}

	if (account->flags & BANNED_FLAG) {
		printf("\t[banned]\n");
		sendData(&user->socket, "ERROR ce compte est banni");
		return;
	}

	if (!checkPassword(account, context->args[3])) {
		printf("\t[wrong password]\n");
		sendData(&user->socket, "ERROR mauvais mot de passe");
		return;
	}

	if (account->flags & ADMIN_FLAG) {
		printf("\t[admin]\n");
		popUserContext(user);
		setUserContext(user, CONTEXT_ADMIN, adminCommandhandler);
		return;
	}

	printf("\t[standard]\n");
	user->account = account;
	popUserContext(user);
	setUserContext(user, CONTEXT_GAMEWORLD, gameworldCommandHandler);
}

pawn_t* setupPawn(server_t* server, account_t* account, user_t* user) {
	pawn_t* pawn = malloc(sizeof(pawn_t));
	CHECKM(pawn, "malloc pawn");
	initPawn(pawn);
	pawn->account = account;
	account->pawn = pawn;
	pawn->name = strdup(account->name);
	pawn->user = user;
	pawn->gold = 1000;
	pawn->eventHandler = playerEventHandler;
	place_t* spawnPoint = server->world.places[1][1];
	spawnPawn(server, spawnPoint, pawn);
	return pawn;
}

void setupItems(pawn_t* pawn) {
	item_t* item = malloc(sizeof(item_t));
	CHECKM(item, "malloc item");
	initItem(item);
	makeSqueaker(item);
	addItemToPawn(pawn, item);
}

void setupChampions(pawn_t* pawn) {
	champion_t* spider = malloc(sizeof(champion_t));
	CHECKM(spider, "malloc spider");
	initChampion(spider);
	makeSpider(spider);
	replenishStats(spider);
	addChampionToPawn(pawn, spider);
	pawn->team[0] = spider;

	ability_t* venom = malloc(sizeof(ability_t));
	CHECKM(venom, "malloc ability");
	initAbility(venom);
	makeVenom(venom);
	addAbilityToChampion(spider, venom);
}

void handleRegister(server_t* server, user_t* user, command_context_t* context) {
	account_t* account = locateAccount(server, context->args[2]);

	if (account != NULL) {
		printf("\t[already exists]\n");
		sendData(&user->socket, "ERROR ce compte existe déjà");
		return;
	}

	if (strlen(context->args[2]) > MAX_LOGIN_SIZE) {
		printf("\t[login too big]\n");
		sendData(&user->socket, "ERROR nom d'utilisateur trop long");
		return;
	}

	if (strlen(context->args[3]) > MAX_PASSWORD_SIZE) {
		printf("\t[password too big]\n");
		sendData(&user->socket, "ERROR mot de passe trop long");
		return;
	}

	account = malloc(sizeof(account_t));
	CHECKM(account, "malloc account");
	initAccount(account);
	printf("\t[creation of '%s' with password '%s']\n", context->args[2], context->args[3]);
	createAccount(server, account, context->args[2], context->args[3], 0);
	user->account = account;
	printf("\t[spawning pawn]\n");
	pawn_t* pawn = setupPawn(server, account, user);
	printf("\t[giving items]\n");
	setupItems(pawn);
	printf("\t[giving champions]\n");
	setupChampions(pawn);
	printf("\t[success]\n");
	popUserContext(user);
	setUserContext(user, CONTEXT_GAMEWORLD, gameworldCommandHandler);
	return;
}

void* initialCommandHandler(void* arg) {
	command_context_t* context = (command_context_t*)arg;
	user_t* user = context->user;
	server_t* server = context->server;

	if (!checkArgCount(user, context->count, 4)) { return NULL; }

	if (strcmp(context->args[1], "LOGIN") == 0) {
		printf("(+) fetching account\n");
		handleLogin(server, user, context);
		return NULL;
	}

	if (strcmp(context->args[1], "REGISTER") == 0) {
		printf("(+) creating account\n");
		handleRegister(server, user, context);
		return NULL;
	}

	printf("(!) invalid argument '%s' (expected: LOGIN|REGISTER)\n", context->args[1]);
	return NULL;
}
