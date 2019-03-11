      subroutine onedinit( a, b, f, nx, s, e )
      integer nx, s, e
      double precision a(0:nx+1, s-1:e+1), b(0:nx+1, s-1:e+1),
     &                 f(0:nx+1, s-1:e+1)
c
      integer i, j
c
      do 10 j=s-1,e+1
         do 10 i=0,nx+1
            a(i,j) = 0.0d0
            b(i,j) = 0.0d0
            f(i,j) = 0.0d0
 10      continue
c      
c    Handle boundary conditions
c
      do 20 j=s,e
         a(0,j) = 1.0d0
         b(0,j) = 1.0d0
         a(nx+1,j) = 0.0d0
         b(nx+1,j) = 0.0d0
 20   continue
      if (s .eq. 1) then
         do 30 i=1,nx
            a(i,0) = 1.0d0
            b(i,0) = 1.0d0
 30      continue 
      endif
c
      return
      end

