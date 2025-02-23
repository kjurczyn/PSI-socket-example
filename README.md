# Simple TCP Client/Server Example
This repository contains a simple TCP client and server written in C. The client generates and sends a linked list of nodes containing a string of text and some integers. The server receives and prints the list.

## Directories
```
.
├── client                   # Files of the client
├── server                   # Files of the server
├── build.sh                 # Script for building Docker images of client and server
├── create.sh                # Script for creating Docker containers of client and server
├── lab_msg.c                # Implementation of the lab_msg library 
├── lab_msg.h                # Header file of the lab_msg library
└── README.md
```

## Building
The client and server can both be built using `make` in their respective directories.

## Usage
The server must be started and listeting before the client is started.
### Client
`client <hostname> <port> <numnodes>`
 - hostname - hostname of the server
 - port - port no. of the server
 - numnodes - number of nodes to sednd

### Server
`server <port>`
- port - port no. on which the server listens