#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <mpi.h>
#include <tweet.h>
#include <errors.h>

#define MAX_FILES

extern int verbose;

int  _mpi_rank = -1;
int  _mpi_size = -1;
bool _mpi_init_done = false;

MPI_Datatype TweetType; 

int __mpi_tweett_marshallize(void) {
    MPI_Datatype types[] = { MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, 
        MPI_UNSIGNED_CHAR, MPI_CHAR, MPI_UNSIGNED_SHORT, MPI_UNSIGNED };
    int blocks[] = { 1, 1, 1, MAX_TWEET_LENGTH + 1, 1, 1};
    MPI_Aint offsets[] = { 
        offsetof(tweet_t, hits),
        offsetof(tweet_t, month),
        offsetof(tweet_t, day),
        offsetof(tweet_t, text),
        offsetof(tweet_t, fnum),
        offsetof(tweet_t, lnum)
    };
    unsigned elems = (unsigned) sizeof(offsets) / sizeof(MPI_Aint);
    if(MPI_Type_create_struct(elems, blocks, offsets, types, &TweetType)) {
        return(-1);
    }
    if(MPI_Type_commit(&TweetType)) {
        return(-1);
    }
    return(0);
}

int _mpi_init_master(void) {
    if(_mpi_rank != 0) return(-1);
    if((_mpi_size % 2)) return(-1);
    int *buf = calloc(10, sizeof(int));
    int receive = 0;
    for(int i = 0 ; i < 10 ; i++) {
        buf[i] = i * 23;
    }
    MPI_Scatter(buf, 1, MPI_INT, &receive, 1, MPI_INT, 0, MPI_COMM_WORLD);
    return(0);
}

int _mpi_init_slave(void) {
    int recv;
    if(!_mpi_rank) return(-1);
    MPI_Scatter(NULL, 0, MPI_INT, &recv, 1, MPI_INT, 0, MPI_COMM_WORLD);
    return(0);
}

int mpi_init(void) {
    if(_mpi_init_done)
        return(0);
    MPI_Init(NULL, NULL);
    (void) MPI_Comm_rank(MPI_COMM_WORLD, &_mpi_rank);
    (void) MPI_Comm_size(MPI_COMM_WORLD, &_mpi_size);
    if(__mpi_tweett_marshallize()) return(-1);
    switch(_mpi_rank) {
        case -1:
            fprintf(stderr, "<%s> Invalid _mpi_rank %d\n", __func__, _mpi_rank);
            break;
        case 0:
            if(_mpi_init_master()) return(-1);
            break;
        default:
            if(_mpi_init_slave()) return(-1);
            break;
    }
    _mpi_init_done = true;
    if(verbose)
        printf("<%s> Setup done: rank(%d) size(%d)\n", __func__, _mpi_rank, _mpi_size);
    return(0);
}

int mpi_get_filenum(void) {
    if(!_mpi_init_done) return(-1);
    return(_mpi_rank);
}

void mpi_done(void) {
    if(!_mpi_init_done) return;
    MPI_Finalize();
}
