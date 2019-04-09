#!/bin/tcsh

setenv OMP_NUM_THREADS 8

foreach i ( "STATIC,100" "dynamic,100" "STATIC,1000" "dynamic,1000" "STATIC,10000" "dynamic,10000" "STATIC,100000" "dynamic,100000" )
  setenv OMP_SCHEDULE $i
  echo "schedule: $OMP_SCHEDULE"

  numactl --cpunodebind=0 --preferred=0 ./a.out
end
