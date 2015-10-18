#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>
#include "mpi.h"

int P;
int rounds;
//int rank;


int main (int argc, char ** argv)
{
  

  MPI_Init(&argc, &argv);
 
  int rank;
  struct timeval tv;
  double curr_time_s;
  double curr_time_us;

  int i,j,k,count=0;
  struct timeval tv1, tv2;
  int N=10000;
  double total_time;

  MPI_Comm_size(MPI_COMM_WORLD, &P);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  if(rank == 0)
  {
    gettimeofday(&tv1, NULL);
  }
  for(i=0;i<N;i++)
  {

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
   
  }
  if(rank==0)
  {
    gettimeofday(&tv2, NULL);

    total_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec)*1000000;
    printf("\nSUMMARY:\nNumber of processes: %d\n Total run-time for %d "
            "loops with 5 barriers per loop: %fs\n"
            "The average time per barrier: %fus\n",
            P, N, total_time/1000000, (double)(total_time/(N*5)));
  }

  MPI_Finalize();
  return 0;
}