#include <mpi.h>
#include <stdio.h>
#include "ibcast.h"

int main( int argc, char *argv[] )
{
    int buf1[10], buf2[20];
    int rank, i;
    Ibcast_handle *ibcast_handle_1, *ibcast_handle_2;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    if (rank == 0) {
	for (i=0; i<10; i++) buf1[i] = i;
	for (i=0; i<20; i++) buf2[i] = -i;
    }
    Ibcast( buf1, 10, MPI_INT, 0, MPI_COMM_WORLD, &ibcast_handle_1 );
    Ibcast( buf2, 20, MPI_INT, 0, MPI_COMM_WORLD, &ibcast_handle_2 );
    Ibcast_wait( &ibcast_handle_1 );
    Ibcast_wait( &ibcast_handle_2 );
    for (i=0; i<10; i++) {
	if (buf1[i] != i) printf( "buf1[%d] = %d on %d\n", i, buf1[i], rank );
    }
    for (i=0; i<20; i++) {
	if (buf2[i] != -i) printf( "buf2[%d] = %d on %d\n", i, buf2[i], rank );
    }
    MPI_Finalize();
    return 0;
}
