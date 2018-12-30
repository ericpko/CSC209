/* The purpose of this program is to practice writing signal handling
 * functions and observing the behaviour of signals.
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

/* Message to print in the signal handling function. */
#define MESSAGE "%ld reads were done in %ld seconds.\n"

/* Global variables to store number of read operations and seconds elapsed.
 */
long num_reads, seconds;


/* Handles a signal
 */
void timer_handler(int sig) {
    // printf("Signal %d was caught.\n", sig);
    printf(MESSAGE, num_reads, seconds);
    exit(EXIT_SUCCESS);
}


/* The first command-line argument is the number of seconds to set a
 * timer to run.
 * The second argument is the name of a binary file containing 100 ints.
 * Assume both of these arguments are correct.
 */
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: time_reads s filename\n");
        exit(EXIT_FAILURE);
    }
    seconds = strtol(argv[1], NULL, 10);

    FILE *fp;
    if ((fp = fopen(argv[2], "rb")) == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // create a new signal action
    struct sigaction sa;
    sa.sa_handler = timer_handler;
    sa.sa_flags = 0;
    if (sigemptyset(&sa.sa_mask) == -1) {
        perror("sigemptyset");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGPROF, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // set an alarm
    struct itimerval timer;
    timer.it_interval.tv_sec = 0;             // only send the signal once
    timer.it_interval.tv_usec = 0;            // microseconds
    timer.it_value.tv_sec = seconds;
    timer.it_value.tv_usec = 0;

    // start the timer
    if (setitimer(ITIMER_PROF, &timer, NULL) == -1) {
        perror("setitimer");
        exit(EXIT_FAILURE);
    }

    /* In an infinite loop, read an int from a random location in the file,
    * and print it to stderr.
    */
    int offset, num;
    for (;;) {
        offset = random() % 100;
        if (fseek(fp, offset * sizeof(int), SEEK_SET) == -1) {
            perror("fseek");
            exit(EXIT_FAILURE);
        }
        if (fread(&num, sizeof(int), 1, fp) != 1) {
            perror("fread");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "%d\n", num);
        num_reads++;
    }

    return EXIT_FAILURE; // something is wrong if we ever get here!
}



// Using Real-time:

// int main(int argc, char **argv) {
//     if (argc != 3) {
//         fprintf(stderr, "Usage: time_reads s filename\n");
//         exit(EXIT_FAILURE);
//     }
//     seconds = strtol(argv[1], NULL, 10);

//     FILE *fp;
//     if ((fp = fopen(argv[2], "rb")) == NULL) {
//         perror("fopen");
//         exit(EXIT_FAILURE);
//     }

//     // create a new signal action
//     struct sigaction sa;
//     sa.sa_handler = timer_handler;
//     sa.sa_flags = 0;
//     if (sigemptyset(&sa.sa_mask) == -1) {
//         perror("sigemptyset");
//         exit(EXIT_FAILURE);
//     }
//     if (sigaction(SIGALRM, &sa, NULL) == -1) {
//         perror("sigaction");
//         exit(EXIT_FAILURE);
//     }

//     // set an alarm
//     struct itimerval timer;
//     timer.it_interval.tv_sec = 0;             // only send the signal once
//     timer.it_interval.tv_usec = 0;            // microseconds
//     timer.it_value.tv_sec = seconds;
//     timer.it_value.tv_usec = 0;

//     // start the timer
//     if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
//         perror("setitimer");
//         exit(EXIT_FAILURE);
//     }

//     /* In an infinite loop, read an int from a random location in the file,
//     * and print it to stderr.
//     */
//     int offset, num;
//     for (;;) {
//         offset = random() % 100;
//         if (fseek(fp, offset * sizeof(int), SEEK_SET) == -1) {
//             perror("fseek");
//             exit(EXIT_FAILURE);
//         }
//         if (fread(&num, sizeof(int), 1, fp) != 1) {
//             perror("fread");
//             exit(EXIT_FAILURE);
//         }
//         fprintf(stderr, "%d\n", num);
//         num_reads++;
//     }

//     return EXIT_FAILURE; // something is wrong if we ever get here!
// }
