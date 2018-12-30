#include <stdio.h>
#include <stdlib.h>

#define MAX_STR_LEN 11

int main(void) {
    char phone[MAX_STR_LEN];
    int num;

    // printf("Enter a ten character string followed by an integer: ");
    scanf("%10s", phone);                  // scanf("%10s%d", phone, &num);
    scanf("%d", &num);

    if (num == -1) {
        printf("%s\n", phone);
    } else if (num >= 0 && num <= 9) {
        printf("%c\n", phone[num]);
    } else {
        printf("ERROR\n");
        exit(1);
    }
    return 0;
}
