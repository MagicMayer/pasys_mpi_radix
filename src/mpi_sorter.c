#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>

#include <mpi.h>

#include <tweet.h>

static char *progname = NULL;
extern int mpi_init(void);

extern int opterr;
extern char *optarg;

static void print_version(void) {
    printf("%s Version: 0.4.2-23 by M.K. & S.B.\n", progname);
}

static void print_usage(void) {
    printf("Usage: %s [-fkhv]\n", progname);
    printf("\t-f:\tName und Pfad des Tweetfiles\n");
    printf("\t-k:\tKeyword nach dem gesucht werden soll\n");
    printf("\t-h:\tHilfe\n");
    printf("\t-v:\tVersionsnummer\n");
    print_version();
}

int main(int argc, char *argv[]) {
    char *fname = NULL, *key = NULL;
    int opt = 0, parsed = 0;
    time_t now, then;

    if((progname = strrchr(argv[0], '/')) != NULL) {
        progname++;
    } else {
        progname = argv[0];
    }

    now = time((time_t *) 0);
    opterr = 0; /* do not print getopt error messages */
    while((opt = getopt(argc, argv, "f:k:hv")) != -1) {
        switch(opt) {
            case 'f':
                fname = optarg;
                break;
            case 'k':
                key = optarg;
                break;
            case 'h':
                print_usage();
                exit(0);
            case 'v':
                print_version();
                exit(0);
            default:
                printf("%s: invalid option '%c'\n", progname, optopt);
                print_usage();
                exit(1);
        }
    }

    if(!key) {
        printf("%s: no Key stated!\n", progname);
        print_usage();
        exit(1);
    }
    if(!fname) {
        fname = "../Beleg/twitter.data.0";   
    }

    if((parsed = tweetfile_parse(fname, key)) <= 0) {
        printf("Fehler\n");
    } else {
        then = time((time_t *) 0);
        printf("%s: parsed %d Lines in %lus\n", progname, parsed, (then - now));
    }

    return(0);
}
