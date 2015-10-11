#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>
#include "mpi.h"

int P = 4;
typedef struct flags{
  bool myflags[2][log2(P)];
  bool *partnerflags[2][log2(P)];
}flags_t;

void dissemination_barrier_init()
{
  int rank;
  MPI_Comm_size(MPI_COMM_WORLD, &P);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  flags_t node_i;//TODO: Does this have to be dynamically allocated?
  flags_t* localflags;
  localflags = &node_i;

  //Initializing myflags
  for (i = 0;i<2;i++){
    for (j = 0; j< log2(P);j++){
      localflags->myflags[i][j]=false;
    }
  }

  //Initializing partnerflags
  
}

void dissemination_barrier(bool *sense)
{
  // Be careful of deadlock when using blocking sends and receives!
  MPI_Send(&my_msg, 2, MPI_INT, my_dst, tag, MPI_COMM_WORLD);
  MPI_Recv(&my_msg, 2, MPI_INT, my_src, tag, MPI_COMM_WORLD, &mpi_result);
}

int main (int argc, char ** argv)
{
  MPI_Status mpi_result;

  MPI_Init(&argc, &argv);
  bool pid_sense = true;
  int parity = 0;
  dissemination_barrier_init();
  int i;
  for(i=0;i<N;i++)
  {
    dissemination_barrier(&pid_sense);
    dissemination_barrier(&pid_sense);
    dissemination_barrier(&pid_sense);
    dissemination_barrier(&pid_sense);
    dissemination_barrier(&pid_sense);
  } 
  MPI_Finalize();
  return 0;
}
