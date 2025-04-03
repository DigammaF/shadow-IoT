
#ifndef DATA_H
#define DATA_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "session.h"

int sendData(socket_t* socket, const char* data);
int sendDGRAM(socket_t* socket, const char* address, const short port, const char* data);
int recvData(socket_t* socket, char* data, size_t maxDataLength);
int recvDGRAM(socket_t* socket, char* data, size_t maxDataLength);

#endif
