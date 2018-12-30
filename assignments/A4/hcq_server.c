#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include <signal.h>
#include <syslog.h>
#include <sys/wait.h>
#include <errno.h>

#include "hcq.h"
#include "socket.h"
#include "rdwrn.h"
#include "hcq_server.h"

#ifndef PORT
  #define PORT 53974
#endif


// Use global variables so we can have exactly one TA list and one student list
Ta *ta_list = NULL;
Student *stu_list = NULL;

Course *courses;
int num_courses = 3;

// linked-list of clients
Client *clients = NULL;


Client *find_client(Client *clients, const char *name) {
    Client *cur = clients;
    while (cur != NULL) {
        if (strcmp(cur->name, name) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}


int handle_ta_next(Client *client) {
    // find the Ta
    Ta *cur_ta = ta_list;
    while (cur_ta != NULL) {
        if (strcmp(client->name, cur_ta->name) == 0) {
            break;
        }
        cur_ta = cur_ta->next;
    }

    // if there is no next student, then just remove the current student
    // from this TA.
    if (stu_list == NULL) {
        next_overall(cur_ta->name, &ta_list, &stu_list);
        return 0;
    }

    // otherwise, find the Client to disconnect
    Client *next_stu = clients;
    int fd_disconnect = -1;
    while (next_stu != NULL) {
        if (strcmp(next_stu->name, stu_list->name) == 0) {
            fd_disconnect = next_stu->client_fd;
            break;
        }
        next_stu = next_stu->next;
    }

    // take the next student
    if (ta_list != NULL) {
        next_overall(cur_ta->name, &ta_list, &stu_list);
    }

    // check if there is a next student
    if (fd_disconnect != -1) {
        char *msg = "Your turn to see the TA.\r\nWe are disconnecting "
                    "you from the server now. Press Ctrl-C to close nc\r\n";
        if (writen(fd_disconnect, msg, strlen(msg)) != strlen(msg)) {
            perror("write next student");
        }
        return fd_disconnect;
    }
    return 0;
}


/* Handle a read message from the client. Return the client fd <cfd> if the
 * client has left or there is an error, otherwise return 0
 */
int handle_rw(int cfd, Client *client, const char *buf) {
    // check if we read the name
    if (client->name == NULL) {
        if (strlen(buf) < MAX_NAME * sizeof(char)) {
            client->name = strdup(buf);
        } else {
            return cfd;              // kick them out
        }

        char *msg = "Are you a TA or a Student (enter T or S)?\r\n";
        if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
            perror("write T or S");
            return cfd;
        }

    // check if we read a role
    } else if (client->role == 'Z') {
        // we found a TA
        if (buf[0] == 'T') {
            char *msg = "Valid commands for TA:\r\n\tstats\n\r\tnext"
                                        "\r\n\t(or use Ctrl-C to leave)\r\n";
            if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
                perror("write T or S second time");
                return cfd;
            }
            client->role = 'T';
            // add to the list of TA's
            add_ta(&ta_list, client->name);

        // we found a Student
        } else if (buf[0] == 'S') {
            char *msg = "Valid courses: CSC108, CSC148, CSC209\r\n"
                                    "Which course are you asking about?\r\n";

            if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
                perror("write asking for course");
                return cfd;
            }
            client->role = 'S';

        // user entered an invalid role
        } else {
            char *msg = "Invalid role. Enter (T or S).\r\n";
            if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
                perror("write T or S second time");
                return cfd;
            }
            fprintf(stderr, "Client [%s] entered invalid role\n",
                                                        client->name);
        }

    // check if we read a course code
    } else if (client->role == 'S' && client->course == NULL) {
        if ((strcmp("CSC108", buf) == 0 || strcmp("CSC148", buf) == 0 ||
                                                strcmp("CSC209", buf) == 0)) {
            char *msg = "You have been entered into the queue. While you wait, "
                            "you can use the command stats to see which TAs "
                                        "are currently serving students.\r\n";
            if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
                perror("write S joined queue");
                return cfd;
            }
            client->course = strdup(buf);
            add_student(&stu_list, client->name, client->course,
                                                        courses, num_courses);
        } else {
            fprintf(stderr, "Client [%s] entered an invalid course code\n",
                                                            client->name);
            char *msg = "That is not a valid course. Good-bye.\r\n";
            if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
                perror("write S joined queue");
            }
            return cfd;
        }

    // check if a TA asked for stats
    } else if (client->role == 'T' && strcmp("stats", buf) == 0) {
        char *msg = print_full_queue(stu_list);
        if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
            perror("write TA stats");
            return cfd;
        }
        free(msg);

    // check if a TA asked for the next student
    } else if (client->role == 'T' && strcmp("next", buf) == 0) {

        int ret = handle_ta_next(client);
        if (ret > 0) {
            return ret;
        }

    // check if a Student asked for stats
    } else if (client->role == 'S' && client->course != NULL &&
                                                    strcmp("stats", buf) == 0) {
        char *msg = print_currently_serving(ta_list);
        if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
            perror("write curr serving");
            return cfd;
        }
        free(msg);

    // client entered an invalid message
    } else {
        char *msg = "Incorrect syntax\r\n";
        if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
            perror("write else block");
            return cfd;
        }
    }

    // success
    return 0;
}


/* Return the client fd socket if the client disconnected or there is an
 * error. Otherwise return 0.
 */
int handle_request(int cfd, Client *clients) {
    // find the client of interest
    Client *cur = clients;
    if (cur == NULL) {
        fprintf(stderr, "Client [%d] not found\n", cfd);
        return cfd;
    }
    while (cur != NULL) {
        if (cfd == cur->client_fd) {
            break;                      // found
        }
        cur = cur->next;
    }

    /* Read the message from the client */
    int nbytes = 0;                              // # of bytes read
    int close_cfd = -1;
    char *after = &cur->buf[cur->inbuf];
    if ((nbytes = read(cfd, after, cur->room - 1)) > 0) {
        // update the number of bytes currently in buf
        cur->inbuf += nbytes;

        int where;
        while ((where = find_network_newline(cur->buf, cur->inbuf)) > 0) {
            cur->buf[where - 2] = '\0';

            /* we now have a complete message */
            close_cfd = handle_rw(cfd, cur, cur->buf);

            // check what the host sent to the server
            printf("Message received from client [%s]: %s\n", cur->name,
                                                                    cur->buf);

            // shift bytes over to start a new string
            memmove(&cur->buf[0], &cur->buf[where], cur->inbuf - where);
            cur->inbuf -= where;
        }
        after = &cur->buf[cur->inbuf];
        cur->room = INPUT_BUF_SIZE - cur->inbuf;
        memset(after, '\0', cur->room);

        // check if we are disconnecting a client
        if (close_cfd > 0) {
            return close_cfd;
        }
    }
    if (nbytes == -1) {
        perror("read from client");
        return cfd;

    // If we checked FD_ISSET to see if there was something to read and
    // nbytes == 0, then the client closed their end of the socket fd.
    } else if (nbytes == 0) {
        return cfd;
    }

    // success
    return 0;
}


/* Create a new client and add to the back of the list, then ask for a
 * name.
 */
void create_client(int client_fd, Client **clients, fd_set *all_fds) {

    // create the new Client struct and set values
    Client *new_client = malloc(sizeof(Client));
    if (new_client == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    new_client->client_fd = client_fd;
    new_client->name = NULL;
    new_client->course = NULL;
    new_client->role = 'Z';                         // unassigned role
    memset(new_client->buf, '\0', INPUT_BUF_SIZE);
    new_client->inbuf = 0;                      // # of bytes currently in buf
    new_client->room = INPUT_BUF_SIZE;          // # of bytes remaining in buf

    // insert into front of the list
    new_client->next = *clients;
    *clients = new_client;

    // send welcome message to client
    char *greeting = "Welcome to the Help Centre, what is your name?\r\n";
    if (writen(client_fd, greeting, strlen(greeting)) != strlen(greeting)) {
        perror("server write");
        remove_client(client_fd, clients, all_fds);
    }
}


/* Remove Client <cfd> from the list of connected Client's */
int remove_client(int cfd, Client **clients, fd_set *all_fds) {

    /* find the correct client */
    Client *prev = NULL;
    Client *head = *clients;

    // Case: empty list
    if (head == NULL) {
        fprintf(stderr, "removing client from empty list\n");
        close(cfd);
        exit(EXIT_FAILURE);

    // Case: client is at the head
    } else if (head->client_fd == cfd) {
        *clients = head->next;

    // Case: client is in the middle or end
    } else {
        while (head != NULL) {
            if (cfd == head->client_fd) {
                break;                          // found
            }
            prev = head;
            head = head->next;
        }
        // update list
        prev->next = head->next;
    }

    // close socket
    FD_CLR(cfd, all_fds);
    if (close(cfd) == -1) {
        perror("close client fd");
        exit(EXIT_FAILURE);
    }

    // check if this is a TA or student and remove from other lists
    int cfd_to_disconnect = -1;
    if (head->role == 'T') {
        // Note: If this TA has a current student,
        // it should already be disconnected
        if ((remove_ta(&ta_list, head->name)) == 1) {
            fprintf(stderr, "remove_ta error\n");
            exit(EXIT_FAILURE);
        }
    } else if (head->role == 'S') {
        // note: no error check.
        // If TA said "next" then this will fail, but that's ok.
        // This is only to remove a student when Ta DID NOT call "next".
        give_up_waiting(&stu_list, head->name);
    }

    // free and goodbye
    printf("Client [%s] disconnected from server\n", head->name);
    free(head->name);
    free(head->course);
    free(head);

    // check if we are disconnecting a TA's student
    if (cfd_to_disconnect > -1) {
        return cfd_to_disconnect;
    }
    return 0;
}


/* Help Centre server */
int main(void) {
    // set up list of courses
    if ((courses = malloc(sizeof(Course) * 3)) == NULL) {
        perror("malloc for course list\n");
        exit(EXIT_FAILURE);
    }
    strcpy(courses[0].code, "CSC108");
    strcpy(courses[1].code, "CSC148");
    strcpy(courses[2].code, "CSC209");

    // set up info about port and create a socket fd to listen
    struct sockaddr_in server;
    init_server_addr(&server, PORT);
    int lfd = set_up_server_socket(&server, MAX_BACKLOG);


    // initialize fd set
    int max_fd = lfd;            // max so far
    fd_set all_fds, listen_fds;
    FD_ZERO(&all_fds);
    FD_SET(lfd, &all_fds);

    // infite loop
    for(;;) {

        // select modifies the fd_set it receives, so we always use a copy and
        // retain the original.
        listen_fds = all_fds;
        int nready = select(max_fd + 1, &listen_fds, NULL, NULL, NULL);
        if (nready == -1) {
            perror("server select");
            close(lfd);
            exit(EXIT_FAILURE);
        }

        // is it the listening socket? Create a new connection ...
        if (FD_ISSET(lfd, &listen_fds)) {
            int client_fd = accept_connection(lfd);
            if (client_fd == -1) {
                continue;
            }
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
            // add the new client to the fd set
            FD_SET(client_fd, &all_fds);

            // initialize the new client and add to the clients list
            create_client(client_fd, &clients, &all_fds);
        }

        // check if any clients have anything to say
        for (Client *cur = clients; cur != NULL; cur = cur->next) {
            int cfd = cur->client_fd;
            if (FD_ISSET(cfd, &listen_fds)) {

                // read what this client has sent
                int cfd_closed = handle_request(cfd, clients);
                if (cfd_closed > 0) {

                    while (cfd_closed > 0) {
                        cfd_closed = remove_client(cfd_closed, &clients,
                                                                    &all_fds);
                    }
                } else {
                    // printf("Message received from [%s]\n", cur->name);
                }
            }
        }
    }

    // should never get here
    return EXIT_FAILURE;
}
