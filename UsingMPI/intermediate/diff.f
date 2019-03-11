c
c  The rest of the 1-d program
c
      double precision function diff( a, b, nx, s, e )
      integer nx, s, e
      double precision a(0:nx+1, s-1:e+1), b(0:nx+1, s-1:e+1)
c
      double precision sum
      integer i, j
c
      sum = 0.0d0
      do 10 j=s,e
         do 10 i=1,nx
            sum = sum + (a(i,j) - b(i,j)) ** 2
 10      continue
c      
      diff = sum
      return
      end
