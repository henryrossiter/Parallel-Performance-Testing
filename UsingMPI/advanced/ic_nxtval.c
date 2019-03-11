#include "mpi.h"
#include "ic_nxtval.h"

int MPE_Counter_nxtval_ic( MPI_Comm counter_comm, int *value )
{
    MPI_Status status;
    /* always request/receive services from 
       intercomm (remote) rank=0 */
    MPI_Send(MPI_BOTTOM, 0, MPI_INT, SERVER_RANK, REQUEST, counter_comm );
    MPI_Recv(value, 1, MPI_INT, SERVER_RANK, VALUE, counter_comm, 
	     &status );
    return(0);
}
