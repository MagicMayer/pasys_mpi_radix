#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/param.h>
#include <errno.h>
#include <unistd.h>

#include <mpi.h>

#include <tweet.h>
#include <pasys_mpi.h>

extern void twcache_print_all(void);

extern int opterr;
extern char *optarg;
extern int optind;

char *progname = NULL;
int verbose = 0;
DistMode __mode = PASYS_UNDEF;

static void print_version(void) {
    printf("%s Version: 0.4.2-23 by M.K. & S.B.\n", progname);
}

static void print_usage(void) {
    printf("Usage: %s [-fhv] <key>\n", progname);
    printf("\t-f:\tName und Pfad des Tweetfiles\n");
    printf("\t-s:\tLese Tweetfile ab Position <count>\n");
    printf("\t-e:\tLese Tweetfile bis Position <count>\n");
    printf("\t-v:\tVerbose Mode\n");
    printf("\t-h:\tHilfe\n");
    printf("\t-V:\tVersionsnummer\n");
    print_version();
}

int main(int argc, char *argv[]) {
    char *key = NULL, fname[MAXPATHLEN + 1];
    off_t start = 0, end = 0;
    int opt = 0, parsed = 0;
    time_t now, then;

    if((progname = strrchr(argv[0], '/')) != NULL) {
        progname++;
    } else {
        progname = argv[0];
    }

    now = time((time_t *) 0);
    opterr = 0; /* do not print getopt error messages */
    while((opt = getopt(argc, argv, "f:k:s:e:hv")) != -1) {
        switch(opt) {
            case 's':
                start = strtoul(optarg, NULL, 10);
                break;
            case 'e':
                end = strtoul(optarg, NULL, 10);
                break;
            case 'f':
                strncpy(fname, optarg, MAXPATHLEN);
                __mode = PASYS_BENCH_1;
                break;
            case 'h':
                print_usage();
                exit(0);
            case 'v':
                verbose++;
                break;
            case 'V':
                print_version();
                exit(0);
            default:
                printf("%s: invalid option '%c'\n", progname, optopt);
                print_usage();
                exit(1);
        }
    }

    if(optind >= argc) {
        fprintf(stderr, "No key Stated!\n");
        print_usage();
        return(1);
    }
    key = argv[optind]; 

    if(__mode == PASYS_UNDEF) {
        __mode = PASYS_BENCH_2;
    }

    if(pasys_init()) {
        fprintf(stderr, "%s: Failed to initialized MPI Cluster\n", progname);
        return(1);
    } else {
        if(verbose)
            fprintf(stderr, "%s: Successfully initialized MPI Cluster\n", progname);
    }

    if(__mode == PASYS_BENCH_1 && (strlen(fname) <= 0 || !*fname)) {
        fprintf(stderr, "%s: Try to start Benchmark 1 without stated Filename!\n", progname);
        goto stop_it;
    } else {
        snprintf(fname, MAXPATHLEN, "/mpidata/parsys14/gross/twitter.data.%d", mpi_get_filenum());
    }

    parsed = tweetfile_parse(fname, key, start, end);
    if(parsed <= 0) {
        fprintf(stderr, "%s: Reading %s failed: (%d) %s\n", progname, fname, errno, strerror(errno));
        goto stop_it;
    }

    if((parsed = tweetfile_parse(fname, key, start, end)) <= 0) {
        printf("fehler\n");
    } else {
        then = time((time_t *) 0);
        printf("%s: parsed %d Lines in %lus\n", progname, parsed, (then - now));
        twcache_print_all();
    }

stop_it:
    mpi_done();
    return(0);
}
