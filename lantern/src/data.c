
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

#include "data.h"

/**
 * 
 * 	\fn			int sendData(socket_t* socket, const char* data);
 * 	\brief		Envoie des données à .remote
 *  \param		socket: la socket à utiliser
 *  \param		data: chaîne de charactères terminée par un zéro
 * 	\result		nombre d'octets envoyés
 * 
 */
int sendData(socket_t* socket, const char* data) {
	size_t totalSent = 0;
    size_t dataLen = strlen(data) + 1;
    ssize_t bytesSent;

    while (totalSent < dataLen) {
        bytesSent = send(socket->fileDescriptor, data + totalSent, dataLen - totalSent, 0);
        if (bytesSent < 0) { return -1; }
        totalSent += bytesSent;
    }

    return (int)totalSent;
}

// TODO mettre à jour sur le principe de sendData
int sendDGRAM(socket_t* socket, const char* address, const short port, const char* data) {
	ssize_t bytesSent = sendto(socket->fileDescriptor, data, strlen(data) + 1, 0,
		(struct sockaddr*)&(socket->remote), sizeof(socket->remote)
	);
	if (bytesSent < 0) { return -1; }
	return (int)bytesSent;
}

/**
 * 
 * 	\fn			int recvData(socket_t* socket, char* data);
 * 	\brief		Reçoit des données depuis .remote; en mode SOCK_DGRAM écrit .remote
 *  \param		socket: la socket à utiliser
 *  \param		data: chaîne de charactères terminée par un zéro
 *  \param		maxDataLength: la longueur maximale des données reçues
 * 	\result		nombre d'octets recus
 * 
 * TODO ajouter un timeout
 * 
 */
int recvData(socket_t* socket, char* data, size_t maxDataLength) {
	size_t head = 0;

	while (1) {
		if (head >= maxDataLength - 1) { perror("lantern recv failed due to reaching communication size limit before reaching delimiter"); exit(EXIT_FAILURE); }
		char character;
		ssize_t bytesReceived = recv(socket->fileDescriptor, &character, 1, 0);
		if (bytesReceived < 0) { perror("lantern recv failed due to stopping the communication before reaching delimiter"); exit(EXIT_FAILURE); }
		if (character == '\0') { break; }
		data[head] = character;
		head += bytesReceived;
	}

	data[head] = '\0';
	return (int)head;
}

int recvDGRAM(socket_t* socket, char* data, size_t maxDataLength) {
	struct sockaddr_in client_address;
	socklen_t client_address_length = sizeof(client_address);
	ssize_t bytesReceived = recvfrom(
		socket->fileDescriptor, data, maxDataLength - 1, 0,
		(struct sockaddr*)&client_address, &client_address_length
	);
	if (bytesReceived < 0) { return -1; }
	socket->remote = client_address;
	data[bytesReceived] = '\0';
	return (int)bytesReceived;
}
