#ifndef __pasys_mpi_h__
#define __pasys_mpi_h__ 1

#define MPI_SYNC_DATA_TAG   1
#define MPI_SYNC_DONE_TAG   2
#define MPI_SEND_TWEET_TAG  3

enum __dist_mode {
    PASYS_BENCH_1   =   0,
    PASYS_BENCH_2   =   1,
    PASYS_BENCH_3   =   2,
    PASYS_UNDEF     =   4
};
typedef enum __dist_mode DistMode; 

enum __bucket_lvl {
    BUCKETLVL_HITS  =   0,
    BUCKETLVL_MONTH =   1,
    BUCKETLVL_DAYS  =   2,
    BUCKETLVL_TEXT  =   3,
    BUCKETLVL_UNDEF =   4
};
typedef enum __bucket_lvl BucketLvl;

struct __cluster_info {
    uint32_t    nodeId;
    BucketLvl   lvl;
    uint8_t     num_buckets;
    uint8_t     *buckets;
};
typedef struct __cluster_info cluster_info_t;

int pasys_init(void);
void pasys_done(void);

int pasys_get_filenum(void);
int pasys_get_rank(void);
void pasys_counter_sync(void);
#endif /* __pasys_mpi_h__ */
