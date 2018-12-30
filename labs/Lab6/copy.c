#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Write the copy function to perform exactly as strncpy does, with one
   exception: your copy function will guarantee that dest is always
   null-terminated.
   You shoud read the man page to learn how strncpy works.

  NOTE: You must write this function without using any string functions.
  The only function that should depend on string.h is memset.
 */

char *copy(char *dest, const char *src, int capacity) {

    // or memset and remove dest[cap - 1] = '\0';
    // memset(dest, '\0', capacity * sizeof(char));


    for (int i = 0; i < capacity - 1; i++) {
        dest[i] = src[i];
    }

    dest[capacity - 1] = '\0';


    // or
    // for (int i = 0; i < capacity; i++) {
    //     memset(dest + i, src[i], sizeof(char));
    // }


    // copies all characters (NOT what we want, but interesting)
    // Note: ++ takes priority over dereference *
    // while ((*dest++ = *src++)) {
    //     NULL;
    // }

    return dest;
}


int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: copy size src\n");
        exit(1);
    }
    int size = strtol(argv[1], NULL, 10);
    if (size <= 0) {
        fprintf(stderr, "Usage: size >= 0\n");
        exit(1);
    }
    char *src = argv[2];

    char dest[size];
    memset(dest, 'x', size - 1);

    copy(dest, src, size);

    printf("%s\n", dest);
    return 0;
}
