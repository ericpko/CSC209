#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int n;

    if (argc != 2) {
        fprintf(stderr, "Usage: forkloop <iterations>\n");
        exit(1);
    }

    int iterations = strtol(argv[1], NULL, 10);

    // cases iterations = 0, 1
    if (iterations == 0) {
        printf("%d\n", getpid());
        exit(EXIT_SUCCESS);
    } else if (iterations == 1) {
        if ((n = fork()) < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (n > 0) {
            // print parent and child, then exit
            printf("%d -> %d\n", getpid(), n);
            exit(EXIT_SUCCESS);
        } else {
            // exit the child as well
            exit(EXIT_SUCCESS);
        }
    }

    // iterations at least >= 2
    for (int i = 0; i < iterations; i++) {

        if ((n = fork()) < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (n > 0 && i == 0) {
            // print the child pid, then exit
            printf("%d -> %d -> ", getpid(), n);
            exit(EXIT_SUCCESS);
        } else if (n > 0 && i < iterations - 1) {
            printf("%d -> ", n);
            exit(EXIT_SUCCESS);
        } else if (n > 0 && i == iterations - 1) {
            printf("%d\n", n);
            exit(EXIT_SUCCESS);
        }

    }
    return 0;
}
