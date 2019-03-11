#include <stdio.h>
#include "ibcast.h"
static int ibcast_keyval = MPI_KEYVAL_INVALID; /* keyval for
                                             attribute caching */
int Ibcast(void *buf, int count, MPI_Datatype datatype, int root,
           MPI_Comm comm, Ibcast_handle **handle_out)
{
    Ibcast_syshandle *syshandle;
    Ibcast_handle    *handle;
    int                   flag, mask, relrank;
    int                   retn, size, rank;
    int                   req_no = 0;
    /* get info about the communicator */
    MPI_Comm_size ( comm, &size );
    MPI_Comm_rank ( comm, &rank );
    /* If size is 1, this is trivially finished */
    if (size == 1) {
        (*handle_out) = (Ibcast_handle *) 0;
        return (MPI_SUCCESS);
    }
    /* first see if this library has ever been called. Generate new
       key value if not. */
    if (ibcast_keyval == MPI_KEYVAL_INVALID) {
        MPI_Keyval_create( MPI_NULL_COPY_FN, MPI_NULL_DELETE_FN, 
                           &ibcast_keyval, NULL);
    }
    /* this communicator might already have used this collective
       operation, and so it would consequently have information
       of use to us cached on it. */
    MPI_Attr_get(comm, ibcast_keyval, (void **)&syshandle, &flag);
    if (flag == 0) { /* there was no attribute previously cached */
        syshandle = 
            (Ibcast_syshandle *)malloc(sizeof(Ibcast_syshandle));
        /* fill in the attribute information */
        syshandle->ordering_tag = 0; /* start with tag zero */
        MPI_Comm_dup(comm, &(syshandle->comm)); /* duplicate comm */
        /* attach information to the communicator */
        MPI_Attr_put(comm, ibcast_keyval, (void *)syshandle);
    }
    /* create a handle for this particular ibcast operation */
    handle = (Ibcast_handle *)malloc(sizeof(Ibcast_handle));
    handle->num_sends = 0;
    handle->num_recvs = 0;
    /* count how many send/recv handles we need */
    mask    = 0x1;
    relrank = (rank - root + size) % size;
    while ((mask & relrank) == 0 && mask < size) {
        if ((relrank | mask) < size) 
            handle->num_sends++;
        mask <<= 1;
    }
    if (mask < size) handle->num_recvs++;
    /* allocate request and status arrays for sends and receives */
    handle->req_array  = (MPI_Request *)
        malloc(sizeof(MPI_Request) * (handle->num_sends +
                                      handle->num_recvs));
    handle->stat_array = (MPI_Status *)
        malloc(sizeof(MPI_Status) * (handle->num_sends +
                                     handle->num_recvs));
    /* create "permanent" send/recv requests */
    mask    = 0x1;
    relrank = (rank - root + size) % size;
    while ((mask & relrank) == 0 && mask < size) {
	if ((relrank|mask) < size) 
	    MPI_Send_init(buf, count, datatype,
                          ((relrank|mask)+root)%size, 
			  syshandle->ordering_tag, syshandle->comm, 
			  &(handle->req_array[req_no++]));
	mask <<= 1;
    }
    if (mask < size) 
	MPI_Recv_init(buf, count, datatype, 
		      ((relrank & (~ mask)) + root) % size,
		      syshandle->ordering_tag, syshandle->comm,
		      &(handle->req_array[req_no++]));
    retn = Ibcast_work(handle);
    /* prepare to update the cached information */
    ++(syshandle->ordering_tag); /* make bigger for next ibcast
                                  operation to avoid back-masking */
    (*handle_out) = handle;     /* return the handle */
    return(retn);
}

/* copier for ibcast cached information */
int Ibcast_copy(MPI_Comm  *oldcomm, int *keyval,
     void *extra, void *attr_in, void **attr_out, int *flag)
{
    Ibcast_syshandle *syshandle = (Ibcast_syshandle *)attr_in;
    Ibcast_syshandle *new_syshandle;
    /* do we have a valid keyval */
    if ( ( (*keyval)    == MPI_KEYVAL_INVALID )        ||
	 ( (*keyval)    != ibcast_keyval )             ||
	 ( (syshandle   == (Ibcast_syshandle *)0) ) ) {
	/* A non-zero return is an error and will cause MPI_Comm_dup
	   to signal an error.  The return value here is *not* the
	   MPI error code that MPI_Comm_dup will return */
      return 1;
    }
    /* create a new syshandle for the new communicator */
    new_syshandle = 
	(Ibcast_syshandle *)malloc(sizeof(Ibcast_syshandle));
    /* fill in the attribute information */
    new_syshandle->ordering_tag = 0; /* start with tag zero */
    /* dup the "hidden" communicator */
    MPI_Comm_dup(syshandle->comm, &(new_syshandle->comm));
    /* return the new syshandle and set flag to true */
    (*attr_out) = (void *)new_syshandle;
    (*flag)     = 1;
    return MPI_SUCCESS;
}

/* destructor for ibcast cached information */
int Ibcast_delete(MPI_Comm *comm, int *keyval, void *attr_val,
                   void *extra)
{
    Ibcast_syshandle *syshandle = (Ibcast_syshandle *)attr_val;
    /* do we have a valid keyval */
    if ( ( (*keyval)    == MPI_KEYVAL_INVALID )        ||
	 ( (*keyval)    != ibcast_keyval )             ||
	 ( (syshandle   == (Ibcast_syshandle *)0) ) ) {
	/* Give a non-zero return code to indicate an error */
	return 1;
    }
    /* free the "hidden" communicator and memory for syshandle */
    MPI_Comm_free ( &(syshandle->comm) );
    free (syshandle);
    return MPI_SUCCESS;
}

int Ibcast_work(Ibcast_handle *handle)
{
    /* if I don't have any recv's, start all my sends -- the root */
    if (handle->num_recvs == 0)
        MPI_Startall( handle->num_sends, handle->req_array );
    /* start all my recv's */
    else 
        MPI_Startall( handle->num_recvs, 
                       &handle->req_array[handle->num_sends] );
    return (MPI_SUCCESS);
}

int Ibcast_wait(Ibcast_handle **handle_out)
{
    Ibcast_handle *handle = (*handle_out);
    int retn, i;
    /* A NULL handle means there's nothing to do */
    if (handle == (Ibcast_handle *)0)
        return (MPI_SUCCESS);
    /* If I wasn't the root, finish my recvs and 
       start & wait on sends */
    if (handle->num_recvs != 0) {
	MPI_Waitall(handle->num_recvs,
		    &handle->req_array[handle->num_sends],
		    &handle->stat_array[handle->num_sends]);
	MPI_Startall ( handle->num_sends, handle->req_array );
    }
    /* Wait for my receive and then start all my sends */
    retn = MPI_Waitall(handle->num_sends, handle->req_array,
                       handle->stat_array);
    /* free permanent requests */
    for (i=0; i < (handle->num_sends + handle->num_recvs); i++)
        MPI_Request_free (&(handle->req_array[i]));
    /* Free request and status arrays and ibcast handle */
    free (handle->req_array);
    free (handle->stat_array);
    free (handle);
    /* Return a NULL handle */
    (*handle_out) = (Ibcast_handle *)0;
    return(retn);
}
