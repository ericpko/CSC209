#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// #define RAND_MAX = 100; // probably better to use modulo

/* Write random integers (in binary) to a file with the name given by the
 * command-line argument. This program creates a data file for use by the
 * time_reads program.
 */

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: write_test_file filename\n");
        exit(1);
    }

    FILE *fp;
    if ((fp = fopen(argv[1], "wb")) == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int r;
    for (int i = 0; i < 100; i++) {
        r = random() % 100; // random number between 0 and 99
        if (fwrite(&r, sizeof(int), 1, fp) != 1) {
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
    }

    fclose(fp);
    return EXIT_SUCCESS;
}


/* To see the binary output: make output
 *
 * od -i output         // most correct since we have written ints
 * od -vtu1 output      // not as clear
 * od -s output         // shows every 2-bytes
 */
