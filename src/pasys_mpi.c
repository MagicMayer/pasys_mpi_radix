#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <errno.h>
#include <math.h>
#include <mpi.h>

#include <tweet.h>
#include <radix.h>
#include <pasys_mpi.h>
#include <errors.h>

const char *BUCKETLVL_STRINGS[] = {
    "BUCKETLVL_HITS",
    "BUCKETLVL_MONTH",
    "BUCKETLVL_DAYS",
    "BUCKETLVL_TEXT",
    "BUCKETLVL_UNDEF"
};

extern int verbose;
extern DistMode __mode; 
extern uint64_t *cn_matrix_dyn;
extern uint64_t cn_matrix[LVLHITS_BUCKETSIZE][LVLMONTH_BUCKETSIZE + 1];

int  _pasys_rank = -1;
int  _pasys_size = -1;
bool _pasys_init_done = false;

cluster_info_t *world = NULL;
MPI_Datatype TweetType;

int __pasys_tweett_marshallize(void) {
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

int _pasys_init_world(void) {
    int i = 0, b_avail = BUCKETLVL_HITS, b_num = 0;
    cluster_info_t *tmp = NULL;
    if(_pasys_rank != 0) return(0);
    if(_pasys_size > 1 && (_pasys_size % 2)) return(-1);
    world = calloc(_pasys_size, sizeof(cluster_info_t));
    ASSERT_NULL_ARG(world, -1, ENOMEM);
    for(i = 0 ; i < _pasys_size ; i++) {
        tmp = (world + i);
        tmp->nodeId = i; 
        tmp->lvl = BUCKETLVL_HITS;
        if(b_avail > 0) {
            if((b_avail % _pasys_size) == 0)
                b_num = b_avail / _pasys_size;
            else
                b_num = (int) roundf((float) (b_avail / _pasys_size) + .5);
            if(b_num > 0 && b_avail <= b_num)
                b_num = b_avail;
            b_avail -= b_num;
            tmp->num_buckets = (unsigned) b_num;
        }
    }
    if(i < _pasys_size) {
        if(_pasys_rank == 0 && verbose)
            fprintf(stdout, "Node(%d): Cluster allows further partitioning: %d Nodes avail\n",
                    _pasys_rank, _pasys_size - i);
    }
    return(0);
}

int pasys_init(void) {
    if(_pasys_init_done)
        return(0);
    MPI_Init(NULL, NULL);
    (void) MPI_Comm_rank(MPI_COMM_WORLD, &_pasys_rank);
    (void) MPI_Comm_size(MPI_COMM_WORLD, &_pasys_size);
    if(__pasys_tweett_marshallize()) return(-1);
    (void) _pasys_init_world();
    _pasys_init_done = true;
    return(0);
}

int pasys_send_tweet(tweet_t *tw, int node) {
    ASSERT_NULL_ARG(tw, -1, EINVAL);
    if(node == _pasys_rank || node == -1)
        return(-1);
    MPI_Send(tw, 1, TweetType, node, MPI_SEND_TWEET_TAG, MPI_COMM_WORLD);
    return(0);
}

void pasys_counter_sync(void) {
    if(_pasys_rank == 0) {
        MPI_Reduce(MPI_IN_PLACE, cn_matrix_dyn, LVLHITS_BUCKETSIZE * (LVLMONTH_BUCKETSIZE + 1), MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    } else {
        MPI_Reduce(cn_matrix_dyn, NULL, LVLHITS_BUCKETSIZE * (LVLMONTH_BUCKETSIZE + 1), MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    }
}

void pasys_done(void) {
    if(!_pasys_init_done) return;
    MPI_Type_free(&TweetType);
    MPI_Finalize();
}

int pasys_get_filenum(void) {
    if(!_pasys_init_done) return(-1);
    return(_pasys_rank);
}

int pasys_get_rank(void) {
    if(!_pasys_init_done) return(0);
    return(_pasys_rank);
}
