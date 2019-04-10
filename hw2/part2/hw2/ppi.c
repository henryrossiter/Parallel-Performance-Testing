#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#define f(x)   ( 4.0e0 / (1.0e0 + (x)*(x)) )
#define TAG1 1
#define TAG2 2

double mysecond();

int main(int argc, char *argv[])
{

  double PI25DT = 3.141592653589793238462643e0;

  double  mypi, pi, h, sum, x, t0,t1;
  int     n, i, ierr, myid, numprocs;

  MPI_Init(& argc ,& argv );
  MPI_Comm_size( MPI_COMM_WORLD ,& numprocs );
  MPI_Comm_rank( MPI_COMM_WORLD ,& myid );


  char line[12];



  if ( myid == 0) {
    scanf("%d",&n);
  }
  MPI_Bcast (&n, 1, MPI_INT , 0 , MPI_COMM_WORLD );
  if (n == 0) {
    break;
  }

  else {
    h    = 1.0e0/n;           /* Calculate the interval size */
    sum  = 0.0e0;
    t0 = mysecond();
    for(i = myid + 1; i <= n ; i += numprocs )
      {
        x = h * ( (double)(i) - 0.5e0 );
        sum = sum + f(x);
      }
    t1 = mysecond();
    mypi = h * sum ;
    MPI_Reduce (& mypi , &pi , 1 , MPI_DOUBLE , MPI_SUM , 0 , MPI_COMM_WORLD );
    if ( myid == 0)
      {
      printf(" calc. pi:%20.16f  Error:%20.16f  %13.9f(sec)\n", pi, pi - PI25DT, t1-t0 );
    }
  }

  MPI_Finalize();




  return(0);
}

double mysecond()
{
   struct timeval tp;
   struct timezone tzp;
   int i;
   i = gettimeofday(&tp,&tzp);
   return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}


/* http://www-unix.mcs.anl.gov/mpi/www/ */
