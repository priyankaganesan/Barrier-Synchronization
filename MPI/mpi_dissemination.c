/*
 * To compile: mpicc mpi_dissemination.c -o mpi_dissemination
 * To run: mpiexec -n 4 mpi_dissemination
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>
#include "mpi.h"


int * partnerflag;
int * receiveflag;
int P;
int rounds;
int rank;

void dissemination_barrier_init()
{
  
  MPI_Comm_size(MPI_COMM_WORLD, &P);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  rounds = ceil(log2(P));
  partnerflag = malloc(sizeof(int)*rounds);
  receiveflag = malloc(sizeof(int)*rounds);


  int i,j;
  for(i=0;i<rounds;i++)
  {
    partnerflag[i]=(rank+(1<<i))%P;
    receiveflag[i]=(rank-(1<<i))%P;
    if(receiveflag[i]<0)
      receiveflag[i]=receiveflag[i]+P;
/*
    if(rank==0)
    {
      printf("%d\n", partnerflag[i]);
      printf("%d\n", receiveflag[i]);
    }
*/
  }
}

void dissemination_barrier(bool *sense)
{
  // Be careful of deadlock when using blocking sends and receives!
  int send_buffer=1;
  int receive_buffer;
  MPI_Status Status;
  int i;
  for(i=0;i<rounds;i++)
  {
    MPI_Send(&send_buffer, 1, MPI_INT, partnerflag[i], i, MPI_COMM_WORLD);
    printf("Processor %d sent message to processor %d\n",rank,partnerflag[i]);
    MPI_Recv(&receive_buffer, 1, MPI_INT, receiveflag[i], i, MPI_COMM_WORLD, &Status);
    printf("Processor %d received message from processor %d\n",rank,receiveflag[i]);
  }
  //MPI_Recv(&my_msg, 2, MPI_INT, my_src, tag, MPI_COMM_WORLD, &mpi_result);
}

void dissemination_barrier_finish()
{
  free(partnerflag);
  free(receiveflag);  
}

int main (int argc, char ** argv)
{
  

  MPI_Init(&argc, &argv);
  bool pid_sense = true;
  int parity = 0;
  int myrank;
  int N=1;
  dissemination_barrier_init();
  int i,j,k;
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);

  for(i=0;i<N;i++)
  {
    dissemination_barrier(&pid_sense);
    printf("Barrier reached by %d\n", myrank);
    dissemination_barrier(&pid_sense);
    printf("Barrier reached by %d\n", myrank);
    dissemination_barrier(&pid_sense);
    printf("Barrier reached by %d\n", myrank);
    dissemination_barrier(&pid_sense);
    printf("Barrier reached by %d\n", myrank);
    dissemination_barrier(&pid_sense);
    printf("Barrier reached by %d\n", myrank);
  }


  dissemination_barrier_finish();
  MPI_Finalize();
  return 0;
}
