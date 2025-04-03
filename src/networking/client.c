
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <string.h>
#include <ncurses.h>
#include <stdbool.h>
#include <wiringPi.h>

#include "client.h"
#include "lantern.h"
#include "constants.h"
#include "function_stack.h"
#include "vector.h"
#include "string_utils.h"

#define UNUSED(x) (void)(x)
#define CHECKM(status, message) { if ((status) == NULL) { perror(message); exit(EXIT_FAILURE); } }

void initClient(client_t* client) {
	client->gui = NULL;
	initFunctionStack(&client->contextHandlers);
}

void dropClient(client_t* client) {
	dropFunctionStack(&client->contextHandlers);
}

void initWidget(widget_t* widget) {
	widget->update = noHandler;
	widget->render = noHandler;
	widget->properties = NULL;
}

void dropWidget(widget_t* widget) { UNUSED(widget); }

void initGUI(gui_t* gui) {
	initHashmap(&gui->widgets);
}

void dropGUI(gui_t* gui) {
	// TODO free
	dropHashmap(&gui->widgets);
}

void resetKeyboard(keyboard_t* keyboard) {
	keyboard->enter = false;
	keyboard->tab = false;
	keyboard->character = '\0';
}

void setupClientFileDescriptorSet(client_t* client, fd_set* fileDescriptorSet, int* maxFileDescriptor) {
	socket_t* clientSocket = &client->socket;
	FD_ZERO(fileDescriptorSet);
	
	//écoute du serveur via le socket client
	FD_SET(clientSocket->fileDescriptor, fileDescriptorSet);
	*maxFileDescriptor = clientSocket->fileDescriptor;
	
	//écoute du clavier local
	int stdinFileDescriptor = fileno(stdin);
	FD_SET(stdinFileDescriptor, fileDescriptorSet);
	if (stdinFileDescriptor > *maxFileDescriptor) { *maxFileDescriptor = stdinFileDescriptor; }
	
	return;
}

void handleClientSockets(client_t* client, fd_set* fileDescriptorSet){
	socket_t* clientSocket = &client->socket;

	if (FD_ISSET(clientSocket->fileDescriptor, fileDescriptorSet)) {
		char line[COMMUNICATION_SIZE];
		int byteCount = recvData(clientSocket, line, COMMUNICATION_SIZE);
		if (byteCount == 0) {
			printf("(-) le serveur a mis fin à la connexion\n");
			client->running = 0;
			return;
		}
		printf("(received) '%s'\n", line);

		// for (unsigned n = 0; n < 1024; n++) {
		// 	printf("%c", line[n]);
		// }
		// printf("\n");
	}

	if (FD_ISSET(fileno(stdin), fileDescriptorSet)) {
		char line[512];
		fgets(line, sizeof(line), stdin);
		line[strlen(line) - 1] = '\0';
		printf("(sending) '%s'\n", line);
		char sent[1024];
		sprintf(sent, "COMMAND %s", line);
		sendData(clientSocket, sent);
	}
	
	return;
}

void awaitInput(client_t* client) {
	int maxFileDescriptor;
	struct timeval timeout = { 0, SERVER_TICK };
	setupClientFileDescriptorSet(client, &client->ioState, &maxFileDescriptor);
	select(maxFileDescriptor + 1, &client->ioState, NULL, NULL, &timeout);
}

int mainClient(int argc, const char* argv[]) {
	UNUSED(argc); UNUSED(argv);

	initscr(); clear(); noecho(); cbreak(); keypad(stdscr, TRUE);
	
	wiringPiSetupGpio();
	pinMode(UP_BUTTON, INPUT);
	pinMode(DOWN_BUTTON, INPUT);
	pinMode(VALIDATE_BUTTON, INPUT);
	pullUpDnControl(UP_BUTTON, PUD_UP);
	pullUpDnControl(DOWN_BUTTON, PUD_UP);
	pullUpDnControl(VALIDATE_BUTTON, PUD_UP);

	socket_t clientSocket;
	connectServer(&clientSocket, argv[2], SERVER_PORT);
	client_t client;
	initClient(&client);
	client.socket = clientSocket;
	client.running = 1;
	client.loggedIn = 0;

	phaseLogin(&client);
	
	while (client.running) {
		awaitInput(&client);
		handleClientSockets(&client, &client.ioState);
	}

	closeSocket(&clientSocket);
	dropClient(&client);
	return 0;
}

void phaseLogin(client_t* client) {
	unsigned outputY = 2;

	char login[MAX_INPUT] = { 0 };
	char password[MAX_INPUT] = { 0 };
	unsigned loginHead = 0;
	unsigned passwordHead = 0;
	unsigned editionMode = 1;

    mvprintw(1, 2, ">Nom d'utilisateur: ");
	mvprintw(2, 2, " Mot de passe: ");
	// unsigned passwordTextWidth = 16;
	refresh();
	
	while (!client->loggedIn) {
		awaitInput(client);

		if (FD_ISSET(client->socket.fileDescriptor, &client->ioState)) {
			handleServerResponse(client, &outputY);
		}

		if (
			FD_ISSET(fileno(stdin), &client->ioState)
			|| (digitalRead(UP_BUTTON) == LOW)
			|| (digitalRead(DOWN_BUTTON) == LOW)
			|| (digitalRead(VALIDATE_BUTTON) == LOW)
		) {
			handleUserInput(client, &outputY, login, &loginHead, password, &passwordHead, &editionMode);
		}
	}

	endwin();
}

// ------------------------------------------------------------------------------------------------------

void addWidgetToGUI(gui_t* gui, widget_t* widget) {
	widget->key = hashmapLocateUnusedKey(&gui->widgets);
	hashmapSet(&gui->widgets, widget->key, widget);
}

void removeWidgetFromGUI(gui_t* gui, widget_t* widget) {
	hashmapSet(&gui->widgets, widget->key, NULL);
}

// ------------------------------------------------------------------------------------------------------

void* updateMenu(void* _) {
	widget_context_t* context = (widget_context_t*)_;
	widget_t* widget = context->widget;
	client_t* client = context->client;
	gui_t* gui = client->gui;
	keyboard_t* keyboard = &gui->keyboard;
	menu_properties_t* properties = widget->properties;

	if (properties->validated) { return NULL; }

	if (keyboard->key_up) {
		if (properties->select > 0) { properties->select--; }
	}

	if (keyboard->key_down) {
		if (properties->select < properties->optionCount - 1) { properties->select++; }
	}

	if (keyboard->enter) {
		properties->validated = true;
	}

	return NULL;
}

void* renderMenu(void* _) {
	widget_context_t* context = (widget_context_t*)_;
	widget_t* widget = context->widget;
	menu_properties_t* properties = widget->properties;

	int x = 2, y = 1;
    box(properties->window, 0, 0);

    for (unsigned n = 0; n < properties->optionCount; n++) {
        if (n == properties->select) { wattron(properties->window, A_REVERSE); }
        mvwprintw(properties->window, y + n, x, "%s", properties->options[n]);
		fprintf(stderr, "drawing\n");
        wattroff(properties->window, A_REVERSE);
    }

	return NULL;
}

void makeMenu(
	widget_t* widget,
	unsigned startx, unsigned starty, unsigned width,
	char** options, unsigned optionCount, unsigned initialSelect
) {
	widget->update = updateMenu;
	widget->render = renderMenu;
	menu_properties_t* properties = malloc(sizeof(menu_properties_t));
	properties->window = newwin(optionCount + 2, width, starty, startx);
	keypad(properties->window, TRUE);
	properties->select = initialSelect;
	properties->options = options;
	properties->optionCount = optionCount;
	properties->validated = false;
	widget->properties = properties;
}

// -------------------------------------------------------------------------------------------------------

void setupInitial(client_t* client) {
	gui_t* gui = client->gui;
	char** options = malloc(2*sizeof(char*));
	CHECKM(options, "malloc options");
	options[0] = "Identification";
	options[1] = "Enregistrement";
	widget_t* menu = malloc(sizeof(widget_t));
	initWidget(menu);
	makeMenu(
		menu,
		(COLS - 20) / 2, (LINES - 2) / 2, 20,
		options, 2, 0
	);
	addWidgetToGUI(gui, menu);
}

// -------------------------------------------------------------------------------------------------------

void* initialClientHandler(void* _) {
	UNUSED(_);
	return NULL;
}

void handleServerResponse(client_t* client, unsigned* outputY) {
	char data[1024];
	unsigned argCount;
	int byteCount = recvData(&client->socket, data, 1024);
	char** args = splitString(data, &argCount);

	if (byteCount == 0) {
		client->running = 0;
		return;
	}

	if (argCount == 2 && strcmp(args[0], "SET-CONTEXT") == 0) {
		context_t context = (context_t)atoi(args[1]);
		if (context == CONTEXT_GAMEWORLD) {
			client->loggedIn = 1;
			clear(); refresh();
			return;
		}
	}

	if (argCount > 2) {
		clear();
		char* text = joinString(&args[1], " ");
		if (strcmp(args[0], "OUTPUT") == 0) {
			mvprintw(1, *outputY, "serveur: %s", text);
		} else if (strcmp(args[0], "ERROR") == 0) {
			mvprintw(1, *outputY, "erreur: %s", text);
		}
		(*outputY)++;
		freeJoin(text);
		refresh();
	}
}

void handleUserInput(client_t* client, unsigned* outputY, char* login, unsigned* loginHead, char* password, unsigned* passwordHead, unsigned* editionMode) {
	int character = getch();

	if (character == '\n') {
		clear();
		login[*loginHead] = '\0';
		password[*passwordHead] = '\0';
		mvprintw(1, *outputY, "Identification avec %s / %s ...", login, password);
		(*outputY)++;
		refresh();
		char message[COMMUNICATION_SIZE];
		sprintf(message, "COMMAND LOGIN %s %s", login, password);
		sendData(&client->socket, message);
	} else if (character == 127) {
		if ((*editionMode == 1 && *loginHead > 0) || (*editionMode == 2 && *passwordHead > 0)) {
			unsigned x = *editionMode == 1 ? 21 + *loginHead : 16 + *passwordHead;
			mvprintw(*editionMode, x, " ");
			move(*editionMode, x);
			if (*editionMode == 1) { (*loginHead)--; }
			if (*editionMode == 2) { (*passwordHead)--; }
		}
	} else if (
		((*editionMode == 1 && *loginHead < MAX_INPUT) || (*editionMode == 2 && *passwordHead < MAX_INPUT))
		&& character >= 32 && character <= 126
	) {
		unsigned x;
		if (*editionMode == 1) {
			login[*loginHead] = character;
			(*loginHead)++;
			x = 21 + *loginHead;
		} else {
			password[*passwordHead] = character;
			(*passwordHead)++;
			x = 16 + *passwordHead;
		}
		mvprintw(*editionMode, x, "%c", character);
		refresh();
	} else if (character == 9 || digitalRead(UP_BUTTON) || digitalRead(DOWN_BUTTON)) {
		mvprintw(*editionMode, 2, " ");
		*editionMode = (*editionMode == 1) ? 2 : 1;
		mvprintw(*editionMode, 2, ">");
		refresh();
	}
}

void* registerClientHandler(void* _) { UNUSED(_); return NULL; }

void* gameworldClientHandler(void* _) { UNUSED(_); return NULL; }

void* moveClientHandler(void* _) { UNUSED(_); return NULL; }

void* interactClientHandler(void* _) { UNUSED(_); return NULL; }

void* buyItemClientHandler(void* _) { UNUSED(_); return NULL; }

void* buyChampionClienthandler(void* _) { UNUSED(_); return NULL; }

void* sellItemClienthandler(void* _) { UNUSED(_); return NULL; }

void* sellChampionClientHandler(void* _) { UNUSED(_); return NULL; }

// -------------------------------------------------------------------------------------------------------

int mainGUIClient(int argc, const char* argv[]) {
	UNUSED(argc); UNUSED(argv);
	setbuf(stderr, NULL);
	socket_t clientSocket;
	connectServer(&clientSocket, "127.0.0.1", SERVER_PORT);
	client_t client;
	initClient(&client);
	client.socket = clientSocket;
	client.running = 1;
	gui_t* gui = malloc(sizeof(gui_t));
	CHECKM(gui, "malloc gui");
	initGUI(gui);
	client.gui = gui;
	pushFunction(&client.contextHandlers, initialClientHandler);
	setupInitial(&client);
	// ncurses
	initscr(); clear(); noecho(); cbreak(); keypad(stdscr, TRUE);

	while (client.running) {
		awaitInput(&client);

		// read server
		while (FD_ISSET(client.socket.fileDescriptor, &client.ioState)) {
			server_request_context_t context;
			context.client = &client;
			char data[1024];
			int byteCount = recvData(&client.socket, data, 1024);
			context.args = splitString(data, &context.argCount);

			if (byteCount == 0) { client.running = 0; return 0; }

			function_t handler = peekFunction(&client.contextHandlers);
			handler((void*)&context);

			if (functionStackEmpty(&client.contextHandlers)) { client.running = 0; }
		}
		
		// read keyboard
		while (FD_ISSET(fileno(stdin), &client.ioState)) {
			keyboard_t* keyboard = &gui->keyboard;
			resetKeyboard(keyboard);
			int character = getch();

			if (character == '\n') { keyboard->enter = true; }
			if (character == 9) { keyboard->tab = true; }
			if (character == KEY_UP) { keyboard->key_up = true; }
			if (character == KEY_DOWN) { keyboard->key_down = true; }
			if (character >= 32 && character <= 126) { keyboard->character = (char)character; }

			for (unsigned n = 0; n < gui->widgets.capacity; n++) {
				widget_t* widget = gui->widgets.elements[n];
				if (widget == NULL) { continue; }
				widget_context_t context = { .widget = widget, .client = &client };
				widget->update((void*)&context);
			}
		}

		// rendering
		clear();

		for (unsigned n = 0; n < gui->widgets.capacity; n++) {
			widget_t* widget = gui->widgets.elements[n];
			if (widget == NULL) { continue; }
			widget_context_t context = { .widget = widget, .client = &client };
			widget->render((void*)&context);
		}

		fprintf(stderr, "refreshing\n");
		refresh();
	}

	endwin();
	closeSocket(&clientSocket);
	dropClient(&client);
	return 0;
}
