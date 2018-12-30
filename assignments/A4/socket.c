#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>     /* inet_ntoa */
#include <netdb.h>         /* gethostname */
#include <sys/socket.h>

#include "socket.h"

#define h_addr h_addr_list[0] // macro only on some systems

/*
 * Initialize a server address associated with the given port.
 */
void init_server_addr(struct sockaddr_in *server, int port) {

    // Allow sockets across machines.
    server->sin_family = PF_INET;

    // The port the process will listen on.
    server->sin_port = htons(port);

    // Clear this field; sin_zero is used for padding for the struct.
    memset(&(server->sin_zero), 0, 8);

    // Listen on all network interfaces.
    server->sin_addr.s_addr = INADDR_ANY;

}

/*
 * Create and set up a socket for a server to listen on.
 */
int set_up_server_socket(struct sockaddr_in *server, int num_queue) {

    int listen_soc = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_soc == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Make sure we can reuse the port immediately after the
    // server terminates. Avoids the "address in use" error
    int on = 1;
    int status = setsockopt(listen_soc, SOL_SOCKET, SO_REUSEADDR,
        (const char *) &on, sizeof(on));
    if (status == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Associate the process with the address and a port.
    // Bind the selected port to the socket.
    if (bind(listen_soc, (struct sockaddr *)server, sizeof(*server)) == -1) {
        // bind failed; could be because port is in use.
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Set up a queue in the kernel to hold pending connections.
    if (listen(listen_soc, num_queue) == -1) {
        // listen failed
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return listen_soc;
}


/*
 * Wait for and accept a new connection.
 * Return -1 if the accept call failed.
 */
int accept_connection(int listen_fd) {

    // peer is the client_addr
    struct sockaddr_in peer;
    unsigned int peer_len = sizeof(peer);
    peer.sin_family = PF_INET;

    // remember that accept is a blocking system call but we know that this
    // is guarenteed not to block because of select
    int client_socket = accept(listen_fd, (struct sockaddr *)&peer, &peer_len);
    if (client_socket == -1) {
        perror("accept");
        return -1;
    } else {
        printf("New connection accepted from %s:%d\n",
                             inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
        return client_socket;
    }
}


/******************************************************************************
 * Client-specific functions
 *****************************************************************************/
/*
 * Create a socket and connect to the server indicated by the port and hostname
 */
int connect_to_server(int port, const char *hostname) {
    int client_soc = socket(PF_INET, SOCK_STREAM, 0);
    if (client_soc < 0) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in server;

    // Allow sockets across machines.
    server.sin_family = PF_INET;
    // The port the server will be listening on.
    server.sin_port = htons(port);
    // Clear this field; sin_zero is used for padding for the struct.
    memset(&(server.sin_zero), 0, 8);

    // Lookup host IP address.
    struct hostent *hp = gethostbyname(hostname);
    if (hp == NULL) {
        fprintf(stderr, "unknown host %s\n", hostname);
        exit(1);
    }

    server.sin_addr = *((struct in_addr *) hp->h_addr);

    // Request connection to server.
    if (connect(client_soc, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("connect");
        exit(1);
    }

    return client_soc;
}

