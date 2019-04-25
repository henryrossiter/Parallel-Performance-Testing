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

void smooth(float *inArr, float *outArr, float *colLeft, float *colRight, float *rowAbove, float *rowBelow, uint64_t n, float a, float b, float c){
        for (uint64_t i = 0; i < n; i++) // y coordinate
        {
                for (uint64_t j = 0; j < n; j++) //x coordinate
                {
                        //top border conditions
                        outArr[i*n+j] = c * inArr[(i)*n+j];
                        if (i == 0) {
                          outArr[i*n+j] = outArr[i*n+j] + ( b * (rowAbove[j] + inArr[(i + 1)*n+j]) );
                        } else if (i == n - 1) {
                          outArr[i*n+j] = outArr[i*n+j] + ( b * (inArr[(i - 1)*n+j] + rowBelow[j]) );
                        } else {
                          outArr[i*n+j] = outArr[i*n+j] + ( b * (inArr[(i - 1)*n+j] + inArr[(i + 1)*n+j]) );
                        }

                        // left and right border conditions
                        if (j == 0) {
                          outArr[i*n+j] = outArr[i*n+j] + ( b * (colLeft[i] + inArr[(i)*n+j + 1] );
                        } else if (j == n - 1) {
                          outArr[i*n+j] = outArr[i*n+j] + ( b * (inArr[(i)*n+j - 1] + colRight[i] );
                        } else {
                          outArr[i*n+j] = outArr[i*n+j] + ( b * (inArr[(i)*n+j - 1] + inArr[(i)*n+j + 1] );
                        }
                }
        }
}

void count(int *my_coord, int gridSize, float *arr, uint64_t n, float threshold, uint64_t *cnt){
        uint64_t localcnt = *cnt;

        uint64_t i = 0;
        uint64_t j = 0;
        uint64_t iend = n;
        uint64_t jend = n;

        //dont count the edges of the edge processes
        if (my_coord[0] == 0) {
          i = 1
        }if (my_coord[0] == gridSize - 1) {
          iend = n - 1
        }if (my_coord[1] == 0) {
          j = 1
        }if (my_coord[1] == gridSize - 1) {
          jend = n - 1
        }

        for (i = 0; i < iend; i++)
        {
                for (j = 0; j < jend; j++)
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
  const uint64_t n = 67;
  const uint64_t elementsPerProc = n*n;
  const uint64_t memPerProc = elementsPerProc*sizeof(float);
  int i;


  float *x; //pointer
  float *y; //pointer

	double start;
  double times[6];

  int myid, numprocs;

	MPI_Status status;
	MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);

	printf("Hello world from %d of %d processors\n",myid, numprocs);


  //mpi cartesian grid
  //create a grid of processes
  int ndim = 2;

  int dimensions[2];
  dimensions[0] = gridSize;
  dimensions[1] = gridSize;

  MPI_Comm comm2d;

  int periodic[2];
  periodic[0] = 0;
  periodic[1] = 0;

  int rank_2d; //literal int
  int my_coord[2]; //pointer

  MPI_Cart_create(MPI_COMM_WORLD, ndim, dimensions, periodic, 1, &comm2d);
  MPI_Cart_coords(comm2d, myid, ndim, my_coord);
  MPI_Cart_rank(comm2d, my_coord, &rank_2d);
  printf("I am %d: (%d,%d); originally %d\n", rank_2d, my_coord[0], my_coord[1], myid );

	start = gtod_timer(); //start clock

  x = (float*)malloc(memPerProc);

  times[0] = gtod_timer();

  y = (float*)malloc(memPerProc);

  times[1] = gtod_timer();
  //start of initialization kernel a

	if (myid == 0) {
		//for each process, make a random array and send it

		for (int islave = 1; islave < numprocs; islave++) {
			initialize(x, elementsPerProc);
			MPI_Send(x, elementsPerProc, MPI_INTEGER, islave, MTAG1, MPI_COMM_WORLD);
		}

    //initialize "my own" array
    initialize(x, elementsPerProc);

	} else {
		MPI_Recv(x, elementsPerProc, MPI_INTEGER, 0, MTAG1, MPI_COMM_WORLD, &status);
	}

	printf(" Thread %d 's array starts with %d",myid, x[0]);

  //end of initialization kernel a
  times[2] = gtod_timer();

  int gridSize = sqrt(numprocs)

  float colLeft[gridSize];
  float colRight[gridSize];
  float rowAbove[gridSize];
  float rowBelow[gridSize];

  int ind;

  //store current processes left column in an array
  ind = 0;
  float myColLeft[gridSize];
  for (i = 0; i< elementsPerProc; i++) {
    if (i % n == 0) {
      myLeftColumn[ind] = x[i];
      ind = ind + 1
    }
  }

  //store current processes Right column in an array
  ind = 0;
  float myColRight[gridSize];
  for (i = 0; i < elementsPerProc; i++) {
    if (i % n == n - 1) {
      myRightColumn[ind] = x[i];
      ind = ind + 1
    }
  }

  float myRowTop[gridSize];
  for (i=0; i < n; i++){
    myTopRow[i] = x[i]
  }

  float myRowBottom[gridSize];
  for (i=0; i < n; i++){
    myBottomRow[i] = x[elementsPerProc - n + i]
  }

  MPI_Cart_shift(comm2d,0,+1,&rank_2d,&rank_right);
  MPI_Cart_shift(comm2d,0,-1,&rank_2d,&rank_left);
  MPI_Cart_shift(comm2d,1,+1,&rank_2d,&rank_up);
  MPI_Cart_shift(comm2d,1,-1,&rank_2d,&rank_down);

  int irequest = 0;
  MPI_Request *requests = malloc(8*sizeof(MPI_Request));

  //send to rank_left
  if (my_coord[0] > 0) {
    MPI_Isend( myColLeft,1,MPI_FLOAT,rank_left,  0,comm, requests+irequest++);
    MPI_Irecv( colLeft, 1,MPI_FLOAT, rank_left,  0,comm, requests+irequest++);
  }

  //send to rank_right
  if (my_coord[0] < gridSize - 1) {
    MPI_Isend( myColRight ,1,MPI_FLOAT,rank_right, 0,comm, requests+irequest++);
    MPI_Irecv( colRight, 1,MPI_FLOAT, rank_right, 0,comm, requests+irequest++);
  }

  //send to rank_up
  if (my_coord[1] > 0) {
    MPI_Isend( myRowTop,1,MPI_FLOAT,rank_up, 0,comm, requests+irequest++);
    MPI_Irecv( rowAbove, 1,MPI_FLOAT, rank_up, 0,comm, requests+irequest++);
  }

  //send to rank_down
  if (my_coord[1] < gridSize - 1) {
    MPI_Isend( myRowBottom,1,MPI_FLOAT,rank_down, 0,comm, requests+irequest++);
    MPI_Irecv( rowBelow, 1,MPI_FLOAT, rank_down, 0,comm, requests+irequest++);
  }

	smooth(x, y, colLeft, colRight, colUp, colDown, n, a, b, c);

  times[3] = gtod_timer();

	uint64_t countx = 0;
  uint64_t county = 0;
	uint64_t countxtot = 0;
	uint64_t countytot = 0;


	count(my_coord, gridSize, y, n, t, &county);

  MPI_Reduce(&countx,&countxtot,1, MPI_INTEGER, MPI_SUM, 0, MPI_COMM_WORLD);

  times[4] = gtod_timer();

  count(my_coord, gridSize, y, n, t, &county);

  MPI_Reduce(&county,&countytot,1, MPI_INTEGER, MPI_SUM, 0, MPI_COMM_WORLD);

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
