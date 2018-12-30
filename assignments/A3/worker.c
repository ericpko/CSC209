#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"


/* Return a positive number if the value of <b> is greater than <a>,
 * a negative number if <b> is less than <a>, and 0 otherwise.
 */
int compare(const void *a, const void *b) {
    // cast the void pointers of <a> and <b>
    const FreqRecord *A = (FreqRecord *)a;
    const FreqRecord *B = (FreqRecord *)b;

    return (B->freq - A->freq); // B - A for decending order
}


/* Return an array of FreqRecord elements for a given <word> with the last
 * element an empty FreqRecord. An empty FreqRecord has a freq of
 * 0 and a filename set to an empty string.
 *
 * If word is not in the linked list <head>, then return an empty
 * FreqRecord.
*/
FreqRecord *get_word(char *word, Node *head, char **file_names) {
    // find the Node with the corresponding word
    Node *curr = NULL;
    for (curr = head; curr != NULL; curr = curr->next) {
        if (strcmp(curr->word, word) == 0) {
            break;
        }
    }
    // case: word is not found in linkedlist.
    FreqRecord *freq_record;
    if (curr == NULL) {
        if ((freq_record = malloc(sizeof(FreqRecord))) == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        freq_record->freq = 0;
        freq_record->filename[0] = '\0';
        return freq_record;
    }
    // get the number of files containing <word>
    int count = 0;
    for (int i = 0; i < MAXFILES; i++) {
        if (curr->freq[i] > 0) {
            count++;
        }
    }
    count++; // for an empty FreqRecord
    // allocate space for the new FreqRecord array
    if ((freq_record = malloc(count * sizeof(FreqRecord))) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    // add to <freq_record> list (unordered)
    for (int i = 0, j = 0; i < MAXFILES; i++) {
        if (curr->freq[i] > 0) {
            freq_record[j].freq = curr->freq[i];
            strncpy(freq_record[j].filename, file_names[i], PATHLENGTH);
            freq_record[j].filename[PATHLENGTH - 1] = '\0';
            j++;
        }
    }
    // add zero freq and empty string
    freq_record[count - 1].freq = 0;
    freq_record[count - 1].filename[0] = '\0';

    // sort the FreqRecord array using built-in qsort with decreasing freq
    qsort(freq_record, count, sizeof(FreqRecord), compare);

    return freq_record;
}

/* Print to standard output the frequency records for a word.
 * Use this for your own testing and also for query.c
 */
void print_freq_records(FreqRecord *frp) {
    int i = 0;

    while (frp != NULL && frp[i].freq != 0) {
        printf("%d    %s\n", frp[i].freq, frp[i].filename);
        i++;
    }
}

/* Search the given directory for the index and filename files. For each
 * word read from file descriptor <in>, write a FreqRecord  to <out> for
 * each file in which the word has a non-zero frequency. Then write on final
 * empty FreqRecord to signify the end.
*/
void run_worker(char *dirname, int in, int out) {
    Node *head = NULL;
    char **filenames = init_filenames();
    char word[MAXWORD], listfile[PATHLENGTH], namefile[PATHLENGTH];
    FreqRecord *frp;
    int n;
    char *newline;

    // get the pathname to the index and filename in the given directory
    strncpy(listfile, dirname, sizeof(listfile));
    strncat(listfile, "/index", sizeof(listfile) - strlen(listfile) - 1);
    listfile[PATHLENGTH - 1] = '\0';
    strncpy(namefile, dirname, sizeof(namefile));
    strncat(namefile, "/filenames", sizeof(namefile) - strlen(namefile) - 1);
    namefile[PATHLENGTH - 1] = '\0';

    // note: if index or filename do not exist, read_list will exit
    read_list(listfile, namefile, &head, filenames);

    // read a word from file descriptor in until it's closed
    while ((n = read(in, word, MAXWORD - 1)) > 0) {
        word[n] = '\0';
        if ((newline = strchr(word, '\n')) != NULL) {
            *newline = '\0';
        }
        // create FreqRecord array and write one FreqRecord to fd out
        frp = get_word(word, head, filenames);
        int i = 0;
        while (frp != NULL && frp[i].freq > 0) {
            if (write(out, &frp[i], sizeof(FreqRecord)) == -1) {
                perror("write: from run_worker");
                exit(EXIT_FAILURE);
            }
            i++;
        }
        // free FreqRecord array
        free(frp);
    }
    if (n == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    // add empty FreqRecord to signify the end
    FreqRecord fr; // can't use frp because free'd memory
    fr.freq = 0;
    fr.filename[0] = '\0';
    if (write(out, &fr, sizeof(FreqRecord)) == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    // let the calling function close the file descriptors

    return;
}
