#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "socket.h"

#ifndef PORT
    #define PORT 30000
#endif

#define BUFSIZE 30


int main() {
    struct sockaddr_in *server_addr = init_server_addr(PORT);
    int listen_soc = set_up_server_socket(server_addr, 5);

    while (1) {
        int client_soc = accept_connection(listen_soc);
        if (client_soc < 0) {
            continue;
        }

        // Receive messages
        char buf[BUFSIZE];
        int nbytes;
        while ((nbytes = read(client_soc, buf, sizeof(buf) - 1)) > 0) {
            buf[nbytes] = '\0';
            printf("Next message: %s\n", buf);
        }
        close(client_soc);
    }

    free(server_addr);
    close(listen_soc);
    return 0;
}
