./a.out

numactl --cpunodebind=0 --preferred=0 ./a.out

numactl --cpunodebind=0 --interleave=0,1 ./a.out

numactl --cpunodebind=0 --preferred=1 ./a.out
