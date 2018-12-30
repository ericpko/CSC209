#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "freq_list.h"
#include "worker.h"

int main(int argc, char **argv) {
    Node *head = NULL;
    char **filenames = init_filenames();
    char arg;
    char *listfile = "index";
    char *namefile = "filenames";

    /* an example of using getop to process command-line flags and arguments */
    while ((arg = getopt(argc, argv, "i:n:")) > 0) {
        switch(arg) {
        case 'i':
            listfile = optarg;
            break;
        case 'n':
            namefile = optarg;
            break;
        default:
            fprintf(stderr, "Usage: test_get_word [-i FILE] [-n FILE]\n");
            exit(EXIT_FAILURE);
        }
    }

    read_list(listfile, namefile, &head, filenames);
    FreqRecord *frp = NULL;
    for (Node *curr = head; curr != NULL; curr = curr->next) {
        frp = get_word(curr->word, head, filenames);
        printf("The FreqRecord array for word [\"%s\"] is:\n", curr->word);
        print_freq_records(frp);
        printf("\n");
    }

    // case: word not found
    printf("Case: word is not found\n");
    frp = get_word("abaracadabara", head, filenames);
    print_freq_records(frp);

    printf("All tests succeeded!\n");

    return EXIT_SUCCESS;
}
