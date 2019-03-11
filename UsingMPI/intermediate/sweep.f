c
c Perform a Jacobi sweep for a 1-d decomposition.
c Sweep from a into b
c
      subroutine sweep1d( a, f, nx, s, e, b )
      integer nx, s, e
      double precision a(0:nx+1,s-1:e+1), f(0:nx+1,s-1:e+1),
     +                 b(0:nx+1,s-1:e+1)
c
      integer i, j
      double precision h
c
      h = 1.0d0 / dble(nx+1)
      do 10 j=s, e
         do 10 i=1, nx
            b(i,j) = 0.25 * (a(i-1,j)+a(i,j+1)+a(i,j-1)+a(i+1,j)) - 
     +               h * h * f(i,j)
 10   continue
      return
      end
