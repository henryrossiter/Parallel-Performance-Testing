	subroutine exchng1( a, nx, s, e, comm1d, &
                            nbrbottom, nbrtop )
	use mpi
	integer nx, s, e
	double precision a(0:nx+1,s-1:e+1)
	integer comm1d, nbrbottom, nbrtop
	integer status_array(MPI_STATUS_SIZE,4), ierr, req(4)
!
        call MPI_IRECV ( &
             a(1,s-1), nx, MPI_DOUBLE_PRECISION, nbrbottom, 0, &
             comm1d, req(1), ierr )
        call MPI_IRECV ( &
             a(1,e+1), nx, MPI_DOUBLE_PRECISION, nbrtop, 1, &
             comm1d, req(2), ierr )
        call MPI_ISEND ( &
             a(1,e), nx, MPI_DOUBLE_PRECISION, nbrtop, 0, &
             comm1d, req(3), ierr )
        call MPI_ISEND ( &
             a(1,s), nx, MPI_DOUBLE_PRECISION, nbrbottom, 1, &
             comm1d, req(4), ierr )
!
        call MPI_WAITALL ( 4, req, status_array, ierr )
	return
	end
