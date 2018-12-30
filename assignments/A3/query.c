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


/* Append <frp> to the end of the <frp_arr>, then sort the FreqRecord
 * array. Set the last index in <frp_arr> to an empty FreqRecord before
 * returning.
 */
void add_freq_record(FreqRecord *frp, FreqRecord *frp_arr) {
    // add the <frp> to the last index of the frp_arr
    frp_arr[MAXRECORDS - 1] = *frp;

    // sort the FreqRecord array
    qsort(frp_arr, MAXRECORDS, sizeof(FreqRecord), compare);

    // make sure the last index has an empty FreqRecord
    FreqRecord fr;
    fr.freq = 0;
    fr.filename[0] = '\0';
    frp_arr[MAXRECORDS - 1] = fr;
}


/* Initialize the FreqRecord array <freq_arr> by setting all elements
 * to an empty FreqRecord.
 */
void init_freq_record(FreqRecord *freq_arr) {
    FreqRecord fr;
    fr.freq = 0;
    fr.filename[0] = '\0';
    for (int i = 0; i < MAXRECORDS; i++) {
        freq_arr[i] = fr;
    }
}


/* Search each subdirectory of the given directory (or cwd) for the given
 * word from stdin. For each word, create a FreqRecord array using parallel
 * processes and print the FreqRecord array from the highest frequency element
 * to the lowest.
 */
int main(int argc, char **argv) {
    char opt;
    char path[PATHLENGTH], word[MAXWORD];
    char *startdir = ".";
    FreqRecord freq_arr[MAXRECORDS];
    init_freq_record(freq_arr);
    FreqRecord fr;


    /* this models using getopt to process command-line flags and arguments */
    while ((opt = getopt(argc, argv, "d:")) != -1) {
        switch (opt) {
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
        exit(EXIT_FAILURE);
    }
    // find the number of subdirectories
    struct stat sbuf;
    struct dirent *dp;
    int subdir = 0;
    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 ||
            strcmp(dp->d_name, "..") == 0 ||
            strcmp(dp->d_name, ".svn") == 0 ||
            strcmp(dp->d_name, ".git") == 0) {
                continue;
        }
        // get the path of this potential subdirectory
        strncpy(path, startdir, PATHLENGTH);
        strncat(path, "/", PATHLENGTH - strlen(path) - 1);
        strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);
        path[PATHLENGTH - 1] = '\0';

        if (stat(path, &sbuf) == -1) {
            perror("stat");
            exit(EXIT_FAILURE);
        }
        if (S_ISDIR(sbuf.st_mode)) {
            subdir++;
        }
    }
    // exit if there are no subdirectories
    if (subdir == 0) {
        fprintf(stderr, "No subdirectories found.\n");
        exit(EXIT_FAILURE);
    }

    int fd[subdir + subdir][2], r, n;
    // create one process for each subdirectory of the target directory
    // read a word from stdin
    while (fscanf(stdin, "%31s", word) == 1) {
        word[MAXWORD - 1] = '\0';

        int i = 0, j = subdir;
        rewinddir(dirp);
        while ((dp = readdir(dirp)) != NULL) {
            if (strcmp(dp->d_name, ".") == 0 ||
                strcmp(dp->d_name, "..") == 0 ||
                strcmp(dp->d_name, ".svn") == 0 ||
                strcmp(dp->d_name, ".git") == 0) {
                    continue;
            }
            // get the path of this potential subdirectory
            strncpy(path, startdir, PATHLENGTH);
            strncat(path, "/", PATHLENGTH - strlen(path) - 1);
            strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);
            path[PATHLENGTH - 1] = '\0';

            if (stat(path, &sbuf) == -1) {
                perror("stat");
                exit(EXIT_FAILURE);
            }
            if (!(S_ISDIR(sbuf.st_mode))) { // if it's not a directory, skip
                continue;
            }
            // we only get here if we found a directory
            // set up the two pipes
            if (pipe(fd[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
            if (pipe(fd[j]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
            // create the child processes for each subdir
            if ((r = fork()) == -1) {
                perror("fork");
                exit(EXIT_FAILURE);

            } else if (r > 0) { // parent (master) process
                // The parent is not reading from ith pipe or writing to the
                // jth pipe. Close these
                if (close(fd[i][0]) == -1) {
                    perror("close");
                    exit(EXIT_FAILURE);
                }
                if (close(fd[j][1]) == -1) {
                    perror("close");
                    exit(EXIT_FAILURE);
                }
                // write the word to the ith pipe
                if (write(fd[i][1], word, MAXWORD - 1) == -1) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                // finished writing to this pipe, so close
                if (close(fd[i][1]) == -1) {
                    perror("close");
                    exit(EXIT_FAILURE);
                }
                // read FreqRecord's from the jth pipe in until it's closed
                while ((n = read(fd[j][0], &fr, sizeof(FreqRecord))) > 0) {
                    add_freq_record(&fr, freq_arr);
                }
                if (n == -1) {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                // finished reading
                if (close(fd[j][0]) == -1) {
                    perror("close");
                    exit(EXIT_FAILURE);
                }

                // wait for the child process to exit
                pid_t child_pid;
                int status, child_ret_id;
                if ((child_pid = wait(&status)) == -1) {
                    perror("wait");
                    exit(EXIT_FAILURE);
                } else if (WIFEXITED(status)) {
                    child_ret_id = WEXITSTATUS(status);
                } else {
                    fprintf(stderr, "should not be here");
                    exit(EXIT_FAILURE);
                }

                if (child_ret_id == 1) {
                    fprintf(stderr, "Missing index and/or filenames file.\n");
                    exit(EXIT_FAILURE);
                }

            } else {
                // Child is not writing to pipe i or reading from pipe j
                if (close(fd[i][1]) == -1) {
                    perror("close");
                    exit(EXIT_FAILURE);
                }
                if (close(fd[j][0]) == -1) {
                    perror("close");
                    exit(EXIT_FAILURE);
                }

                // work it
                run_worker(path, fd[i][0], fd[j][1]);

                // finished with pipe, so close
                if (close(fd[i][0]) == -1) {
                    perror("close");
                    exit(EXIT_FAILURE);
                }
                if (close(fd[j][1]) == -1) {
                    perror("close");
                    exit(EXIT_FAILURE);
                }

                // terminate the child after it's done its job
                exit(EXIT_SUCCESS);
            }
            i++;
            j++;
        }
        // sort the FreqRecord array before printing
        qsort(freq_arr, MAXRECORDS, sizeof(FreqRecord), compare);
        print_freq_records(freq_arr);

        // reset the freq_arr
        init_freq_record(freq_arr);

    }
    // close the given directory (or cwd)
    if (closedir(dirp) == -1) {
        perror("closedir");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
