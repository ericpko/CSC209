#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
    This program has two arguments: the first is a greeting message, and the
    second is a name.

    The message is an impersonal greeting, such as "Hi" or "Good morning".
    name is set to refer to a string holding a friend's name, such as
    "Emmanuel" or "Xiao".

    First copy the first argument to the array greeting. (Make sure it is
    properly null-terminated.)

    Write code to personalize the greeting string by appending a space and
    then the string pointed to by name.
    So, in the first example, greeting should be set to "Hi Emmanuel", and
    in the second it should be "Good morning Xiao".

    If there is not enough space in greeting, the resulting greeting should be
    truncated, but still needs to hold a proper string with a null terminator.
    For example, "Good morning" and "Emmanuel" should result in greeting
    having the value "Good morning Emmanu" and "Top of the morning to you" and
    "Patrick" should result in greeting having the value "Top of the morning ".

    Do not make changes to the code we have provided other than to add your
    code where indicated.
*/

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: greeting message name\n");
        exit(1);
    }
    char greeting[20];
    char *name = argv[2];

    // copy greeting message
    // note: add sizeof(...) - 1, so that strncpy will append \0
    strncpy(greeting, argv[1], sizeof(greeting) - 1); // won't add null terminator if argv[1] is too long
    greeting[19] = '\0';           // IMPORTANT!!!! sets proper length (don't need though)

    // add space to greeting
    int greet_len = strlen(greeting);
    if (greet_len < 19) {
        greeting[greet_len] = ' ';
    }

    // concatenate name to greeting
    strncat(greeting, name, sizeof(greeting) - strlen(greeting) - 1);

    // set null terminator to be safe
    greeting[19] = '\0'; // DO NOT need this since strncat always adds null terminator

    printf("%s\n", greeting);
    return 0;
}

//              *** Alternative solution ***
// int main(int argc, char **argv) {
//     if (argc != 3) {
//         fprintf(stderr, "Usage: greeting message name\n");
//         exit(1);
//     }
//     char greeting[20];
//     char *name = argv[2];

//     // copy the first arg to greeting
//     strncpy(greeting, argv[1], 19);
//     greeting[19] = '\0';

//     int greet_len = strlen(greeting);

//     // add a space
//     if (greet_len < 19) {
//         greeting[greet_len] = ' ';
//     }

//     // modify greeting message
//     for (int i = greet_len + 1; *name != '\0' && i < 19; i++, name++) {
//         greeting[i] = *name;
//     }


//     printf("%s\n", greeting);
//     return 0;
// }


