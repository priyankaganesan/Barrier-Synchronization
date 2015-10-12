#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>

#define DROPOUT 4
#define WINNER 0
#define LOSER 1
#define BYE 2
#define CHAMPION 3

typedef struct 
{
  int role;
  int opponent;
  int flag;
  MPI_Status status;
  int send_buffer;
  int receive_buffer;
} record_t;

record_t* record;
int P,N;
int rank;
bool globalsense = true;

void tournament_barrier_init()
{
  int i, k;
  MPI_Comm_size(MPI_COMM_WORLD,&P);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  int rounds = (int)(log(P)/log(2));
  printf("Rounds is %d\n", rounds);
  record = (record_t*)malloc(sizeof(record_t));
  int power_of_round;

  for(k=0;k<=rounds;k++)
  {
    //Initializing flag
    record[k].flag = false;
    record[k].send_buffer=1;
    power_of_round=1<<(k-1);
      
    //Initializing role
    if (k==0)
    {
      record[k].role = DROPOUT;
    }
    else if (k>0)
    {
      if (rank == 0 && (1<<k)>=P)
      {
        record[k].role = CHAMPION;
        record[k].opponent = power_of_round + rank;
      }
      else if (rank%(1<<k) == 0)
      {
        if (((rank + (1<<(k-1))) < P) && ((1<<k) < P))
        {
          record[k].role = WINNER;
          record[k].opponent = power_of_round + rank;
        } 
        else if ((rank + (1<<(k-1))) >= P)
        {
          record[k].role = BYE;
        }
      }
      else if ((rank%(1<<k)) == (1<<(k-1)))
      {
        record[k].role = LOSER;
        record[k].opponent = rank - power_of_round;
      }
    }
      //Initializing opponent
//    record[k].opponent = false;
  }
}

void tournament_barrier()
{
  int round = 0;
  int flag=1;
  
  //arrival
  while(flag)
  {
    round=round+1;
    printf("Processor %d reached barrier in round %d\n", rank,round);
    switch(record[round].role)
    {
      case WINNER:
        MPI_Recv(&record[round].receive_buffer, 1, MPI_INT, record[round].opponent, round, MPI_COMM_WORLD,&record[round].status);
        break;
      case LOSER:
        MPI_Send(&record[round].send_buffer, 1, MPI_INT, record[round].opponent, round, MPI_COMM_WORLD);
        MPI_Recv(&record[round].receive_buffer, 1, MPI_INT, record[round].opponent, round, MPI_COMM_WORLD, &record[round].status);
        flag=0;
        break;
      case CHAMPION:
        MPI_Recv(&record[round].receive_buffer, 1, MPI_INT, record[round].opponent, round, MPI_COMM_WORLD, &record[round].status);
        MPI_Send(&record[round].send_buffer, 1, MPI_INT, record[round].opponent, round, MPI_COMM_WORLD);
        flag=0;
        break;
      case BYE:
        continue;
      case DROPOUT:
        continue;
    }
  }
  flag=1;

  //wake up
  while(flag)
  {
    round = round - 1;
    switch(record[round].role)
    {
      case WINNER:
        MPI_Send(&record[round].send_buffer, 1, MPI_INT, record[round].opponent, round, MPI_COMM_WORLD);
        continue;
      case LOSER:
      case CHAMPION:
      case BYE:
        continue;
      case DROPOUT:
        flag=0;
        break;        
    }
  }
  printf("Processor %d finished barrier\n",rank);
}

void tournament_barrier_finish()
{
  free(record);
}


int main(int argc, char **argv)
{
  int my_id, my_dst, my_src, num_processes;
  int i;
 
  MPI_Init(&argc, &argv);

  tournament_barrier_init();

  for(i=0;i<5;i++)
  {
    tournament_barrier();
    tournament_barrier();
    tournament_barrier();
    tournament_barrier();
  }
  

  MPI_Finalize();
  return 0;
}