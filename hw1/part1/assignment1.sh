#!/bin/tcsh

   echo " a.out"
   foreach i ( 1 8 1 8 1 8)
      setenv OMP_NUM_THREADS $i
      numactl --cpunodebind=0 --preferred=0 ./a.out
   end
