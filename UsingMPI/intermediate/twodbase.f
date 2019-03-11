      subroutine twodinit( a, b, f, nx, sx, ex, sy, ey )
      integer nx, sx, ex, sy, ey
      double precision a(sx-1:ex+1, sy-1:ey+1), b(sx-1:ex+1, sy-1:ey+1),
     &                 f(sx-1:ex+1, sy-1:ey+1)
c
      integer i, j
c
      do 10 j=sy-1,ey+1
         do 10 i=sx-1,ex+1
            a(i,j) = 0.0d0
            b(i,j) = 0.0d0
            f(i,j) = 0.0d0
 10      continue
c      
c    Handle boundary conditions
c
      if (sx .eq. 1) then 
         do 20 j=sy,ey
            a(0,j) = 1.0d0
            b(0,j) = 1.0d0
 20      continue
      endif
      if (ex .eq. nx) then
         do 21 j=sy,ey
            a(nx+1,j) = 0.0d0
            b(nx+1,j) = 0.0d0
 21      continue
      endif 
      if (sy .eq. 1) then
         do 30 i=sx,ex
            a(i,0) = 1.0d0
            b(i,0) = 1.0d0
 30      continue 
      endif
c
      return
      end

