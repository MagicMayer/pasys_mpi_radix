#ifndef __pasys_mpi_h__
#define __pasys_mpi_h__ 1

enum __dist_mode {
    PASYS_BENCH_1   =   0,
    PASYS_BENCH_2   =   1,
    PASYS_BENCH_3   =   2,
    PASYS_UNDEF     =   4
};
typedef enum __dist_mode DistMode; 

pasys_init(void);
pasys_done(void);

pasys_get_filenum(void);
#endif /* __pasys_mpi_h__ */
