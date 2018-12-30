#include <stdlib.h>
#include <stdio.h>


void fib(int **fib_sequence, int n);

int main(int argc, char **argv) {
    /* do not change this main function */
    int count = strtol(argv[1], NULL, 10);
    int *fib_sequence;

    fib(&fib_sequence, count);
    for (int i = 0; i < count; i++) {
        printf("%d ", fib_sequence[i]);
    }
    free(fib_sequence);
    return 0;
}

/*
 * Define a function void fib(...) below. This function takes parameter n
 * and generates the first n values in the Fibonacci sequence.  Recall that this
 * sequence is defined as:
 *         0, 1, 1, 2, 3, 5, ... , fib[n] = fib[n-2] + fib[n-1], ...
 * The values should be stored in a dynamically-allocated array composed of
 * exactly the correct number of integers. The values should be returned
 * through a pointer parameter passed in as the first argument.
 *
 * See the main function for an example call to fib.
 * Pay attention to the expected type of fib's parameters.
 */
void fib(int **fib_sequence, int n) {
    *fib_sequence = malloc((n+2) * sizeof(int));
    fib_sequence[0][0] = 0;
    (*fib_sequence)[1] = 1;

    for (int i = 2; i < n; i++) {
        (*fib_sequence)[i] = (*fib_sequence)[i - 2] + (*fib_sequence)[i - 1];
    }
}