#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void get_mp_with_ppid(char **mp_user, int *max_seen, int given_ppid);
void get_mp_user(char **mp_user, int *max_seen);

int main(int argc, char const **argv) {

    // check the argument count
    bool arg_given = false;
    int given_ppid;

    if (argc > 2) {
        fprintf(stderr, "USAGE: most_processes [ppid]\n");
        exit(1);
    } else if (argc == 2) {
        given_ppid = strtol(argv[1], NULL, 10);    // convert str to int => int given_ppid = atoi(argv[1]);
        arg_given = true;
    }

    // initialize variables
    int max_seen = 0;
    char *mp_user;              // how many will this be able to hold?

    if (arg_given == true) {
        get_mp_with_ppid(&mp_user, &max_seen, given_ppid);
        if (max_seen) {        // only print if max_seen != 0
            printf("%s %d\n", mp_user, max_seen);
        }
    } else {
        get_mp_user(&mp_user, &max_seen);
        if (max_seen) {
            printf("%s %d\n", mp_user, max_seen);
        }
    }

    return 0;
}

void get_mp_with_ppid(char **mp_user, int *max_seen, int given_ppid) {
    int curr_seen = 0;
    long pid, ppid;
    char prev_uid[32], curr_uid[32], line[1024];
    bool first_iter = true;

    while (fgets(line, sizeof(line), stdin)) {
        sscanf(line, "%s %ld %ld", curr_uid, &pid, &ppid);

        if (first_iter) {
            strcpy(prev_uid, curr_uid);
            first_iter = false;
        }

        if (strcmp(prev_uid, curr_uid) == 0 && ppid == given_ppid) {
            curr_seen += 1;
        } else if (ppid == given_ppid) {
            curr_seen = 1;
        } else if (strcmp(prev_uid, curr_uid) != 0) {
            curr_seen = 0;
        }

        strcpy(prev_uid, curr_uid);         // update prev

        if (curr_seen > *max_seen) {
            *max_seen = curr_seen;
            strcpy(*mp_user, curr_uid);
        }

    }
}


void get_mp_user(char **mp_user, int *max_seen) {
    int curr_seen = 0;
    char prev_uid[32], curr_uid[32], line[1024];
    bool first_iter = true;

    while (fgets(line, sizeof(line), stdin)) {
        sscanf(line, "%s", curr_uid);

        if (first_iter) {
            strcpy(prev_uid, curr_uid);
            first_iter = false;
        }

        if (strcmp(prev_uid, curr_uid) == 0) {
            curr_seen += 1;
        } else {
            curr_seen = 1;
        }

        strcpy(prev_uid, curr_uid);         // update prev

        if (curr_seen > *max_seen) {
            *max_seen = curr_seen;
            strcpy(*mp_user, curr_uid);
        }

    }
}


            // this works
    // while (fgets(line, sizeof(line), stdin)) {
    //     sscanf(line, "%s %d %d", curr_uid, &pid, &ppid);
    //     printf("UID: %s, PID: %d, PPID: %d\n\n", curr_uid, pid, ppid);

    // }
