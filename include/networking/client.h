
#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>

#include "hashmap.h"
#include "lantern.h"
#include "function_stack.h"
#include "vector.h"
#include "ncurses.h"

#define MAX_INPUT 300

#define UP_BUTTON 0
#define DOWN_BUTTON 0
#define VALIDATE_BUTTON 0

typedef struct {
	bool enter; bool tab; bool key_up; bool key_down;
	char character;
} keyboard_t;

typedef struct {
	function_t update; // (widget_context_t*)
	function_t render; // (widget_context_t*)
	void* properties; // possède les valeurs
	unsigned key;
} widget_t;

typedef struct {
	hashmap_t widgets; // possède les valeurs, widget_t*
	keyboard_t keyboard;
} gui_t;

typedef struct {
	unsigned running;
	socket_t socket;
	function_stack_t contextHandlers;
	fd_set ioState;
	gui_t* gui;
	int loggedIn;
} client_t;

typedef struct {
	client_t* client;
	widget_t* widget;
} widget_context_t;

void initClient(client_t* client);
void dropClient(client_t* client);

void initWidget(widget_t* widget);
void dropWidget(widget_t* widget);

void initGUI(gui_t* gui);
void dropGUI(gui_t* gui);

int mainClient(int argc, const char* argv[]);
int mainGUIClient(int argc, const char* argv[]);

void addWidgetToGUI(gui_t* gui, widget_t* widget);
void removeWidgetFromGUI(gui_t* gui, widget_t* widget);

typedef struct {
	client_t* client;
	char** args;
	unsigned argCount;
} server_request_context_t;

void handleUserInput(client_t* client, WINDOW* window, WINDOW* output, unsigned* outputY, char* login, unsigned* loginHead, char* password, unsigned* passwordHead, unsigned* editionMode);
void handleServerResponse(client_t* client, WINDOW* output, unsigned* outputY);
void phaseLogin(client_t* client);

void* initialClientHandler(void* _);
void* registerClientHandler(void* _);
void* gameworldClientHandler(void* _);
void* moveClientHandler(void* _);
void* interactClientHandler(void* _);
void* buyItemClientHandler(void* _);
void* buyChampionClienthandler(void* _);
void* sellItemClienthandler(void* _);
void* sellChampionClientHandler(void* _);

typedef struct {
	WINDOW* window;
	unsigned select;
	char** options;
	unsigned optionCount;
	bool validated;
} menu_properties_t;

#endif
