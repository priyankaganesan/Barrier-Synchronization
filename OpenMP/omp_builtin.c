/*
 * BUILT IN BARRIER: OPENMP
 * To show correct functionality of barrier: Uncomment lines 142 and 145
 * To compile: gcc -o openmp_tournament openmp_tournament.c -lm -fopenmp
 * To run: ./openmp_tournament [num_threads num_barriers]
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
#include <stdbool.h>

int P, N;

int main(int argc, char* argv[])
{
	if (argc == 3){
        if (sscanf (argv[1], "%d", &P)!=1)
        	printf ("P - not an integer\n");
        if (sscanf (argv[2], "%d", &N)!=1)
        	printf ("N - not an integer\n");
    }
    else{
    	//Number of processors
		P = 4;
		//Number of loops
		N = 5;
    }
    struct timeval tv1, tv2;
    double total_time;
	int pid_sense = 1;
	omp_set_num_threads(P);
	gettimeofday(&tv1, NULL);
	#pragma omp parallel shared (N) firstprivate(pid_sense)
	{
		int i;
		for(i=0;i<N;i++)
		{
			// printf("==============BARRIER %d=================\n", i);
			#pragma omp barrier
			#pragma omp barrier
			#pragma omp barrier
			#pragma omp barrier
			#pragma omp barrier
		}
		// printf("Barriers executed\n");	
	}
	gettimeofday(&tv2, NULL);
	total_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec)*1000000;
    printf("\nSUMMARY:\nTotal run-time for %d "
            "loops with 5 barriers per loop: %fs\n"
            "The average time per barrier: %fus\n",
            N, total_time/1000000, (double)(total_time/(N*5)));
	return 0;
}