#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

// use these to connect to teach.cs removely -> not needed in this lab
// #include <netdb.h>
// #define h_addr h_addr_list[0]


#ifndef PORT
  #define PORT 30000
#endif
#define BUF_SIZE 128

int main(void) {
    // Create the client socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("client: socket");
        exit(EXIT_FAILURE);
    }

    // Set the IP and port of the server to connect to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    memset(&(server.sin_zero), 0, 8);

    // hard-coding localhost. either of these works
    // server.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) < 1) {
        perror("client: inet_pton");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    /* use this to connect to a remove server. Add an arg, so you don't
     * need to hard-code "teach.cs..."
     */
    // struct hostent *hp = gethostbyname("teach.cs.utoronto.ca");
    // if (hp == NULL) {
    //     fprintf(stderr, "unknown host %s\n", "teach.cs.utoronto.ca");
    //     exit(EXIT_FAILURE);
    // }
    // server.sin_addr = *((struct in_addr *) hp->h_addr);

    // Connect to the server.
    if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("client: connect");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // step 1: part 1: send the username to the server
    char buf[BUF_SIZE + 1];
    printf("Enter a username: ");
    fscanf(stdin, "%128s", buf);
    if (write(sock_fd, buf, strlen(buf)) == -1) {
        perror("write username");
        exit(EXIT_FAILURE);
    }

    // Read input from the user, send it to the server, and then accept the
    // echo that returns. Exit when stdin is closed.

    // initialize fd set
    int max_fd = sock_fd;           // max so far
    fd_set all_fds, listen_fds;
    FD_ZERO(&all_fds);
    FD_SET(sock_fd, &all_fds);
    FD_SET(STDIN_FILENO, &all_fds);

    int nbytes_read;
    // Note: could set this up as a while loop with select
    for (;;) {

        listen_fds = all_fds;
        int nready = select(max_fd + 1, &listen_fds, NULL, NULL, NULL);
        if (nready == -1) {
            perror("client select");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }

        // check if stdin is ready to be read from
        if (FD_ISSET(STDIN_FILENO, &listen_fds)) {
            if ((nbytes_read = read(STDIN_FILENO, buf, BUF_SIZE)) == -1) {
                perror("stdin read");
                close(sock_fd);
                exit(EXIT_FAILURE);

            // check if stdin is closed
            } else if (nbytes_read == 0) {
                break;
            }
            buf[nbytes_read] = '\0';

            // write to the server
            if (write(sock_fd, buf, nbytes_read) != nbytes_read) {
                perror("client write");
                close(sock_fd);
                exit(EXIT_FAILURE);
            }
        }

        // check if sock_fd is ready to be read from
        // this is an echo from the server or a message from another client
        if (FD_ISSET(sock_fd, &listen_fds)) {
            if ((nbytes_read = read(sock_fd, buf, BUF_SIZE)) == -1) {
                perror("client fd read");
                close(sock_fd);
                exit(EXIT_FAILURE);
            }
            buf[nbytes_read] = '\0';

            // write to output stream?
            if (write(STDOUT_FILENO, buf, strlen(buf)) == -1) {
                perror("write stdout");
                close(sock_fd);
                exit(EXIT_FAILURE);
            }
        }
    }

    close(sock_fd);
    return EXIT_SUCCESS;
}
