
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

#include "session.h"

#define CHECK(status, message) { if ((status) == -1) { perror(message); exit(EXIT_FAILURE); } }

/**
 *
 * 	\fn			int createSocket(const int mode, socket_t* socketObject);
 * 	\brief		Créée une socket en mode SOCK_STREAM ou SOCK_DGRAM, remplissant les champs .fileDescriptor et .mode
 *  \param		mode: SOCK_STREAM ou SOCK_DGRAM
 *  \param		socketObject: une socket vierge
 * 
 */
void createSocket(const int mode, socket_t* socketObject) {
	CHECK(socketObject->fileDescriptor = socket(AF_INET, mode, 0), "socket");
	socketObject->mode = mode;
}

/**
 * 
 * 	\fn 		int closeSocket(socket_t* socket);
 * 	\brief		Ferme une socket
 * 	\param		socket: socket en mode STREAM
 * 
 */
void closeSocket(socket_t* socket) {
	close(socket->fileDescriptor);
}

/**
 * 
 * 	\fn			int bindLocal(socket_t* socket, const char* address, const short port);
 * 	\brief		Associe la socket à une addresse, remplissant le champ .local
 *  \param		socket: socket
 * 	\param		address: adresse IP
 * 	\param		port: port
 * 
 */
void bindLocal(socket_t* socket, const char* address, const short port) {
	struct sockaddr_in addressObject;
	memset(&addressObject, 0, sizeof(addressObject));
	addressObject.sin_family = AF_INET;
	addressObject.sin_port = htons(port);
	CHECK(inet_pton(AF_INET, address, &addressObject.sin_addr), "inet_pton");
	CHECK(bind(
		socket->fileDescriptor,
		(struct sockaddr*)&addressObject, sizeof(addressObject)
	), "bind");
	socket->local = addressObject;
}

/**
 * 
 * 	\fn			int listenSocket(socket_t* socket, const int maxClientCount);
 * 	\brief		Attends que des clients soient prêt à être acceptés
 *  \param		socket: socket en mode STREAM
 * 	\param		maxClientCount: nombre maximal de client dans la file d'attente
 * 
 */
void listenSocket(socket_t* socket, const int maxClientCount) {
	listen(socket->fileDescriptor, maxClientCount);
}

/**
 * 
 * 	\fn			int acceptRemote(socket_t* socket);
 * 	\brief		Accepte un client. Les informations relatives à cette nouvelle connexion sont renseignées dans newSocket
 *  \param		socket: socket en mode STREAM
 *  \param		newSocket: la socket client en mode STREAM
 * 
 */
void acceptRemote(socket_t* socket, socket_t* newSocket) {
	struct sockaddr_in remoteAddress;
	socklen_t clientAddressLength = sizeof(remoteAddress);
	int remoteFileDescriptor;
	CHECK(remoteFileDescriptor = accept(
		socket->fileDescriptor, (struct sockaddr*)&remoteAddress, &clientAddressLength
	), "accept");
	createSocket(SOCK_STREAM, newSocket);
	newSocket->fileDescriptor = remoteFileDescriptor;
	newSocket->local = socket->local;
	newSocket->remote = remoteAddress;
}

/**
 * 
 * 	\fn			int connectToRemote(socket_t* socket, const char* address, const short port);
 * 	\brief		Se connecte à un serveur, remplissant le champ .remote
 *  \param		socket: socket en mode STREAM
 * 	\param		address: adresse IP
 * 	\param		port: port
 * 
 */
void connectToRemote(socket_t* socket, const char* address, const short port) {
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	inet_pton(AF_INET, address, &serverAddress.sin_addr);
	socklen_t serverAddressLength = sizeof(serverAddress);
	CHECK(connect(
		socket->fileDescriptor,
		(struct sockaddr*)&serverAddress, serverAddressLength
	), "connect");
	socket->remote = serverAddress;
}

/**
 * 
 * 	\fn			int setRemote(socket_t* socket, const char* address, const short port);
 * 	\brief		Indique l'addresse de destination de la socket.
 *  \param		socket: socket en mode DGRAM
 * 	\param		address: adresse IP
 * 	\param		port: port
 * 
 */
void setRemote(socket_t* socket, const char* address, const short port) {
	struct sockaddr_in addressObject;
	memset(&addressObject, 0, sizeof(addressObject));
	addressObject.sin_family = AF_INET;
	addressObject.sin_port = htons(port);
	inet_pton(AF_INET, address, &addressObject.sin_addr);
	socket->remote = addressObject;
}

void serverDGRAM(socket_t* socket, const char* address, const short port) {
	createSocket(SOCK_DGRAM, socket);
	bindLocal(socket, address, port);
}

void serverSTREAM(socket_t* socket, const char* address, const short port, const int maxClients) {
	createSocket(SOCK_STREAM, socket);
	bindLocal(socket, address, port);
	listenSocket(socket, maxClients);
}

void connectServer(socket_t* socket, const char* address, const short port) {
	createSocket(SOCK_STREAM, socket);
	connectToRemote(socket, address, port);
}
