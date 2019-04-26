#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#define f(x)   ( 4.0e0 / (1.0e0 + (x)*(x)) )
#define TAG 1

double mysecond();

double MPI_ManualReduce(double mypi)
{
	int id, numprocs;
	double pi;

	MPI_Comm_size( MPI_COMM_WORLD ,&numprocs );
	MPI_Comm_rank( MPI_COMM_WORLD ,&id );
	MPI_Status status;

	if (id == 0) {
		pi = mypi;
		for (int i = 1; i < numprocs; i++) {
			MPI_Recv(&mypi, 1, MPI_DOUBLE_PRECISION, i, TAG, MPI_COMM_WORLD, &status);
			pi+=mypi;
		}
		return pi;
	} else {
		MPI_Send(&mypi, 1, MPI_DOUBLE_PRECISION, 0, TAG, MPI_COMM_WORLD);
	}

	return 0;
}

int main(int argc, char *argv[])
{

  	double PI25DT = 3.141592653589793238462643e0;

  	double  myintegrationtime,integrationtime,integrationtimemax,integrationtimemin,myreductiontime,reductiontime,reductiontimemax,reductiontimemin,mypi, pi, h, sum, x, t0,t1,t2;
  	int     n, i, ierr, id, numprocs;

  	MPI_Status status;
  	MPI_Init(& argc ,& argv );
  	MPI_Comm_size( MPI_COMM_WORLD ,&numprocs );
  	MPI_Comm_rank( MPI_COMM_WORLD ,&id );

  
  	printf("hello from process: %d",id);
  	char line[12];


  	if ( id == 0) {
    		scanf("%d",&n);
  	}
  	MPI_Bcast (&n, 1, MPI_INT , 0 , MPI_COMM_WORLD );

    	h    = 1.0e0/n;           /* Calculate the interval size */
    	sum  = 0.0e0;
    	t0 = mysecond();

    	//integration section
    	for(i = id + 1; i <= n ; i += numprocs )
      	{
        	x = h * ( (double)(i) - 0.5e0 );
        	sum = sum + f(x);
      	}
    	t1 = mysecond();
    
	
    	//Reduction section
    	mypi = h * sum;

    	pi = MPI_ManualReduce(mypi);
    
    	t2 = mysecond();
    

    	//calculate min, max, and averages timings
    	myintegrationtime = t1 - t0;
    	myreductiontime = t2 - t1;

    	MPI_Reduce(&myintegrationtime, &integrationtime , 1 , MPI_DOUBLE , MPI_SUM , 0 , MPI_COMM_WORLD );
    	MPI_Reduce(&myreductiontime, &reductiontime , 1 , MPI_DOUBLE , MPI_SUM , 0 , MPI_COMM_WORLD );
    
   	MPI_Reduce(&myintegrationtime, &integrationtimemax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    	MPI_Reduce(&myreductiontime, &reductiontimemax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
   
    	MPI_Reduce(&myintegrationtime, &integrationtimemin, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD );
    	MPI_Reduce(&myreductiontime, &reductiontimemin, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD );
 
    	if ( id == 0)
      	{
        	printf("\n No. of MPI Processes:        %d",numprocs);
		printf("\n Results:            Calc. pi:%20.16f  \n                     Error:   %20.16f \n", pi, pi - PI25DT );
      
      		printf("\n Integration Timing: Average: %13.9f(s)\n                     Minimum: %13.9f(s)\n                     Maximum: %13.9f(s)\n",integrationtime/numprocs,integrationtimemin,integrationtimemax );
      		printf("\n Reduction Timing:   Average: %13.9f(s)\n                     Minimum: %13.9f(s)\n                     Maximum: %13.9f(s)\n",reductiontime/numprocs,reductiontimemin,reductiontimemax );
      		printf("\n Total Timing:       Average: %13.9f(s)", (reductiontime+integrationtime)/numprocs);
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
