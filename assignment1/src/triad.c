#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <omp.h>
#include <assert.h>

double get_curr_time(){
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

double calculateChecksum(long datasetSize, const volatile double* vector) {
    double checksum = 0;
    for (long i = 0; i < datasetSize; i++) {
        checksum += vector[i];
    }
    return checksum;
}

// TASK 1.b
double triad (const long N, const long REP, int *numThreads){
    double time_spent = 0.0;
    double begin, end;

// TASK 1.c
//#pragma omp parallel
//{
	*numThreads=omp_get_num_threads();
//}

// TASK 1.d
    double* a = (double*) aligned_alloc (4096, N * sizeof(double)); //this only works for N>256
    double* b = (double*) aligned_alloc (4096, N * sizeof(double));
    double* c = (double*) aligned_alloc (4096, N * sizeof(double));
    double* d = (double*) aligned_alloc (4096, N * sizeof(double));

// TASK 1.e
#pragma omp parallel for schedule(static)
    for (long j=0; j<N; j++) {
	    a[j] = 0.0;
	    b[j] = 1.0;
	    c[j] = 2.0;
	    d[j] = 3.0;
    }

// TESTING TASK 1.f
#pragma omp parallel
{
    for (long i=0; i<REP; i++)
#pragma omp for schedule(static) nowait
//#pragma omp for schedule(static) 
        for (long j=0; j<N; j++)
            a[j] = b[j]+c[j]*d[j];
}

// TASK 1.g
    begin = get_curr_time();
#pragma omp parallel
{
    for (long i=0; i<REP; i++)
#pragma omp for schedule(static) nowait
        for (long j=0; j<N; j++)
            a[j] = b[j]+c[j]*d[j];
}
    end = get_curr_time();
    time_spent = end - begin;

// TASK 1.h
    double sum = calculateChecksum(N, a);
    assert (abs(sum-N*7.0)<0.1);
    
    free(a); free(b); free(c); free(d);

    return time_spent;
}

int main(int argc, char * argv[]) {
    long int N, REP;
    double time_a;
    double performance_a;

    if (argc != 3 && argc != 4){
        printf("The two parameters N and REP need to be provided.\n");
        exit(1);
    }

    char * pEnd;
    N = strtol (argv[1],&pEnd,10);
    if (errno == ERANGE){
        printf("Problem with the first number.");
        exit(2);
    }
    REP = strtol (argv[2],&pEnd,10);
    if (errno == ERANGE){
        printf("Problem with the second number.");
        exit(3);
    }

    fprintf(stderr, "N = %ld and REP = %ld. Performance in MFLOPS.\n", N, REP);

    printf("| %12s | %12s | %12s | %12s |\n","Dataset size", "Threads", "MFLOPS", "Cycles");

    long datasetSize = N; //for experiment 2e: set to the maximum vector size input
    double m_flop = 0;
    int threads;
    long cycles=10;

    // TASK 1.a
    //while (datasetSize <= N && cycles > 1)
    //{
        cycles = REP; /// datasetSize; for experiment 2e: deleted 

        time_a = triad(datasetSize, cycles, &threads); //send to triad the address of the buffer
        m_flop = 2.0 * (double)datasetSize * (double)cycles * 1.0e-6;
        performance_a = m_flop / time_a;

        printf("| %12ld | %12d | %12.2f | %12ld |\n", datasetSize, threads, performance_a, cycles);

        //datasetSize *= 2;
    //}

    return 0;
}
