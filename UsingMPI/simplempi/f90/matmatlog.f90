!     matmat.f - matrix - matrix multiply, 
!     simple self-scheduling version
      program main
      use mpi
      integer MAX_AROWS, MAX_ACOLS, MAX_BCOLS 
      parameter (MAX_AROWS = 20, MAX_ACOLS = 1000, MAX_BCOLS = 20)
      double precision a(MAX_AROWS,MAX_ACOLS), b(MAX_ACOLS,MAX_BCOLS)
      double precision c(MAX_AROWS,MAX_BCOLS)
      double precision buffer(MAX_ACOLS), ans(MAX_ACOLS)
      double precision starttime, stoptime
      integer myid, master, numprocs, ierr, status(MPI_STATUS_SIZE)
      integer i, j, numsent, sender
      integer anstype, row, arows, acols, brows, bcols, crows, ccols
      call MPI_INIT( ierr )
      call MPI_COMM_RANK( MPI_COMM_WORLD, myid, ierr )
      call MPI_COMM_SIZE( MPI_COMM_WORLD, numprocs, ierr )
      arows  = 10
      acols  = 20
      brows  = 20
      bcols  = 10
      crows  = arows
      ccols  = bcols
      call MPE_INIT_LOG()
      if ( myid .eq. 0 ) then
         call MPE_DESCRIBE_STATE(1, 2, "Bcast",  "red:vlines3")
         call MPE_DESCRIBE_STATE(3, 4, "Compute","blue:gray3")
         call MPE_DESCRIBE_STATE(5, 6, "Send",   "green:light_gray")
         call MPE_DESCRIBE_STATE(7, 8, "Recv",   "yellow:gray")
      endif
      if ( myid .eq. 0 ) then
!        master initializes and then dispatches
!        initialize a and b
         do 11 i = 1,acols
            do 10 j = 1,arows
               a(j,i) = i
 10         continue
 11      continue
         do 21 i = 1,bcols
            do 20 j = 1,brows
               b(j,i) = i
 20         continue
 21      continue
         numsent = 0
!        send a row of a to each other process; tag with row number
!        For simplicity, assume arows .ge. numprocs - 1
         do 40 i = 1,numprocs-1
            do 30 j = 1,acols
 30            buffer(j) = a(i,j)
            call MPE_LOG_EVENT(5, i, "send", ierr)
            call MPI_SEND(buffer, acols, MPI_DOUBLE_PRECISION, i, &
                          i, MPI_COMM_WORLD, ierr)
            call MPE_LOG_EVENT(6, i, "sent", ierr)
 40         numsent = numsent+1
         do 70 i = 1,crows
            call MPE_LOG_EVENT(7, i, "recv", ierr)
            call MPI_RECV(ans, ccols, MPI_DOUBLE_PRECISION, &
                 MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status, &
                 ierr)
            sender     = status(MPI_SOURCE)
            anstype    = status(MPI_TAG)
            call MPE_LOG_EVENT(8, anstype, "recvd", ierr)
            do 45 j = 1,ccols
 45            c(anstype,j) = ans(j)
            if (numsent .lt. arows) then
               do 50 j = 1,acols
 50               buffer(j) = a(numsent+1,j)
               call MPE_LOG_EVENT(5, i, "send", ierr)
               call MPI_SEND(buffer, acols, MPI_DOUBLE_PRECISION, &
                             sender, numsent+1, MPI_COMM_WORLD, ierr)
               call MPE_LOG_EVENT(6, i, "sent", ierr)
               numsent = numsent+1
            else
               call MPE_LOG_EVENT(5, 0, "send", ierr)
               call MPI_SEND(1.0, 1, MPI_DOUBLE_PRECISION, sender, &
                             0, MPI_COMM_WORLD, ierr)
               call MPE_LOG_EVENT(6, 0, "sent", ierr)
            endif
 70      continue
      else
!        slaves receive b, then compute rows of c until done message
         call MPE_LOG_EVENT(1, 0, "bstart")
         do 85 i = 1,bcols
         call MPI_BCAST(b(1,i), brows, MPI_DOUBLE_PRECISION, master, &
                        MPI_COMM_WORLD, ierr)
 85      continue
         call MPE_LOG_EVENT(2, 0, "bend")
         call MPE_LOG_EVENT(7, i, "recv")
 90      call MPI_RECV(buffer, acols, MPI_DOUBLE_PRECISION, master, &
                       MPI_ANY_TAG, MPI_COMM_WORLD, status, ierr)
         if (status(MPI_TAG) .eq. 0) then
            go to 200
         else
            row = status(MPI_TAG)
            call MPE_LOG_EVENT(8, row, "recvd")
            call MPE_LOG_EVENT(3, row, "compute")
            do 100 i = 1,bcols
               ans(i) = 0.0
               do 95 j = 1,acols
                  ans(i) = ans(i) + buffer(j)*b(j,i)
 95            continue
 100        continue
            call MPE_LOG_EVENT(4, row, "computed")
            call MPE_LOG_EVENT(5, row, "send")
            call MPI_SEND(ans, bcols, MPI_DOUBLE_PRECISION, master, &
                          row, MPI_COMM_WORLD, ierr)
            call MPE_LOG_EVENT(6, row, "sent")
            go to 90
         endif
 200     continue
      endif
      call MPE_FINISH_LOG("pmatmat.log")
      call MPI_FINALIZE(ierr)
      stop
      end
