#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <mpi.h>

#include <tweet.h>

static char *progname = NULL;
extern int mpi_init(void);

int main(int argc, char *argv[]) {
    tweet_t t1 = { .line = 0, .hits = 42, .month=6 , .day=3, .text = "Hallo"};
    tweet_t t2 = { .line = 0, .hits = 42, .month=7 , .day=2, .text = "Hallo"};
    int res = 0;

    if((progname = strrchr(argv[0], '/')) != NULL) {
        progname++;
    } else {
        progname = argv[0];
    }

    MPI_Init(&argc, &argv);
    if(mpi_init() != 0) {
        MPI_Finalize();
        return(1);
    }
    res = tweet_compare(&t1, &t2);
    printf("Comparing tweets returned: %d\n", res);
    tweet_print(stdout, &t1);
    tweet_print(stdout, &t2);

    MPI_Finalize();

    return(0);
}
