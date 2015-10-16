#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>
#include "mpi.h"

int main (int argc, char ** argv)
{
  MPI_Init(&argc, &argv);
  int myrank;
  int N=1;
  struct timeval tv;
  double curr_time_s;
  double curr_time_us;
  int i, count = 0;
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);

  for(i=0;i<N;i++)
  {
    fflush(stdout);
    gettimeofday(&tv, NULL);
    curr_time_s=(double) tv.tv_usec + (double) tv.tv_sec*1000000;
    printf("Barrier %d reached by %d at time %f\n", count++, myrank,curr_time_s);
    MPI_Barrier(MPI_COMM_WORLD);

    fflush(stdout);
    gettimeofday(&tv, NULL);
    curr_time_s=(double) tv.tv_usec + (double) tv.tv_sec*1000000;
    printf("Barrier %d reached by %d at time %f\n", count++, myrank,curr_time_s);
    MPI_Barrier(MPI_COMM_WORLD);

    fflush(stdout);
    gettimeofday(&tv, NULL);
    curr_time_s=(double) tv.tv_usec + (double) tv.tv_sec*1000000;
    printf("Barrier %d reached by %d at time %f\n", count++, myrank,curr_time_s);
    MPI_Barrier(MPI_COMM_WORLD);

    fflush(stdout);
    gettimeofday(&tv, NULL);
    curr_time_s=(double) tv.tv_usec + (double) tv.tv_sec*1000000;
    printf("Barrier %d reached by %d at time %f\n", count++, myrank,curr_time_s);
    MPI_Barrier(MPI_COMM_WORLD);

    fflush(stdout);
    gettimeofday(&tv, NULL);
    curr_time_s=(double) tv.tv_usec + (double) tv.tv_sec*1000000;
    printf("Barrier %d reached by %d at time %f\n", count++, myrank,curr_time_s);
    MPI_Barrier(MPI_COMM_WORLD);
   
  }
  MPI_Finalize();
  return 0;
}
