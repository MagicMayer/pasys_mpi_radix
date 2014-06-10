#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <mpi.h>

#include <tweet.h>

static char *progname = NULL;
extern int mpi_init(void);

extern int tweetfile_parse(const char *);

int main(int argc, char *argv[]) {
    if((progname = strrchr(argv[0], '/')) != NULL) {
        progname++;
    } else {
        progname = argv[0];
    }

    if(tweetfile_parse("../klein/twitter.data.0")) {
        printf("Fehler\n");
    }

//     if(mpi_init() != 0) {
//         MPI_Finalize();
//         return(1);
//     }
//     MPI_Finalize();
// 
    return(0);
}
