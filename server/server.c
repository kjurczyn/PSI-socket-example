#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../lab_msg.h"


int main(int argc, char* argv[]) {

    if (argc != 2) {
        printlog("Usage: server port | argc:%i", argc);
        return -1;
    }

    int listen_sock, conn_sock;
    struct sockaddr_in server, client;
    
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1) {
        printlog("Failed opening socket");
        return -1;
    } else {
        printlog("Socket created");
    }

    int port = atoi(argv[1]);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    
    if((bind(listen_sock, (struct sockaddr*) &server, sizeof(server))) != 0) {
        printlog("Failed to bind socket");
        return -1;
    }
   
    printlog("Bound socket");
    
    if (listen(listen_sock, 5) != 0) {
        printlog("Listen failed");
        return -1;
    }

    printlog("Listening on port %i", port);

    unsigned int client_len = sizeof(client);
    conn_sock = accept(listen_sock, (struct sockaddr*) &client, &client_len);

    if (conn_sock < 0) {
        printlog("Failed accept");
        return -1;
    }

    printlog("Accepted connection from %s", inet_ntoa(client.sin_addr));

    msg_list* list = recv_list(conn_sock);
    if (!list) {
        printlog("Failed to receive list");
        return -1;
    }
    
    print_list(list, printlog);

    printlog("Program finished successfully");

    close(listen_sock);
    return 0;
}