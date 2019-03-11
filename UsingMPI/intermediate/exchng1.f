	subroutine exchng1( a, nx, s, e, comm1d, nbrbottom, nbrtop )
	include 'mpif.h'
	integer nx, s, e
	double precision a(0:nx+1,s-1:e+1)
	integer comm1d, nbrbottom, nbrtop
	integer status(MPI_STATUS_SIZE), ierr
c
	call MPI_SENDRECV( 
     &            a(1,e), nx, MPI_DOUBLE_PRECISION, nbrtop, 0, 
     &            a(1,s-1), nx, MPI_DOUBLE_PRECISION, nbrbottom, 0, 
     &            comm1d, status, ierr )
	call MPI_SENDRECV( 
     &            a(1,s), nx, MPI_DOUBLE_PRECISION, nbrbottom, 1, 
     &            a(1,e+1), nx, MPI_DOUBLE_PRECISION, nbrtop, 1, 
     &            comm1d, status, ierr )
	return
	end
