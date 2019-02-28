#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <cstdlib>
#include <thread>
#include <stdio.h>      /* printf */
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
using namespace std;

void initialize(float* arr, int n){
        int i, j;
        for (i = 0; i < n; i++)
        {
                for (j = 0; j < n; j++)
                {
                arr[i*n+j] = rand()/(float)RAND_MAX;
                }
        }
}

void smooth(float* inArr, float* outArr, int n, float a, float b, float c){
        int i, j;
        for (i = 1; i < n-1; i++)
        {
                for (j = 1; j < n-1; j++)
                {
                        outArr[i*n+j] = a * (inArr[(i - 1)*n+j - 1] + inArr[(i - 1)*n+j + 1] + inArr[(i + 1)*n+j - 1] + inArr[(i + 1)*n+j + 1]) + b * (inArr[(i)*n+j - 1] + inArr[(i)*n+j + 1] + inArr[(i - 1)*n+j] + inArr[(i + 1)*n+j]) + c * (inArr[(i)*n+j]);
                }
        }
}

void count(float* arr, int n, float threshold, long &cnt){
        int i, j;
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
