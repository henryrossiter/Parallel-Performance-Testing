	subroutine exchng1( a, nx, s, e, comm1d, nbrbottom, nbrtop )
	use mpi
	integer nx, s, e
	double precision a(0:nx+1,s-1:e+1)
	integer comm1d, nbrbottom, nbrtop, rank, coord
	integer status(MPI_STATUS_SIZE), ierr
!
        call MPI_COMM_RANK( comm1d, rank, ierr )
        call MPI_CART_COORDS( comm1d, rank, 1, coord, ierr )
        if (mod( coord, 2 ) .eq. 0) then
	  call MPI_SEND( a(1,e), nx, MPI_DOUBLE_PRECISION, &
                         nbrtop, 0, comm1d, ierr )
	  call MPI_RECV( a(1,s-1), nx, MPI_DOUBLE_PRECISION, & 
                         nbrbottom, 0, comm1d, status, ierr )
	  call MPI_SEND( a(1,s), nx, MPI_DOUBLE_PRECISION, &
                         nbrbottom, 1, comm1d, ierr )
	  call MPI_RECV( a(1,e+1), nx, MPI_DOUBLE_PRECISION, & 
                         nbrtop, 1, comm1d, status, ierr )
        else 
	  call MPI_RECV( a(1,s-1), nx, MPI_DOUBLE_PRECISION, &
                         nbrbottom, 0, comm1d, status, ierr )
	  call MPI_SEND( a(1,e), nx, MPI_DOUBLE_PRECISION, &
                         nbrtop, 0, comm1d, ierr )
	  call MPI_RECV( a(1,e+1), nx, MPI_DOUBLE_PRECISION, & 
                         nbrtop, 1, comm1d, status, ierr )
	  call MPI_SEND( a(1,s), nx, MPI_DOUBLE_PRECISION, &
                         nbrbottom, 1, comm1d, ierr )
        endif
	return
	end
