// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>

// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <sys/select.h>

// #include <signal.h>
// #include <syslog.h>
// #include <sys/wait.h>
// #include <errno.h>

// #include "hcq.h"
// #include "socket.h"
// #include "rdwrn.h"
// #include "hcq_server.h"

// #ifndef PORT
//   #define PORT 53974
// #endif


// // Use global variables so we can have exactly one TA list and one student list
// Ta *ta_list = NULL;
// Student *stu_list = NULL;

// Course *courses;
// int num_courses = 3;

// // linked-list of clients
// Client *clients = NULL;



// /* Help Centre server */
// int main(void) {

//     /* Establish SIGCHLD handler to reap terminated child processes */
//     struct sigaction sa;
//     sa.sa_flags = 0;
//     sa.sa_handler = grim_reaper;
//     if (sigemptyset(&sa.sa_mask) == -1) {
//         perror("sigemptyset");
//         exit(EXIT_FAILURE);
//     }
//     if (sigaction(SIGCHLD, &sa, NULL) == -1) {
//         perror("sigaction");
//         exit(EXIT_FAILURE);
//     }

//     // set up list of courses
//     if ((courses = malloc(sizeof(Course) * 3)) == NULL) {
//         perror("malloc for course list\n");
//         exit(EXIT_FAILURE);
//     }
//     strcpy(courses[0].code, "CSC108");
//     strcpy(courses[1].code, "CSC148");
//     strcpy(courses[2].code, "CSC209");

//     // set up info about port and create a socket fd to listen
//     struct sockaddr_in server;
//     init_server_addr(&server, PORT);
//     int lfd = set_up_server_socket(&server, MAX_BACKLOG);


//     // initialize fd set
//     int max_fd = lfd;            // max so far
//     fd_set all_fds, listen_fds;
//     FD_ZERO(&all_fds);
//     FD_SET(lfd, &all_fds);

//     // infite loop
//     for(;;) {

//         // select updates the fd_set it receives, so we always use a copy and
//         // retain the original.
//         listen_fds = all_fds;
//         int nready = select(max_fd + 1, &listen_fds, NULL, NULL, NULL);
//         if (nready == -1) {
//             perror("server select");
//             close(lfd);
//             exit(EXIT_FAILURE);
//         }

//         // Is it the original socket? Create a new connection ...
//         if (FD_ISSET(lfd, &listen_fds)) {
//             int client_fd = accept_connection(lfd);
//             if (client_fd == -1) {
//                 continue;
//             }
//             if (client_fd > max_fd) {
//                 max_fd = client_fd;
//             }
//             FD_SET(client_fd, &all_fds);

//             // initialize the new client
//             create_client(client_fd, &clients, &all_fds);

//             /* Handle each client request in a new child process */
//             int pipe_fd[2];
//             if (pipe(pipe_fd) == -1) {
//                 perror("pipe");
//                 remove_client(client_fd);
//                 close(client_fd);
//                 continue;
//             }
//             switch(fork()) {
//                 case ERROR:
//                     perror("fork");
//                     remove_client(client_fd);
//                     close(client_fd);           // give up on this client
//                     close(pipe_fd[0]);
//                     close(pipe_fd[1]);
//                     break;                      // try next client

//                 case CHILD:
//                     // child doesn't need listening socket or read pipe
//                     if ((close(lfd)) == -1) {
//                         perror("child close listenfd");
//                         exit(EXIT_FAILURE);
//                     }
//                     if ((close(pipe_fd[0])) == -1) {
//                         perror("child close read pipe");
//                         exit(EXIT_FAILURE);
//                     }

//                     // set up this clients username, etc., then exit
//                     handle_client(client_fd, pipe_fd[1], new_client);

//                     // close pipe before exiting
//                     if ((close(pipe_fd[1])) == -1) {
//                         perror("child close write pipe");
//                         exit(EXIT_FAILURE);
//                     }

//                     exit(EXIT_SUCCESS);

//                 default: // PARENT
//                     if (pipe_fd[0] > max_fd) {
//                         max_fd = pipe_fd[0];
//                     }
//                     FD_SET(pipe_fd[0], &all_fds);

//                     if ((close(pipe_fd[1])) == -1) {
//                         perror("parent close write pipe");
//                         exit(EXIT_FAILURE);
//                     }

//                     // read the Client from the child and add to the list

//                     break;                  // accept more connections
//             }
//         }


//         // check if any clients have anything to say
//         for (Client *cur = clients; cur != NULL; cur = cur->next) {
//             int cfd = cur->client_fd;
//             if (FD_ISSET(cfd, &listen_fds)) {

//                 int cfd_closed = handle_request(cfd, clients);
//                 if (cfd_closed > 0) {
//                     FD_CLR(cfd_closed, &all_fds);
//                     remove_client(cfd_closed, &clients, &all_fds);
//                 } else {
//                     printf("Reading message from [%s]\n", cur->username);
//                 }
//             }
//         }
//     }

//     // should never get here
//     return EXIT_FAILURE;
// }



// /* Handle a read message from the client. Return the client fd <cfd> if the
//  * client has left or there is an error, otherwise return 0
//  */
// int handle_rw(int cfd, Client *client, char *buf) {

//     // check if we read the username
//     if (client->username == NULL) {
//         if (strlen(buf) < MAX_NAME) {
//             client->username = strdup(buf);
//         } else {
//             return cfd;              // kick them out
//         }

//         char *msg = "Are you a TA or a Student (enter T or S)?\r\n";
//         if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
//             perror("write T or S");
//             return cfd;
//         }

//     // check if we read a role
//     } else if (client->role == 'Z') {

//         // we found a TA
//         if (buf[0] == 'T') {
//             char *msg = "Valid commands for TA:\r\n\tstats\n\r\tnext"
//                                         "\r\n\t(or use Ctrl-C to leave)\r\n";
//             if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
//                 perror("write T or S second time");
//                 return cfd;
//             }
//             client->role = 'T';
//             // add to the list of TA's
//             add_ta(&ta_list, client->username);

//         // we found a Student
//         } else if (buf[0] == 'S') {
//             char *msg = "Valid courses: CSC108, CSC148, CSC209\r\n"
//                                     "Which course are you asking about?\r\n";

//             if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
//                 perror("write asking for course");
//                 return cfd;
//             }
//             client->role = 'S';

//         // user entered an invalid role
//         } else {
//             char *msg = "Invalid role. Enter (T or S).\r\n";
//             if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
//                 perror("write T or S second time");
//                 return cfd;
//             }
//             fprintf(stderr, "Client [%s] entered invalid role\n",
//                                                         client->username);
//             memset(buf, '\0', sizeof(buf) - 1);
//         }

//     // check if we read a course code
//     } else if (client->role == 'S' && client->course == NULL) {
//         if ((strcmp("CSC108", buf) || strcmp("CSC148", buf) ||
//                                                     strcmp("CSC209", buf))) {
//             char *msg = "You have been entered into the queue. While you wait, "
//                             "you can use the command stats to see which TAs "
//                                         "are currently serving students.\r\n";
//             if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
//                 perror("write S joined queue");
//                 return cfd;
//             }
//             client->course = strdup(buf);
//             add_student(&stu_list, client->username, client->course,
//                                                         courses, num_courses);
//         } else {
//             fprintf(stderr, "Client [%s] entered an invalid course code\n",
//                                                             client->username);
//             char *msg = "That is not a valid course. Good-bye.\r\n";
//             if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
//                 perror("write S joined queue");
//                 return cfd;
//             }
//         }

//     // check if a TA asked for stats
//     } else if (client->role == 'T' && strcmp("stats", buf)) {
//         char *msg = print_full_queue(stu_list);
//         if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
//             perror("write TA stats");
//             return cfd;
//         }
//         free(msg);

//     // check if a TA asked for the next student
//     } else if (client->role == 'T' && strcmp("next", buf)) {
//         // find the Ta
//         Ta *cur_ta = ta_list;
//         while (cur_ta != NULL) {
//             if (strcmp(client->username, cur_ta->name)) {
//                 break;
//             }
//             cur_ta = cur_ta->next;
//         }

//         // find the Student (client) to disconnect
//         Client *stu_client = clients;
//         while (stu_client != NULL) {
//             if (strcmp(stu_client->username, cur_ta->current_student->name)) {
//                 break;
//             } else {
//                 close(cfd);
//                 exit(EXIT_FAILURE);
//             }
//             stu_client = stu_client->next;
//         }
//         // take the next student if there is one
//         next_overall(cur_ta->name, &ta_list, &stu_list);

//         // this client will be disconnected, freed, etc.
//         char *msg = "You are the next student.......\r\n";
//         if (writen(stu_client->client_fd, msg, strlen(msg)) != strlen(msg)) {
//             perror("write next student");
//             close(cfd);
//             exit(EXIT_FAILURE);
//         }
//         return stu_client->client_fd;

//     // check if a Student asked for stats
//     } else if (client->role == 'S' && client->course != NULL &&
//                                                     strcmp("stats", buf)) {
//         char *msg = print_currently_serving(ta_list);
//         if (writen(cfd, msg, strlen(msg)) != strlen(msg)) {
//             perror("write curr serving");
//             return cfd;
//         }
//         free(msg);

//     // client entered an invalid message
//     } else {
//         ;
//     }

//     // success
//     return 0;
// }



// /*
//  * Search the first n characters of buf for a network newline (\r\n).
//  * Return one plus the index of the '\n' of the first network newline,
//  * or -1 if no network newline is found.
//  * Definitely do not use strchr or other string functions to search here. (Why not?)
//  */
// int find_network_newline(const char *buf, int n) {

//     for (int i = 1; i < n; i++) {
//         if (buf[i - 1] == '\r' && buf[i] == '\n') {
//             return i + 1;
//         }
//     }

//     return -1;
// }



// int handle_request(int cfd, Client *clients) {

//     // find the client of interest
//     Client *cur = clients;
//     while (cur != NULL) {
//         if (cfd == cur->client_fd) {
//             break;                      // found
//         }
//         cur = cur->next;
//     }

//     if (cur == NULL) {
//         fprintf(stderr, "Client [%d] not found\n", cfd);
//         return cfd;
//     }

//     /* Read the message from the client */
//     char buf[INPUT_BUF_SIZE] = {'\0'};
//     int nbytes = 0;                         // # of bytes read
//     int inbuf = 0;                          // # of bytes currently in buf
//     int room = sizeof(buf);                 // # of bytes remaining in buf
//     char *after = buf;
//     while ((nbytes = read(cfd, after, room - 1)) > 0) {
//         inbuf += nbytes;

//         int where;

//         while ((where = find_network_newline(buf, inbuf)) > 0) {
//             buf[where - 2] = '\0';

//             /* we now have a complete message */
//             int close_cfd = handle_rw(cfd, cur, buf);
//             if (close_cfd > 0) {
//                 return close_cfd;
//             }

//             // shift bytes over to start a new string
//             memmove(&buf[0], &buf[where], inbuf - where);
//             inbuf -= where;
//             printf("Client [%s] sent: %s\n", cur->username, buf);
//         }
//         after = &buf[inbuf];
//         room = INPUT_BUF_SIZE - inbuf;
//     }
//     if (nbytes == -1) {
//         perror("read from client");
//         return cfd;
//     } else if (nbytes == 0) {
//         printf("Client [%s] disconnected\n", cur->username);
//         return cfd;
//     }

//     return 0;
// }


// /* Create a new client and add to the back of the list, then ask for a
//  * username.
//  */
// void create_client(int client_fd, Client **clients, fd_set *all_fds) {

//     Client *new_client = malloc(sizeof(Client));
//     if (new_client == NULL) {
//         perror("malloc");
//         exit(EXIT_FAILURE);
//     }
//     new_client->next = NULL;
//     new_client->client_fd = client_fd;
//     new_client->username = NULL;
//     new_client->course = NULL;
//     new_client->role = 'Z';                         // unassigned role

//     // add new_client to the end of the list
//     Client *cur = *clients;
//     Client *prev = NULL;
//     while (cur != NULL) {
//         prev = cur;
//         cur = cur->next;
//     }

//     if (prev != NULL) {             // len(clients) >= 2
//         prev->next = new_client;
//     } else if (cur != NULL) {       // len(clients) == 1
//         cur->next = new_client;
//     } else {                        // len(clients) == 0
//         *clients = new_client;
//     }

//     // send welcome message to client
//     char *greeting = "Welcome to the Help Centre, what is your name?\r\n";
//     if (writen(client_fd, greeting, strlen(greeting)) != strlen(greeting)) {
//         perror("server write");
//         remove_client(client_fd, clients, all_fds);
//     }

//     printf("Client [%d] connected\n", client_fd);
// }


// /* Free a client from the heap and linked list */
// void free_client(Client *client) {
//     free(client->username);
//     free(client->course);
//     free(client);
// }


// /* Remove Client <cfd> from the list of connected Client's */
// void remove_client(int cfd, Client **clients, fd_set *all_fds) {
//     // find the correct client
//     Client *prev = NULL;
//     Client *cur = *clients;
//     while (cur != NULL) {
//         if (cfd == cur->client_fd) {
//             break;                          // found
//         }
//         prev = cur;
//         cur = cur->next;
//     }

//     if (cur == NULL) {
//         fprintf(stderr, "Client [%d] already removed\n", cfd);
//         return;
//     }

//     // close socket
//     FD_CLR(cfd, all_fds);
//     if (close(cfd) == -1) {
//         perror("close client fd");
//         exit(EXIT_FAILURE);
//     }

//     printf("Client [%s] disconnected\n", cur->username);

//     if (cur->role == 'T') {
//         remove_ta(&ta_list, cur->username);
//     } else if (cur->role == 'S') {
//         give_up_waiting(&stu_list, cur->username);
//     }

//     free_client(cur);

//     // update list
//     if (prev != NULL) {
//         prev->next = cur->next;
//     } else {
//         *clients = NULL;
//     }
// }

// /* SIGCHLD handler to reap dead child processes */
// void grim_reaper(int sig) {
//     int saved_errno;             /* Save 'errno' in case changed here */

//     saved_errno = errno;
//     while (waitpid(-1, NULL, WNOHANG) > 0)
//         continue;
//     errno = saved_errno;
// }
