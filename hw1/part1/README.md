# This folder contains HW 1, Part 1.

part1.c can be executed serially with the following commands:

```
$ ./compile-serial
$ umactl --cpunodebind=0 --preferred=0 ./a.out
```

part1.c can be executed in on X threads in parallel with the following commands:

```
$ export OMP_NUM_THREADS=X
$ ./compile
$ umactl --cpunodebind=0 --preferred=0 ./a.out
```
