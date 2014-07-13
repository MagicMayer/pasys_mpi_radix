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
#include <radix.h>
#include <pasys_mpi.h>

#define BASEPATH    "/home/steffen/Studium/_aktuelles.Semester/M31.Prallel.Systems/Beleg"
// #define BASEPATH    "/mpidata/parsys14/gross"

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

/* implicit ((noreturn)) labled */
void terminate(int code) {
    twcache_destroy();
    pasys_done();
    exit(code);
}

int main(int argc, char *argv[]) {
    char *key = NULL, fname[MAXPATHLEN + 1];
    off_t start = 0, end = 0;
    int opt = 0, parsed = 0;
    time_t prog_start; 

    if((progname = strrchr(argv[0], '/')) != NULL) {
        progname++;
    } else {
        progname = argv[0];
    }

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
        terminate(-1);
    } else if(__mode >= PASYS_BENCH_2) {
        snprintf(fname, MAXPATHLEN, "%s/twitter.data.%d", BASEPATH, pasys_get_filenum());
        if(verbose)
            fprintf(stdout, "%s: Node(%d) handling file: %s\n", progname, pasys_get_rank(), fname);
    } else {
        fprintf(stderr, "%s: Unknown Mode %d\n", progname, __mode);
        terminate(-1);
    }

    prog_start = time(NULL);
    if((parsed = tweetfile_parse(fname, key, start, end)) <= 0) {
        fprintf(stderr, "%s: failed to parse file '%s': (%d) %s\n", progname, fname, errno, 
                strerror(errno));
        terminate(-1);
    }
    if(verbose)
        fprintf(stdout, "Parsed %d Elements - took %02lu seconds\n", parsed, (time(NULL) - prog_start));
    
    MPI_Barrier(MPI_COMM_WORLD);
    pasys_counter_sync();
    twcache_print_all();

    terminate(0);
}
