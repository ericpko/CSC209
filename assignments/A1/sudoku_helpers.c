#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

int check_group(int **elements, int n);
int check_regular_sudoku(int **puzzle);


/* Each of the n elements of array elements, is the address of an
 * array of n integers.
 * Return 0 if every integer is between 1 and n^2 and all
 * n^2 integers are unique, otherwise return 1.
 */
int check_group(int **elements, int n) {
    bool digit_seen[n * n + 1];
    for (int i = 0; i < (n * n) + 1; i++) {
        digit_seen[i] = false;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            // check conditions
            if (elements[i][j] < 1 || elements[i][j] > n * n || digit_seen[elements[i][j]] == true) {
                return 1;
            } else {
                digit_seen[elements[i][j]] = true;
            }
        }
    }

    return 0;
}

/* puzzle is a 9x9 sudoku, represented as a 1D array of 9 pointers
 * each of which points to a 1D array of 9 integers.
 * Return 0 if puzzle is a valid sudoku and 1 otherwise. You must
 * only use check_group to determine this by calling it on
 * each row, each column and each of the 9 inner 3x3 squares
 */
int check_regular_sudoku(int **puzzle) {
    // allocate memory on the heap
    // int **elements = malloc(3 * sizeof(int*));
    // elements[0] = malloc(3 * sizeof(int));
    // elements[1] = malloc(3 * sizeof(int));
    // elements[2] = malloc(3 * sizeof(int));
    int *elements[3];
    int element0[3];
    int element1[3];
    int element2[3];

    // create the tranpose of puzzle
    int transpose[9][9];
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col ++) {
            transpose[col][row] = puzzle[row][col];
        }
    }

    // check the rows and columns of puzzle
    for (int row = 0; row < 9; row++) {
        // check the rows
        for (int i = 0, j = 3, k = 6; i < 3; i++, j++, k++) {
            element0[i] = puzzle[row][i];
            element1[i] = puzzle[row][j];
            element2[i] = puzzle[row][k];
        }
        elements[0] = element0, elements[1] = element1, elements[2] = element2;
        if (check_group(elements, 3) == 1) {
            return 1;
        }
        // check the columns
        for (int i = 0, j = 3, k = 6; i < 3; i++, j++, k++) {
            element0[i] = transpose[row][i];
            element1[i] = transpose[row][j];
            element2[i] = transpose[row][k];
        }
        elements[0] = element0, elements[1] = element1, elements[2] = element2;
        if (check_group(elements, 3) == 1) {
            return 1;
        }
    }

    // check the 3x3 subsquares of puzzle
    int square[9];
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            square[j] = puzzle[(i / 3) * 3 + (j / 3)][((i * 3) % 9) + (j % 3)];
        }
        // build elements
        for (int x = 0, y = 3, z = 6; x < 3; x++, y++, z++) {
            element0[x] = square[x];
            element1[x] = square[y];
            element2[x] = square[z];
        }
        elements[0] = element0, elements[1] = element1, elements[2] = element2;
        // check subsquares
        if (check_group(elements, 3) == 1) {
            return 1;
        }
    }

    return 0;       // sudoku is valid
}
