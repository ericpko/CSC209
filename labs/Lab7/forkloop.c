#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int i;
    int iterations;

    if (argc != 2) {
        fprintf(stderr, "Usage: forkloop <iterations>\n");
        exit(1);
    }

    iterations = strtol(argv[1], NULL, 10);

    for (i = 0; i < iterations; i++) {
        int n = fork();
        if (n < 0) {
            perror("fork");
            exit(1);
        }
        printf("ppid = %d, pid = %d, i = %d\n", getppid(), getpid(), i);
    }

    return 0;
}

// Question 3: How many processes are created, including the original parent, when forkloop is called with 2, 3, and 4 as arguments? n arguments?
// When forkloop is called with 2 as an argv, then 4 processes created.
// When forkloop is called with 3 as an argv, then 8 processes created.
// When forkloop is called with 4 as an argv, then 16 processes created.
// When forkloop is called with n as an argv, then 2^n processes created.

// Question 4:
// My output
// 27088 -> 29138
// 29138 -> 29139
// 29138 -> 29142
// 29138 -> 29140
// 29139 -> 29143
// 29139 -> 29141
// 29140 -> 29144
// 29141 -> 29145
