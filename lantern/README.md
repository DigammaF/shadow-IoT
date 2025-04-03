# README.md

# My C Library

This project is a C library for managing socket connections and sending/receiving data over sockets. It provides a set of functions to create, bind, listen, accept, and connect sockets, as well as to send and receive data.

## Project Structure

```
my-c-library
├── src
│   ├── data.c          # Implementation of data sending and receiving functions
│   ├── session.c       # Implementation of socket management functions
│   └── socket_t.h      # Definition of the socket_t structure
├── include
│   ├── data.h          # Declarations for data sending and receiving functions
│   ├── session.h       # Declarations for socket management functions
│   └── socket_t.h      # Declaration of the socket_t structure
├── Makefile             # Build script to compile the library
└── README.md            # Documentation for the project
```

## Functions

### Data Functions
- `sendData`: Sends data to a remote socket.
- `recvData`: Receives data from a remote socket.

### Session Functions
- `createSocket`: Creates a socket in either SOCK_STREAM or SOCK_DGRAM mode.
- `closeSocket`: Closes a socket.
- `bindLocal`: Binds a socket to a local address and port.
- `listenSocket`: Listens for incoming connections on a socket.
- `acceptRemote`: Accepts a remote connection.
- `connectToRemote`: Connects to a remote server.
- `setRemote`: Sets the remote address and port for a socket in DGRAM mode.

## Compilation

To compile the library, run the following command in the project directory:

```
make
```

This will create a static library named `lantern.a` in the project directory.

## Usage

Include the header files in your C source files to use the library functions:

```c
#include "data.h"
#include "session.h"
#include "socket_t.h"
```

## License

This project is licensed under the MIT License. See the LICENSE file for more details.