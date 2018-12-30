#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>


#ifndef PORT
  #define PORT 30000
#endif
#define MAX_BACKLOG 5
#define MAX_CONNECTIONS 12
#define BUF_SIZE 128


struct sockname {
    int sock_fd;
    char *username;
};


/* Accept a connection. Note that a new file descriptor is created for
 * communication with the client. The initial socket descriptor is used
 * to accept connections, but the new socket is used to communicate.
 * Return the new client's file descriptor or -1 on error.
 */
int accept_connection(int fd, struct sockname *usernames) {

    // find an empty spot in the usernames list
    int i = 0;
    while (i < MAX_CONNECTIONS && usernames[i].sock_fd != -1) {
        i++;
    }

    // we already have the max number of clients connected
    if (i == MAX_CONNECTIONS) {
        fprintf(stderr, "server: max concurrent connections\n");
        return -1;
    }

    // not used in this program. (client_addr/peer)
    struct sockaddr_in client_addr;
    unsigned int peer_len = sizeof(client_addr);
    client_addr.sin_family = AF_INET;

    // get the fd to communicate with the client
    int client_fd = accept(fd, (struct sockaddr *)&client_addr, &peer_len);
    if (client_fd == -1) {
        perror("server: accept");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // add the username to the list
    char username[BUF_SIZE + 1];
    int bytes_read;
    if ((bytes_read = read(client_fd, username, BUF_SIZE)) == -1) {
        perror("server read username");
        close(client_fd);
        close(fd);
        exit(EXIT_FAILURE);
    }
    username[bytes_read] = '\0';

    // add the client to the list
    usernames[i].sock_fd = client_fd;
    usernames[i].username = strdup(username);  // remember to free
    return client_fd;
}


/* Read a message from client_index and echo it to all connected clients.
 * Return the fd if it has been closed or 0 otherwise.
 */
int read_from(int client_index, struct sockname *usernames) {

    // get the fd socket of this client
    int fd = usernames[client_index].sock_fd;

    // prepend the username of the message
    char message[BUF_SIZE + 1];
    strncpy(message, usernames[client_index].username, sizeof(message));
    message[BUF_SIZE] = '\0';
    strncat(message, ": ", sizeof(message) - strlen(message) - 1);

    // read message from this client. This read will not to block
    int bytes_read;
    char buf[BUF_SIZE + 1];
    if ((bytes_read = read(fd, buf, BUF_SIZE)) == -1) {
        perror("server read");
        close(fd);
        exit(EXIT_FAILURE);
    }
    buf[bytes_read] = '\0';

    // check if this client disconnected
    if (bytes_read == 0) {
        return fd;
    }

    // echo the message to all the clients connected to the server
    // need to fix this
    strncat(message, buf, sizeof(message) - strlen(message) - 1);
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        int client_fd = usernames[i].sock_fd;
        if (client_fd != -1) {
            if (write(client_fd, message, strlen(message)) != strlen(message)) {
                perror("server write");
                close(client_fd);
                close(fd);
                exit(EXIT_FAILURE);
            }
        }
    }

    return 0;
}


int main(void) {

    // initialize usernames array
    struct sockname usernames[MAX_CONNECTIONS];
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        usernames[i].sock_fd = -1;
        usernames[i].username = NULL;
    }

    // Create the listen socket FD (server_soc/listen_soc).
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("server: socket");
        exit(EXIT_FAILURE);
    }

    // Set information about the port (and IP) we want to be connected to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    // This should always be zero. On some systems, it won't error if you
    // forget, but on others, you'll get mysterious errors. So zero it.
    memset(&server.sin_zero, 0, 8);

    // Bind the selected port to the socket.
    if (bind(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("server: bind");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // Announce willingness to accept connections on this socket.
    if (listen(sock_fd, MAX_BACKLOG) == -1) {
        perror("server: listen");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // initialize fd set
    int max_fd = sock_fd;            // max so far
    fd_set all_fds, listen_fds;
    FD_ZERO(&all_fds);
    FD_SET(sock_fd, &all_fds);

    while (1) {
        // select updates the fd_set it receives, so we always use a copy and
        // retain the original.
        listen_fds = all_fds;
        int nready = select(max_fd + 1, &listen_fds, NULL, NULL, NULL);
        if (nready == -1) {
            perror("server: select");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }

        // Is it the original socket? Create a new connection ...
        if (FD_ISSET(sock_fd, &listen_fds)) {
            int client_fd = accept_connection(sock_fd, usernames);
            if (client_fd == -1) {
                continue;
            }
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
            FD_SET(client_fd, &all_fds);

            for (int i = 0; i < MAX_CONNECTIONS; i++) {
                if (usernames[i].sock_fd == client_fd) {
                    printf("%s connected\n", usernames[i].username);
                }
            }
        }

        // find the clients that have something to say
        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            int client_fd = usernames[i].sock_fd;

            if (client_fd > -1 && FD_ISSET(client_fd, &listen_fds)) {

                // this client has something to say. Note: never reduces max_fd
                int client_closed = read_from(i, usernames);
                if (client_closed > 0) {
                    FD_CLR(client_closed, &all_fds);
                    printf("Client %s disconnected\n", usernames[i].username);
                    close(usernames[i].sock_fd);
                    usernames[i].sock_fd = -1;
                    free(usernames[i].username);
                    usernames[i].username = NULL;
                } else {
                    printf("Echoing message from client %s\n",
                                                        usernames[i].username);
                }
            }
        }
    }

    // Should never get here.
    return EXIT_FAILURE;
}
