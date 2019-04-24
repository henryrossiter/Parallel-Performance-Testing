#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>
#include <mpi.h>
#define MTAG1 1
#define MTAG2 2

double gtod_secbase = 0.0E0;
double gtod_timer(){
   struct timeval tv;
   struct timezone *Tzp=0;
   double sec;
   gettimeofday(&tv, Tzp);

   if(gtod_secbase == 0.0E0)
      gtod_secbase = (double)tv.tv_sec;
   sec = (double)tv.tv_sec - gtod_secbase;

   return sec + 1.0E-06*(double)tv.tv_usec;
}

void initialize(float *arr, uint64_t n){
        for (uint64_t i = 0; i < n; i++)
        {
                arr[i] = rand()/(float)RAND_MAX;
        }
}

void smooth(float *inArr, float *outArr, uint64_t n, float a, float b, float c){

        for (uint64_t i = 1; i < n-1; i++)
        {
                for (uint64_t j = 1; j < n-1; j++)
                {
                        outArr[i*n+j] = a * (inArr[(i - 1)*n+j - 1] + inArr[(i - 1)*n+j + 1] + inArr[(i + 1)*n+j - 1] + inArr[(i + 1)*n+j + 1]) + b * (inArr[(i)*n+j - 1] + inArr[(i)*n+j + 1] + inArr[(i - 1)*n+j] + inArr[(i + 1)*n+j]) + c * (inArr[(i)*n+j]);
                }
        }
}

void count(float *arr, uint64_t n, float threshold, uint64_t *cnt){
        uint64_t localcnt = *cnt;
        
        for (uint64_t i = 1; i < n-1; i++)
        {
                for (uint64_t j = 1; j < n-1; j++)
                {
                        if (arr[i*n+j] < threshold)
                        {
                                localcnt = localcnt + 1;
                        }
                }
        }
        *cnt = localcnt;

}
int main( int argc, char *argv[] )
{
        const float a = 0.05;
        const float b = 0.1;
        const float c = 0.4;
        const float t = 0.1;
        const uint64_t n = 67;// 98306;
        float *x; //pointer
        float *y; //pointer
        uint64_t memPerProc = n*n*sizeof(float);
        
	double start;
        double times[6];
	
        int myid, numprocs;
    
	MPI_Status status;
	MPI_Init(&argc,&argv);
    	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	int elementsPerProc = n*n/numprocs;
	int colsPerProc = n/numprocs;

	printf("Hello world from %d of %d processors\n",myid, numprocs);


	start = gtod_timer(); //start clock

        x = (float*)malloc(memNeeded/numprocs);

        times[0] = gtod_timer();

        y = (float*)malloc(memNeeded);

        times[1] = gtod_timer();

	if (myid == 0) {
		//for each slave, make a random array and send it to the proc
		printf("elements per proc: %d",elementsPerProc);
		
		for (int islave = 1; islave < numprocs; islave++) {
			initialize(x, elementsPerProc);
			MPI_Send(x, elementsPerProc, MPI_INTEGER, islave, MTAG1, MPI_COMM_WORLD);
		}
		
	} else {
		MPI_Recv(x, elementsPerProc, MPI_INTEGER, 0, MTAG1, MPI_COMM_WORLD, &status);
	}

	printf(" Thread %d 's array starts with %d",myid, x[0]);


        times[2] = gtod_timer();

	if ( myid != 0 ){
		smooth(x, y, n, a, b, c);
	}

        times[3] = gtod_timer();
	uint64_t countx = 0;
        uint64_t county = 0;
	uint64_t countxtot = 0;
	uint64_t countytot = 0;
	
	if (myid != 0){
		count(x, elementsPerProc, t, &countx);
		MPI_Send(&countx, 1, MPI_INTEGER, 0, MTAG2, MPI_COMM_WORLD);	
	} else {
		//receive all counts
		//summ all counts
		MPI_Reduce(&countx,&countxtot,1, MPI_INTEGER, MPI_SUM, MTAG2, MPI_COMM_WORLD);
	}

        times[4] = gtod_timer();

        count(y, n, t, &county);

        times[5] = gtod_timer();
	
	if (myid == 0) {	
		printf("Summary\n");
        	printf("-----------------------------------------------\n");
        	printf("Number of threads used in parallel region:   %u\n", numprocs);
        	printf("Number of elements in a row/column:          %u\n", n);
        	printf("Number of inner elements in a row/column:    %u\n", n-2);
        	printf("Total number of elements:                    %lu\n", n*n);
        	printf("Total number of inner elements:              %lu\n", (n-2)*(n-2));
        	printf("Memory used per array:                       %lu\n", memNeeded);
        	printf("Threshold:                                   %1.2f\n", t);
        	printf("Smoothing constants (a,b,c)                  %1.2f %1.2f %1.2f\n", a,b,c);
        	printf("Number   of elements below threshold (x)     %u\n", countx);
        	printf("Fraction of elements below threshold (x)     %1.8f\n",(float)countx/n/n);
        	printf("Number   of elements below threshold (y)     %u\n", county);
        	printf("Fraction of elements below threshold (y)     %1.8f\n",(float)county/(n-2)/(n-2));

		printf ("Process         time(s)    resolution: 1.0e-04\n");
        	printf ("-----------------------\n");
        	printf ("Allocate-x:     %4.3f\n", times[0]-start);
        	printf ("Allocate-y:     %4.3f\n", times[1]-times[0]);
        	printf ("Initialize-x:   %4.3f\n", times[2]-times[1]);
        	printf ("Smooth:         %4.3f\n", times[3]-times[2]);
        	printf ("Count-x:        %4.3f\n", times[4]-times[3]);
        	printf ("Count-y:        %4.3f\n", times[5]-times[4]);
	}	

	MPI_Finalize();
        return 0;
}

