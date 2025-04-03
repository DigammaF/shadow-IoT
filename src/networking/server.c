
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdint.h>

#include "server.h"
#include "lantern.h"
#include "user.h"
#include "account.h"
#include "constants.h"
#include "initial_command_handler.h"
#include "string_utils.h"
#include "world.h"
#include "pawn.h"

#define CHECK(status, message) { if ((status) == -1) { perror(message); exit(EXIT_FAILURE); } }
#define CHECKM(status, message) { if ((status) == NULL) { perror(message); exit(EXIT_FAILURE); } }
#define UNUSED(x) (void)(x)

void initServer(server_t* server) {
	server->deltaTime = 1;
	server->lastUpdateTime = time(NULL);
	initHashmap(&server->users);
	initHashmap(&server->accounts);
	initWorld(&server->world);
}

void dropServer(server_t* server) {
	for (unsigned n = 0; n < server->users.capacity; n++) {
		user_t* user = server->users.elements[n];
		if (user == NULL) { continue; }
		dropUser(user);
		free(user);
	}

	for (unsigned n = 0; n < server->accounts.capacity; n++) {
		account_t* account = server->accounts.elements[n];
		if (account == NULL) { continue; }
		dropAccount(account);
		free(account);
	}

	dropHashmap(&server->users);
	dropHashmap(&server->accounts);
	dropWorld(&server->world);
}

/**
 * 
 * 	\param socket	Prends possession de la valeur
 * 
 */
void createUser(server_t* server, user_t* user, socket_t* socket) {
	user->running = 1;
	user->socket = *socket;
	user->lastActivity = time(NULL);
	user->id = hashmapLocateUnusedKey(&server->users);
	hashmapSet(&server->users, user->id, user);
	setUserContext(user, CONTEXT_INITIAL, initialCommandHandler);
}

void deleteUser(server_t* server, user_t* user) {
	hashmapSet(&server->users, user->id, NULL);
}

void createAccount(server_t* server, account_t* account, char* name, char* password, unsigned flags) {
	account->name = strdup(name);
	account->password = strdup(password);
	account->flags = flags;
	account->id = hashmapLocateUnusedKey(&server->accounts);
	hashmapSet(&server->accounts, account->id, account);
}

void deleteAccount(server_t* server, account_t* account) {
	hashmapSet(&server->accounts, account->id, NULL);
}

account_t* locateAccount(server_t* server, char* name) {
	for (unsigned n = 0; n < server->accounts.capacity; n++) {
		account_t* account = server->accounts.elements[n];
		if (account == NULL) { continue; }
		if (strcmp(account->name, name) == 0) { return account; }
	}

	return NULL;
}

int checkPassword(account_t* account, char* password) {
	return strcmp(account->password, password) == 0;
}

void handleNewConnection(server_t* server) {
	printf("(+) accepting client\n");
	socket_t socket;
	acceptRemote(&server->socket, &socket);
	user_t* user = malloc(sizeof(user_t));
	CHECKM(user, "malloc user");
	initUser(user);
	createUser(server, user, &socket);
	char IPAddress[512];
	inet_ntop(AF_INET, &(user->socket.remote.sin_addr), IPAddress, sizeof(IPAddress));
	printf("(+) created user %i for remote host %s:%i\n", user->id, IPAddress, user->socket.remote.sin_port);
}

void handleUserCommand(server_t* server, user_t* user, char**args, int argCount) {
	if (functionStackEmpty(&user->commandHandlers)) {
		printf("(!) user has no command handler\n");
		return;
	}

	command_context_t commandContext = { .args = args, .count = argCount, .user = user, .server = server };
	function_t commandHandler = peekFunction(&user->commandHandlers);
	commandHandler((void*)&commandContext);
	user->lastActivity = time(NULL);
	if (functionStackEmpty(&user->commandHandlers)) { user->running = 0; }
}

void handleUserEnd(user_t* user) {
	closeSocket(&user->socket);
	user->running = 0;
}

void handleUserKeepAlive(user_t* user) {
	user->lastActivity = time(NULL);
}

void handleUserRequest(server_t* server, user_t* user) {
	char data[4096];
	int byteCount = recvData(&user->socket, data, 4096);
	unsigned argCount;
	char** args = splitString(data, &argCount);

	if (byteCount == 0) {
		printf("(-) [%i] user disconnected\n", user->id);
		user->running = 0;
		return;
	}

	if (argCount <= 0) {
		printf("(!) [%i] user sent no data\n", user->id);
		return;
	}

	printf("(&) [%i] %s\n", user->id, data);
	unsigned processed = 0;

	if (strcmp(args[0], "COMMAND") == 0) {
		processed = 1;
		handleUserCommand(server, user, args, argCount);
	}

	if (strcmp(args[0], "END") == 0) {
		processed = 1;
		handleUserEnd(user);
	}

	if (strcmp(args[0], "KEEP-ALIVE") == 0) {
		processed = 1;
		handleUserKeepAlive(user);
	}

	if (!processed) {
		printf("invalid argument '%s' (awaiting: COMMAND|END|KEEP-ALIVE)\n", args[0]);
	}

	freeSplit(args, argCount);
}

void update(server_t* server) {
	struct timeval timeValue;
    gettimeofday(&timeValue, NULL);
	uint64_t now = timeValue.tv_sec*1e6 + timeValue.tv_usec;
	server->deltaTime = now - server->lastUpdateTime;
	server->lastUpdateTime = now;

	for (unsigned n = 0; n < server->users.capacity; n++) {
		user_t* user = server->users.elements[n];
		if (user == NULL) { continue; }
		updateUser(server, user);
	}

	updateWorld(server, &server->world);
}

void updateUser(server_t* server, user_t* user) {
	if (!user->running) {
		account_t* account = user->account;
		if (account != NULL) { user->account->pawn->user = NULL; }
		deleteUser(server, user);
		dropUser(user);
	}
}

void setupServer(server_t* server) {
	account_t* account = malloc(sizeof(account_t));
	CHECKM(account, "malloc account");
	initAccount(account);
	createAccount(server, account, "admin", "admin\n", ADMIN_FLAG);
	generateWorld(WORLD_SEED, &server->world);
}

void setupServerFileDescriptorSet(server_t* server, fd_set* fileDescriptorSet, int* maxFileDescriptor) {
	FD_ZERO(fileDescriptorSet);
	FD_SET(server->socket.fileDescriptor, fileDescriptorSet);
	*maxFileDescriptor = server->socket.fileDescriptor;

	for (unsigned n = 0; n < server->users.capacity; n++) {
		user_t* user = server->users.elements[n];
		if (user == NULL) { continue; }
		FD_SET(user->socket.fileDescriptor, fileDescriptorSet);

		if (user->socket.fileDescriptor > *maxFileDescriptor) {
			*maxFileDescriptor = user->socket.fileDescriptor;
		}
	}
}

void handleServerSockets(server_t* server, fd_set* fileDescriptorSet) {
	if (FD_ISSET(server->socket.fileDescriptor, fileDescriptorSet)) {
		handleNewConnection(server);
	}

	for (unsigned n = 0; n < server->users.capacity; n++) {
		user_t* user = server->users.elements[n];
		if (user == NULL) { continue; }

		if (FD_ISSET(user->socket.fileDescriptor, fileDescriptorSet)) {
			handleUserRequest(server, user);
		}
	}
}

int mainServer(int argc, const char** argv) {
	UNUSED(argc); UNUSED(argv);
	setbuf(stderr, NULL);
	setbuf(stdout, NULL);
	server_t server;
	initServer(&server);
	setupServer(&server);
	server.running = 1;
	serverSTREAM(&server.socket, argv[2], SERVER_PORT, 5);
	printf("(+) server ready\n");

	while (server.running) {
		update(&server);
		struct timeval timeout = { .tv_sec = 0, .tv_usec = SERVER_TICK };
		fd_set fileDescriptorSet;
		int maxFileDescriptor;
		setupServerFileDescriptorSet(&server, &fileDescriptorSet, &maxFileDescriptor);
		select(maxFileDescriptor + 1, &fileDescriptorSet, NULL, NULL, &timeout);
		handleServerSockets(&server, &fileDescriptorSet);
	}

	closeSocket(&server.socket);
	dropServer(&server);
	return 0;
}
