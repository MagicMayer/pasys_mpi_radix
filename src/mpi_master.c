#include <stdio.h>
#include <string.h>
#if __STDC_VERSION__ >= 199901L
    #include <stdbool.h>
#else
    #define bool char
    #define true 1
    #define false 0
#endif

#include <mpi.h>

int  _mpi_rank = -1;
int  _mpi_size = -1;
bool _mpi_init_done = false;

int _mpi_init_master(void) {
    if(_mpi_rank != 0) return(-1);
    if((_mpi_size % 2)) {
        fprintf(stderr, "<%s> Uneven Number of %d Nodes in Cluster not supported!\n", __func__, _mpi_size);
        return(-1);
    }
    return(0);
}

int _mpi_init_slave(void) {
    if(_mpi_rank == 0) return(-1);
    return(0);
}

int mpi_init(void) {
    if(_mpi_init_done)
        return(0);
    (void) MPI_Comm_rank(MPI_COMM_WORLD, &_mpi_rank);
    (void) MPI_Comm_size(MPI_COMM_WORLD, &_mpi_size);
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
    printf("<%s> Setup done: rank(%d) size(%d)\n", __func__, _mpi_rank, _mpi_size);
    return(0);
}


