#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include "../lab_msg.h"
#include <string.h>


int main(int argc, char* argv[]) {
    
    if (argc != 4) {
        printlog("Usage: client hostname port nodenum");
        return 0;
    }

    int sock;
    struct sockaddr_in server;
    struct hostent* hp;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printlog("Failed opening socket");
        return -1;
    } else {
        printlog("Socket created");
    }

    server.sin_family = AF_INET;
    hp = gethostbyname(argv[1]);
    if (hp == (struct hostent*) 0) {
        printlog("Unknown server");
        return -1;
    } 
    
    memcpy(&server.sin_addr, hp->h_addr_list[0], hp->h_length);

    int port = atoi(argv[2]);
    server.sin_port = htons(port);
    printlog("Connecting to port %i", port);
    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) != 0) {
        printlog("Failed connecting socket");
        return -1;
    }

    printlog("Connected socket");

    const char* words[10] = {
        "objective",
        "leader",
        "charismatic",
        "photocopy",
        "variable",
        "correspondence",
        "responsibility",
        "enfix",
        "consolidate",
        "helicopter",
    };

    msg_list* list = msg_list_new();
    int node_num = atoi(argv[3]);

    for (int i = 1; i <= node_num; i++) {
        char txt[TXT_SIZE];
        sprintf(txt, "Node no. %i %s", i, words[i%9]);
        msg_list_add(list, 1000000000000000 - i * 100, i * 2, txt);
        printlog("Added node no %i", i);
        
    }

    print_list(list, printlog);

    if(send_list(list, sock) != 0) {
        printlog("Program failed");
        return  -1;
    }

    printlog("Program finished successfully");
    close(sock);
      
}