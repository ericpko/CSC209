#include <stdio.h>

#define SIZE 4
#define OVERFLOW 4  // set to 5 to test gdb

int main() {
    int index = 0;
    int i;
    int before[SIZE] = {10, 10, 10, 10};
    int a[SIZE] = {0, 0, 0, 0};
    int after[SIZE] = {10, 10, 10, 10};

    printf("Address of the variables:\n");
    for (index = 0; index < SIZE; index++) {
        printf("%lx -> &after[%d]\n", (unsigned long) &after[index], index);
    }
    for (index = 0; index < SIZE; index++) {
        printf("%lx -> &a[%d]\n", (unsigned long) &a[index], index);
    }
    for (index = 0; index < SIZE; index++) {
        printf("%lx -> &before[%d]\n", (unsigned long) &before[index], index);
    }
    printf("%lx -> &i\n", (unsigned long)&i);
    printf("%lx -> &index\n", (unsigned long)&index);
    printf("\n");


    printf("Initial values:\n");
    printf("i = %d\n", i);
    printf("before = {%d, %d, %d, %d}\n", before[0], before[1], before[2], before[3]);
    printf("a = {%d, %d, %d, %d}\n", a[0], a[1], a[2], a[3]);
    printf("after = {%d, %d, %d, %d}\n", after[0], after[1], after[2], after[3]);
    printf("\n");


    for (i = 0; i < OVERFLOW; i++) {
        a[i] = i * 10;
        printf("i = %d\n", i);
        printf("before = {%d, %d, %d, %d}\n", before[0], before[1], before[2], before[3]);
        printf("a = {%d, %d, %d, %d}\n", a[0], a[1], a[2], a[3]);
        printf("after = {%d, %d, %d, %d}\n", after[0], after[1], after[2], after[3]);
    }

    return 0;
}

// gdb commands:

// gdb executable	            start gdb on this executable
// list [n]	                    list some of the code starting from line n or from the end of last call to list
// break [n or fun_name] or b	set a breakpoint either at line n or at the beginning of the function fun_name
// run [args]	                begin execution with these command-line arguments
// next	or n                    execute one line
// print                        variable or expression	print the value once
// display                      variable or expression	print the value after every gdb command
// continue	or c                execute up to the next breakpoint
// clear [linenum]              remove breakpoint
// jump [line_num]              go to that line number
// quit	                        bye-bye!


// eg)
// in gdb type: print &(a[4]) to see mem address
