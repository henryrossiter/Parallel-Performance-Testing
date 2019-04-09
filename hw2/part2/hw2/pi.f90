   program main
   implicit none

   integer,   parameter :: KI8 = selected_int_kind( 14)
   integer,   parameter :: KR8 = selected_real_kind(13) 
   real(KR8)            :: PI25DT = 3.141592653589793238462643_KR8

   integer(KI8)         :: ic0, ic1, icountr
   real(KR8)            ::  pi, h, sum, x, f, a, time
   integer              ::  n, i

   f(a) = 4.d0 / (1.d0 + a*a)    !function to integrate

   read(*,*) n

   h = 1.0_KR8/n                 !Calculate the interval size
   sum  = 0.0_KR8
   call system_clock(ic0, icountr)
   call system_clock(ic0)
   do i = 1, n
      x = h * (dble(i) - 0.5_KR8)
      sum = sum + f(x)
   end do
   call system_clock(ic1)

   pi = h * sum

   time = dble(ic1-ic0)/icountr
   write(*,'(" calc. pi:",f20.16,"  Error:",f20.16,3x,f13.9,"(sec)")'), &
   pi, pi - PI25DT, time

   end program
