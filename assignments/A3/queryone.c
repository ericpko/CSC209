#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "freq_list.h"
#include "worker.h"

/* A program to model calling run_worker and to test it. Notice that run_worker
 * produces binary output, so the output from this program to STDOUT will
 * not be human readable.  You will need to work out how to save it and view
 * it (or process it) so that you can confirm that your run_worker
 * is working properly.
 */
int main(int argc, char **argv) {
    char ch;
    char path[PATHLENGTH];
    char *startdir = ".";

    /* this models using getopt to process command-line flags and arguments */
    while ((ch = getopt(argc, argv, "d:")) != -1) {
        switch (ch) {
        case 'd':
            startdir = optarg;
            break;
        default:
            fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME]\n");
            exit(EXIT_FAILURE);
        }
    }

    // Open the directory provided by the user (or current working directory)
    DIR *dirp;
    if ((dirp = opendir(startdir)) == NULL) {
        perror("opendir");
        exit(1);
    }

    /* For each entry in the directory, eliminate . and .., and check
     * to make sure that the entry is a directory, then call run_worker
     * to process the index file contained in the directory.
     * Note that this implementation of the query engine iterates
     * sequentially through the directories, and will expect to read
     * a word from standard input for each index it checks.
     */
    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 ||
            strcmp(dp->d_name, "..") == 0 ||
            strcmp(dp->d_name, ".svn") == 0 ||
            strcmp(dp->d_name, ".git") == 0) {
                continue;
        }

        strncpy(path, startdir, PATHLENGTH);
        strncat(path, "/", PATHLENGTH - strlen(path));
        strncat(path, dp->d_name, PATHLENGTH - strlen(path));
        path[PATHLENGTH - 1] = '\0';

        struct stat sbuf;
        if (stat(path, &sbuf) == -1) {
            // This should only fail if we got the path wrong
            // or we don't have permissions on this entry.
            perror("stat");
            exit(EXIT_FAILURE);
        }

        // Only call run_worker if it is a directory
        // Otherwise ignore it.
        if (!(S_ISDIR(sbuf.st_mode))) {
            continue;
        }

        // only gets here if we found a directory
        run_worker(path, STDIN_FILENO, STDOUT_FILENO);
    }

    if (closedir(dirp) < 0) {
        perror("closedir");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}



// good attempt *** FOR TESTING ONLY ***

// /* A program to model calling run_worker and to test it. Notice that run_worker
//  * produces binary output, so the output from this program to STDOUT will
//  * not be human readable.  You will need to work out how to save it and view
//  * it (or process it) so that you can confirm that your run_worker
//  * is working properly.
//  */
// int main(int argc, char **argv) {
//     char ch;
//     char path[PATHLENGTH];
//     char *startdir = ".";

//     // initialize FreqRecord array
//     FreqRecord fr_arr[2];
//     FreqRecord fr;
//     fr.freq = 0;
//     fr.filename[0] = '\0';
//     fr_arr[1] = fr;

//     /* this models using getopt to process command-line flags and arguments */
//     while ((ch = getopt(argc, argv, "d:")) != -1) {
//         switch (ch) {
//         case 'd':
//             startdir = optarg;
//             break;
//         default:
//             fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME]\n");
//             exit(1);
//         }
//     }

//     // Open the directory provided by the user (or current working directory)
//     DIR *dirp;
//     if ((dirp = opendir(startdir)) == NULL) {
//         perror("opendir");
//         exit(1);
//     }

//     /* For each entry in the directory, eliminate . and .., and check
//      * to make sure that the entry is a directory, then call run_worker
//      * to process the index file contained in the directory.
//      * Note that this implementation of the query engine iterates
//      * sequentially through the directories, and will expect to read
//      * a word from standard input for each index it checks.
//      */
//     // find path
//     int fd[2][2], r, n;
//     char word[MAXWORD];
//     struct dirent *dp;
//     while ((dp = readdir(dirp)) != NULL) {
//         if (strcmp(dp->d_name, ".") == 0 ||
//             strcmp(dp->d_name, "..") == 0 ||
//             strcmp(dp->d_name, ".svn") == 0 ||
//             strcmp(dp->d_name, ".git") == 0) {
//                 continue;
//         }

//         strncpy(path, startdir, PATHLENGTH);
//         strncat(path, "/", PATHLENGTH - strlen(path));
//         strncat(path, dp->d_name, PATHLENGTH - strlen(path));
//         path[PATHLENGTH - 1] = '\0';

//         struct stat sbuf;
//         if (stat(path, &sbuf) == -1) {
//             // This should only fail if we got the path wrong
//             // or we don't have permissions on this entry.
//             perror("stat");
//             exit(1);
//         }

//         // Only call run_worker if it is a directory
//         // Otherwise ignore it.
//         if (!(S_ISDIR(sbuf.st_mode))) {
//             continue;
//         }
//         // only gets here if we found a directory

//         // create two pipes
//         if (pipe(fd[0]) == -1) {
//             perror("pipe");
//             exit(EXIT_FAILURE);
//         }
//         if (pipe(fd[1]) == -1) {
//             perror("pipe");
//             exit(EXIT_FAILURE);
//         }

//         // fork
//         if ((r = fork()) > 0) { // parent process
//             // close unused pipes
//             if (close(fd[0][0]) == -1) {
//                 perror("close");
//                 exit(EXIT_FAILURE);
//             }
//             if (close(fd[1][1]) == -1) {
//                 perror("close");
//                 exit(EXIT_FAILURE);
//             }
//             // read a word from stdin
//             if (fgets(word, MAXWORD, stdin) != NULL) {
//                 word[MAXWORD - 1] = '\0';
//             } else {
//                 break;
//             }
//             // write word to pipe 1
//             if (write(fd[0][1], word, MAXWORD - 1) == -1) {
//                 perror("write");
//                 exit(EXIT_FAILURE);
//             }
//             if (close(fd[0][1]) == -1) {
//                 perror("close");
//                 exit(EXIT_FAILURE);
//             }

//             // read FreqRecord output from pipe 2
//             while ((n = read(fd[1][0], &fr, sizeof(FreqRecord))) > 0) {
//                 printf("FreqRecord:\n");
//                 fr_arr[0] = fr;
//                 print_freq_records(fr_arr);
//             }
//             if (n == -1) {
//                 perror("read");
//                 exit(EXIT_FAILURE);
//             }
//             // finished reading
//             if (close(fd[1][0]) == -1) {
//                 perror("close");
//                 exit(EXIT_FAILURE);
//             }

//             // wait for child to exit
//             pid_t child_pid;
//             int status, child_ret_id;
//             if ((child_pid = wait(&status)) == -1) {
//                 perror("wait");
//                 exit(EXIT_FAILURE);
//             } else if (WIFEXITED(status)) {
//                 child_ret_id = WEXITSTATUS(status);
//             } else {
//                 fprintf(stderr, "should not be here");
//                 exit(EXIT_FAILURE);
//             }

//             if (child_ret_id == -1) {
//                 fprintf(stderr, "something went terribly wrong.");
//                 exit(EXIT_FAILURE);
//             }

//         } else if (r == 0) { // child process
//             // close unused pipes
//             if (close(fd[0][1]) == -1) {
//                 perror("close");
//                 exit(EXIT_FAILURE);
//             }

//             // run worker
//             run_worker(path, fd[0][0], fd[1][1]);

//             if (close(fd[0][0]) == -1) {
//                 perror("close");
//                 exit(EXIT_FAILURE);
//             }
//             if (close(fd[1][1]) == -1) {
//                 perror("close");
//                 exit(EXIT_FAILURE);
//             }

//             exit(EXIT_SUCCESS);

//         } else { // r == -1
//             perror("fork");
//             exit(EXIT_FAILURE);
//         }
//     }

//     if (closedir(dirp) < 0) {
//         perror("closedir");
//         exit(EXIT_FAILURE);
//     }

//     return EXIT_SUCCESS;
// }
