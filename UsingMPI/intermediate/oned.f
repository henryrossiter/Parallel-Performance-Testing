c*******************************************************************
c   oned.f - a solution to the Poisson problem using Jacobi 
c   interation on a 1-d decomposition
c     
c   The size of the domain is read by processor 0 and broadcast to
c   all other processors.  The Jacobi iteration is run until the 
c   change in successive elements is small or a maximum number of 
c   iterations is reached.  The difference is printed out at each 
c   step.
c*******************************************************************
      program main 
C
      include "mpif.h"
      integer maxn
      parameter (maxn = 128)
      double precision  a(maxn,maxn), b(maxn,maxn), f(maxn,maxn)
      integer nx, ny
      integer myid, numprocs, ierr
      integer comm1d, nbrbottom, nbrtop, s, e, it
      double precision diff, diffnorm, dwork
      double precision t1, t2
      double precision MPI_WTIME
      external MPI_WTIME
      external diff

      call MPI_INIT( ierr )
      call MPI_COMM_RANK( MPI_COMM_WORLD, myid, ierr )
      call MPI_COMM_SIZE( MPI_COMM_WORLD, numprocs, ierr )
c
      if (myid .eq. 0) then
c
c         Get the size of the problem
c
c          print *, 'Enter nx'
c          read *, nx
           nx = 110
      endif
      call MPI_BCAST(nx,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)
      ny   = nx
c
c Get a new communicator for a decomposition of the domain
c
      call MPI_CART_CREATE( MPI_COMM_WORLD, 1, numprocs, .false., 
     $                    .true., comm1d, ierr )
c
c Get my position in this communicator, and my neighbors
c 
      call MPI_COMM_RANK( comm1d, myid, ierr )
      call MPI_Cart_shift( comm1d, 0,  1, nbrbottom, nbrtop, ierr   )
c
c Compute the actual decomposition
c     
      call MPE_DECOMP1D( ny, numprocs, myid, s, e )
c
c Initialize the right-hand-side (f) and the initial solution guess (a)
c
      call onedinit( a, b, f, nx, s, e )
c
c Actually do the computation.  Note the use of a collective operation to
c check for convergence, and a do-loop to bound the number of iterations.
c
      call MPI_BARRIER( MPI_COMM_WORLD, ierr )
      t1 = MPI_WTIME()
      do 10 it=1, 100
c      do 10 it=1, 2
	call exchng1( a, nx, s, e, comm1d, nbrbottom, nbrtop )
	call sweep1d( a, f, nx, s, e, b )
	call exchng1( b, nx, s, e, comm1d, nbrbottom, nbrtop )
	call sweep1d( b, f, nx, s, e, a )
	dwork = diff( a, b, nx, s, e )
	call MPI_Allreduce( dwork, diffnorm, 1, MPI_DOUBLE_PRECISION, 
     $                      MPI_SUM, comm1d, ierr )
        if (diffnorm .lt. 1.0e-5) goto 20
c        if (myid .eq. 0) print *, 2*it, ' Difference is ', diffnorm
10     continue
      if (myid .eq. 0) print *, 'Failed to converge'
20    continue
      t2 = MPI_WTIME()
      if (myid .eq. 0) then
         print *, 'Converged after ', 2*it, ' Iterations in ', t2 - t1,
     $        ' secs '
      endif
c
      call MPI_FINALIZE(ierr)
      end


