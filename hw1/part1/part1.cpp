#include <stdlib.h>
#include <thread>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <cstdint>

void initialize(float* arr, uint64_t n){
        for (size_t i = 0; i < n; i++)
        {
                for (size_t j = 0; j < n; j++)
                {
                arr[i*n+j] = rand()/(float)RAND_MAX;
                }
        }
}

void smooth(float* inArr, float* outArr, uint64_t n, float a, float b, float c){
        int i, j;
        #pragma omp parallel for private(i, j)
        for (i = 1; i < n-1; i++)
        {
                for (j = 1; j < n-1; j++)
                {
                        outArr[i*n+j] = a * (inArr[(i - 1)*n+j - 1] + inArr[(i - 1)*n+j + 1] + inArr[(i + 1)*n+j - 1] + inArr[(i + 1)*n+j + 1]) + b * (inArr[(i)*n+j - 1] + inArr[(i)*n+j + 1] + inArr[(i - 1)*n+j] + inArr[(i + 1)*n+j]) + c * (inArr[(i)*n+j]);
                }
        }

}

void count(float* arr, uint64_t n, float threshold, uint64_t &cnt){
        int i, j;
        #pragma omp parallel for reduction(+:cnt) private(i, j)
        for (i = 1; i < n-1; i++)
        {
                for (j = 1; j < n-1; j++)
                {
                        if (arr[i*n+j] < threshold)
                        {
                                cnt = cnt + 1;
                        }
                }
        }

}
int main()
{
        const float a = 0.05;
        const float b = 0.1;
        const float c = 0.4;
        const float t = 0.1;
        const uint64_t n = 98304;
        float *x; //pointer
        float *y; //pointer
        uint64_t memNeeded = n*n*sizeof(float);
        int numThreads;
        clock_t start;
        float times[6];

        #pragma omp parallel
        {
                printf ("This is thread num %i\n", omp_get_thread_num() );
                numThreads = omp_get_num_threads();
        }

        start = clock(); //start clock

        x = (float*)malloc(memNeeded);

        times[0] = (clock() - start)/(float) CLOCKS_PER_SEC;
        start = clock(); //restart clock

        y = (float*)malloc(memNeeded);

        times[1] = (clock() - start)/(float) CLOCKS_PER_SEC;
        start = clock(); //restart clock

        //x and y are now allocated
        printf("%u got this far\n", 0);

        initialize(x, n);

        printf("%u got this far\n", 0);

        times[2] = (clock() - start)/(float) CLOCKS_PER_SEC;
        start = clock(); //restart clock
        //now x is populated
        smooth(x, y, n, a, b, c);

        times[3] = (clock() - start)/(float) CLOCKS_PER_SEC;
        start = clock(); //restart clock

        //y is now populated, a 'smoothed' version of x

        uint64_t countx = 0;
        uint64_t county = 0;

        count(x, n, t, countx);

        times[4] = (clock() - start)/(float) CLOCKS_PER_SEC;
        start = clock(); //restart clock

        count(y, n, t, county);

        times[5] = (clock() - start)/(float) CLOCKS_PER_SEC;
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
        printf("Fraction of elements below threshold (x)     %1.8f\n", countx/(n*n));
        printf("Number   of elements below threshold (y)     %u\n", county);
        printf("Fraction of elements below threshold (y)     %1.8f\n", county/(n-2)/(n-2));
        //Print timing
        printf ("Process         time(s)    resolution: 1.0e-04\n");
        printf ("-----------------------\n");
        printf ("Allocate-x:     %4.3f\n", times[0]);
        printf ("Allocate-y:     %4.3f\n", times[1]);
        printf ("Initialize-x:   %4.3f\n", times[2]);
        printf ("Smooth:         %4.3f\n", times[3]);
        printf ("Count-x:        %4.3f\n", times[4]);
        printf ("Count-y:        %4.3f\n", times[5]);

        return 0;
}
