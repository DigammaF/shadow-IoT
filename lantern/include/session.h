
#ifndef SESSION_H
#define SESSION_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "socket_t.h"

void createSocket(const int mode, socket_t* socketObject);
void closeSocket(socket_t* socket);
void bindLocal(socket_t* socket, const char* address, const short port);
void listenSocket(socket_t* socket, const int maxClientCount);
void acceptRemote(socket_t* socket, socket_t* newSocket);
void connectToRemote(socket_t* socket, const char* address, const short port);
void setRemote(socket_t* socket, const char* address, const short port);

void serverDGRAM(socket_t* socket, const char* address, const short port);
void serverSTREAM(socket_t* socket, const char* address, const short port, const int maxClients);
void connectServer(socket_t* socket, const char* address, const short port);

#endif
