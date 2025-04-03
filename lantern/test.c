
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include "lantern.h"

#define CHECK(status, message) { if ((status) == -1) { perror(message); exit(EXIT_FAILURE); } }

int main() {
	socket_t socket;
	socket_t clientSocket;
	serverSTREAM(&socket, "127.0.0.1", 20000, 5);

	while (1) {
		acceptRemote(&socket, &clientSocket);
		char buffer[1024];
		recvData(&clientSocket, (char*)&buffer, 1024);
		printf("'%s'\n", buffer);
		sendData(&clientSocket, "back\0");
		closeSocket(&clientSocket);
	}

	closeSocket(&socket);
}
