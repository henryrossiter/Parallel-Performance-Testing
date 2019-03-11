       program main
       use mpi
       integer ierr
       integer n, m, size
       parameter (n=10,m=20)
       real    lmatrix(n,m), lx(m), ly(m)      ! Local matrix and vectors
       integer MAX_PROCS
       parameter (MAX_PROCS=64)
       integer counts(MAX_PROCS)

       call MPI_INIT( ierr )
       call MPI_COMM_SIZE( MPI_COMM_WORLD, size, ierr )
       do i=1, size
          counts(i) = m
       enddo
       call matvec( n, m, lmatrix, lx, ly, counts, MPI_COMM_WORLD )

       call MPI_FINALIZE( ierr )
       stop
       end
