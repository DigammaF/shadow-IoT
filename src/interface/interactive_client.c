
#include <stdlib.h>
#include <stdio.h>

#include "constants.h"
#include "function_stack.h"
#include "interactive_client.h"
#include "string_utils.h"

#define UNUSED(x) (void)(x)
#define CHECKM(status, message) { if ((status) == NULL) { perror(message); exit(EXIT_FAILURE); } }

void initClient(client_t* client) {
	client->messageHandler = noHandler;
	client->context = NULL;
}

void dropClient(client_t* client) { UNUSED(client); }

void initWidget(widget_t* widget) {
	widget->update = noHandler;
	widget->render = noHandler;
	widget->properties = NULL;
}

void dropWidget(widget_t* widget) { UNUSED(widget); }

void initGUI(gui_t* gui) { UNUSED(gui); }

void dropGUI(gui_t* gui) { UNUSED(gui); }

void handleUpdate(gui_t* gui, client_t* client) {
	widget_context_t context = { .client = client, .widget = &gui->root };
	gui->root.update(&context);
}

void handleRender(gui_t* gui, client_t* client) {
	widget_t root = gui->root;

	if (root.requireRender) {
		printf("\n\n\n\n\n\n");
		widget_context_t context = { .client = client, .widget = &root };
		root.render(&context);
	}
}

void setupClientFileDescriptorSet(client_t* client, fd_set* fileDescriptorSet, int* maxFileDescriptor) {
	socket_t* clientSocket = &client->socket;
	FD_ZERO(fileDescriptorSet);
	
	// server
	FD_SET(clientSocket->fileDescriptor, fileDescriptorSet);
	*maxFileDescriptor = clientSocket->fileDescriptor;
	
	// keyboard
	int stdinFileDescriptor = fileno(stdin);
	FD_SET(stdinFileDescriptor, fileDescriptorSet);
	if (stdinFileDescriptor > *maxFileDescriptor) { *maxFileDescriptor = stdinFileDescriptor; }
	
	return;
}

void awaitInput(client_t* client) {
	int maxFileDescriptor;
	struct timeval timeout = { 0, SERVER_TICK };
	setupClientFileDescriptorSet(client, &client->ioState, &maxFileDescriptor);
	select(maxFileDescriptor + 1, &client->ioState, NULL, NULL, &timeout);
}

int mainClient(int argc, const char** argv) {
	UNUSED(argc); UNUSED(argv);
	socket_t clientSocket;
	connectServer(&clientSocket, argv[2], SERVER_PORT);
	client_t client;
	initClient(&client);
	client.socket = clientSocket;
	client.running = 1;
	setupInitial(&client);

	while (client.running) {
		awaitInput(&client);

		// read server
		while (FD_ISSET(client.socket.fileDescriptor, &client.ioState)) {
			message_context_t context;
			context.client = &client;
			char data[1024];
			int byteCount = recvData(&client.socket, data, 1024);
			context.args = splitString(data, &context.argCount);

			if (byteCount == 0) { client.running = 0; return 0; }

			client.messageHandler(&context);
		}

		handleUpdate(&client.gui, &client);
		handleRender(&client.gui, &client);
	}
}

void setupInitial(client_t* client) {

}

void resetInput(input_t* input) {
	input->down = false;
	input->up = false;
	input->right = false;
	input->left = false;
	input->validate = false;
}

void scanUserInput(input_t* input) { UNUSED(input); } // TODO

// --- MENU -------------------------------------------

void initMenu(widget_t* widget, char** entries, unsigned entryCount) {
	menu_properties_t* properties = malloc(sizeof(menu_properties_t));
	CHECKM(properties, "malloc menu_properties_t");
	properties->entries = entries;
	properties->entryCount = entryCount;
	properties->selection = 0;
	properties->handleValidate = noHandler;
	widget->properties = properties;
	widget->update = updateMenu;
	widget->render = renderMenu;
}

void dropMenu(widget_t* widget) {
	free(widget->properties);
}

void* updateMenu(void* _) {
	widget_context_t* context = (widget_context_t*)_;
	client_t* client = context->client;
	widget_t* widget = context->widget;
	menu_properties_t* properties = (menu_properties_t*)widget->properties;

	if (client->gui.keyboard.up) {
		properties->selection--;
		widget->requireRender = true;

		if (properties->selection < 0) { properties->selection = properties->entryCount; }
	}

	if (client->gui.keyboard.down) {
		properties->selection++;
		widget->requireRender = true;

		if (properties->selection >= properties->entryCount) { properties->selection = 0; }
	}

	if (client->gui.keyboard.validate) { properties->handleValidate(context); }
}

void* renderMenu(void* _) {
	widget_context_t* context = (widget_context_t*)_;
	widget_t* widget = context->widget;
	menu_properties_t* properties = (menu_properties_t*)widget->properties;

	for (int n = 0; n < properties->entryCount; n++) {
		if (n == properties->selection) { printf("(*) %s"); }
		else { printf("( ) %s"); }
	}
}

// ----------------------------------------------------
