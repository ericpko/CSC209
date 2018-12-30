#include <stdio.h>
#include <stdlib.h>

#define MAX_STR_LEN 11

int main(void) {
    char phone[MAX_STR_LEN];
    int num;

    // printf("Enter a ten character string: ");
    scanf("%10s", phone);

    // printf("Enter an integer or EOF: ");
    while(scanf("%d", &num) != EOF) {
        if (num == -1) {
            printf("%s\n", phone);
        } else if (num >= 0 && num <= 9) {
            printf("%c\n", phone[num]);
        } else {
            printf("ERROR\n");
            exit(1);
        }
        // printf("Enter an integer or EOF: ");
    }

    printf("\n");

    return 0;
}
