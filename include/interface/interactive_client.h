
#ifndef INTERACTIVE_CLIENT_H
#define INTERACTIVE_CLIENT_H

#include <stdbool.h>
#include <sys/select.h>

#include "function_stack.h"
#include "hashmap.h"
#include "lantern.h"

#define UP_BUTTON 37
#define DOWN_BUTTON 33
#define VALIDATE_BUTTON 35

typedef struct {
	bool up; bool down; bool left; bool right;
	bool validate;
} input_t;

typedef struct {
	function_t update; // (widget_context_t*)
	function_t render; // (widget_context_t*)
	void* properties; // possède les valeurs
	unsigned key;
	bool requireRender;
} widget_t;

typedef struct {
	widget_t root;
	input_t input;
} gui_t;

typedef struct {
	bool running;
	socket_t socket;
	fd_set ioState;
	gui_t gui;
	function_t messageHandler; // (message_context_t*)
	void* context; // possède les valeurs
} client_t;

typedef struct {
	client_t* client;
	char** args;
	unsigned argCount;
} message_context_t;

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

void handleUpdate(gui_t* gui, client_t* client);
void handleRender(gui_t* gui, client_t* client);

void setupClientFileDescriptorSet(client_t* client, fd_set* fileDescriptorSet, int* maxFileDescriptor);
void awaitInput(client_t* client);

void testInput();
void testButton();

int mainClient(int argc, const char** argv);

void setupInitial(client_t* client);

void resetInput(input_t* input);
void scanUserInput(client_t* input);

// --- MENU -------------------------------------------

typedef struct {
	char** entries;
	unsigned entryCount;
	unsigned selection;
	function_t handleValidate; // (widget_context_t*)
} menu_properties_t;

void initMenu(widget_t* widget, char** entries, unsigned entryCount);
void dropMenu(widget_t* widget);
void* updateMenu(void* _);
void* renderMenu(void* _);

// ----------------------------------------------------

#endif
