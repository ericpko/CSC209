#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAX_PASSWORD 10

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
    char user_id[MAXLINE];
    char password[MAXLINE];
    int child_ret_id;

    if (fgets(user_id, MAXLINE, stdin) == NULL) {
        perror("fgets");
        exit(EXIT_FAILURE);
    }
    if (fgets(password, MAXLINE, stdin) == NULL) {
        perror("fgets");
        exit(EXIT_FAILURE);
    }

    // set up a pipe
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // split program
    int r;
    if ((r = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);

    } else if (r > 0) { // parent process

        // parent won't be reading from the pipe, so close
        if (close(fd[0]) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }

        // write the user_id and password to the pipe
        if (write(fd[1], user_id, MAX_PASSWORD) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        if (write(fd[1], password, MAX_PASSWORD) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        // close writing to the pipe so child knows it can stop reading
        if (close(fd[1]) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }

    } else { // child process

        // child won't be writing to the pipe
        if (close(fd[1]) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }

        // redirect stdin to read from pipe
        if (dup2(fd[0], STDIN_FILENO) == -1) { // or fileno(stdin)
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        // we don't need to read from this file descriptor anymore
        // because we still have stdin reading from the pipe.
        if (close(fd[0]) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }

        // execute validate
        execl("./validate", "validate", NULL);

        // should not get here unless execl fails
        perror("exec");
        exit(EXIT_FAILURE);
    }

    // wait for the child to terminate
    pid_t child_pid;
    int status;
    if ((child_pid = wait(&status)) == -1) {
        perror("wait");
        exit(EXIT_FAILURE);
    } else if (WIFEXITED(status)) {
        child_ret_id = WEXITSTATUS(status);
    } else {
        fprintf(stderr, "should not be here");
        exit(EXIT_FAILURE);
    }

    // print child's return value
    if (child_ret_id == 0) {
        printf(SUCCESS);
    } else if (child_ret_id == 2) {
        printf(INVALID);
    } else if (child_ret_id == 3) {
        printf(NO_USER);
    } else {
        fprintf(stderr, "Child returned an error\n");
    }

    return 0;
}
