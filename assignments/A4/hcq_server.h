#ifndef _HCQ_SERVER_H_
#define _HCQ_SERVER_H_

// #include <stdio.h>
#include <stdlib.h>
// #include <string.h>
// #include <unistd.h>


#define MAX_BACKLOG 5
#define INPUT_BUF_SIZE 256
#define OUT_BUF_SIZE 1024
#define MAX_NAME 30
#define ERROR -1
#define CHILD 0


/* A simple linked-list of Client's */
struct client {
    int client_fd;
    char *name;
    char role;                      // TA or Student
    char *course;                   // only for students
    char buf[INPUT_BUF_SIZE];       // to hold partial reads
    int inbuf;                      // # of bytes currently in buf
    int room;                       // # of bytes remaining in buf
    struct client *next;
};
typedef struct client Client;



int handle_request(int cfd, Client *clients);
void create_client(int client_fd, Client **clients, fd_set *all_fds);
int remove_client(int cfd, Client **clients, fd_set *all_fds);
int handle_rw(int cfd, Client *client, const char *buf);
int handle_ta_next(Client *client);
Client *find_client(Client *clients, const char *name);

#endif
