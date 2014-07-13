#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include <tweet.h>
#include <radix.h>
#include <pasys_mpi.h>
#include <errors.h>

extern int verbose;
extern int _pasys_rank;

uint64_t *cn_matrix_dyn = NULL;
bool matrix_initialized = false;
static int total_matrix_count;

int count_elements(tweet_t *tw) {
    if(!matrix_initialized) {
        cn_matrix_dyn = calloc(LVLHITS_BUCKETSIZE * (LVLMONTH_BUCKETSIZE + 1), sizeof(uint64_t));
        total_matrix_count = 0;
        matrix_initialized = true;
    }
    if(tw->hits > LVLHITS_BUCKETSIZE || tw->month > LVLMONTH_BUCKETSIZE) {
        fprintf(stderr, "%s Error: (%d > %d) || (%d > %d)\n", __func__, tw->hits, LVLHITS_BUCKETSIZE,
                tw->month, LVLMONTH_BUCKETSIZE);
        return(-1);
    }
    *(cn_matrix_dyn + CN_MATRIX_OFFSET_TW(tw)) += 1;
    *(cn_matrix_dyn + CN_MATRIX_OFFSET_DIM(tw,tw->month)) += 1;
    total_matrix_count++;
    return(0);
}

void print_matrix(void) {
    uint64_t total = 0, month = 0;
    for(int i = 0 ; i < CN_MATRIX_MAXSIZE ; i++) {
        if((i % 13) == 0 && *(cn_matrix_dyn + i) != 0) {
            total += *(cn_matrix_dyn + i);
            if(verbose)
                fprintf(stdout, "cn_matrix_dyn[%d] = %lu\n", i, cn_matrix_dyn[i]);
        } else if ((i % 13) != 0) {
            month += *(cn_matrix_dyn + i);
        }
    }
    fprintf(stderr, "Node(%d) Number of Tweets in Cluster: %d/%lu/%lu\n", 
            _pasys_rank, total_matrix_count, total, month);
}
