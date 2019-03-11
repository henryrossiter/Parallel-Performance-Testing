#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <stdint.h>
#include <sys/time.h>

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
{
	for (uint64_t i = 0; i < n; i++)
        {
                for (uint64_t j = 0; j < n; j++)
                {
                arr[i*n+j] = rand()/(float)RAND_MAX;
                }
        }
	printf("starting%u\n",3);
}

void smooth(float *inArr, float *outArr, uint64_t n, float a, float b, float c){
        #pragma omp parallel for
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
        #pragma omp parallel for reduction(+:localcnt)
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
int main()
{
        const float a = 0.05;
        const float b = 0.1;
        const float c = 0.4;
        const float t = 0.1;
        const uint64_t n = 98306;
        float *x; //pointer
        float *y; //pointer
        uint64_t memNeeded = n*n*sizeof(float);
        int numThreads;
	//double gtod_timer(void);
	double start;
        double times[6];
	
	#ifdef _OPENMP
        #pragma omp parallel
	{
                omp_set_num_threads(8);
		printf ("this is thread: %i\n", omp_get_thread_num() );
	}
	#endif        

	start = gtod_timer(); //start clock

        x = (float*)malloc(memNeeded);

        times[0] = gtod_timer();


        y = (float*)malloc(memNeeded);

        times[1] = gtod_timer();

        //x and y are now allocated
        initialize(x, n);

        times[2] = gtod_timer();

        //now x is populated
        smooth(x, y, n, a, b, c);	

        times[3] = gtod_timer();

        //y is now populated, a 'smoothed' version of x

        uint64_t countx = 0;
        uint64_t county = 0;

        count(x, n, t, &countx);

        times[4] = gtod_timer();

        count(y, n, t, &county);

        times[5] = gtod_timer();
        //Print summary
        printf("Summary\n");
        printf("-----------------------------------------------\n");
        printf("Number of threads used in parallel region:   %u\n", numThreads);
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
        //Print timing
        printf ("Process         time(s)    resolution: 1.0e-04\n");
        printf ("-----------------------\n");
        printf ("Allocate-x:     %4.3f\n", times[0]-start);
        printf ("Allocate-y:     %4.3f\n", times[1]-times[0]);
        printf ("Initialize-x:   %4.3f\n", times[2]-times[1]);
        printf ("Smooth:         %4.3f\n", times[3]-times[2]);
        printf ("Count-x:        %4.3f\n", times[4]-times[3]);
        printf ("Count-y:        %4.3f\n", times[5]-times[4]);

        return 0;
}
