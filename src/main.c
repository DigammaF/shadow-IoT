
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "server.h"
#include "client.h"
#include "stack.h"
#include "hashmap.h"

int main(int argc, const char** argv) {
	if (argc > 1) {
		if (strcmp(argv[1], "server") == 0) { mainServer(argc, argv); }
		if (strcmp(argv[1], "client") == 0) { mainClient(argc, argv); }
		if (strcmp(argv[1], "blind") == 0) { mainBlindClient(argc, argv); }
		if (strcmp(argv[1], "guiclient") == 0) { mainGUIClient(argc, argv); }
		if (strcmp(argv[1], "test") == 0) { clientTest(); }
	}

	return 0;
}
