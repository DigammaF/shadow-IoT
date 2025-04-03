
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef SOCKET_T_H
#define SOCKET_T_H

typedef struct {
	int fileDescriptor;
	int mode;
	struct sockaddr_in local;
	struct sockaddr_in remote;
} socket_t;

#endif
