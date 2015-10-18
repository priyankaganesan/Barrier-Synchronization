#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>

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
  int rounds = ceil((log(P)/log(2)));
//  printf("Rounds is %d\n", rounds);
  record = (record_t*) calloc(rounds+1,sizeof(record_t));
  int power_of_round;

  for(k=0;k<=rounds;k++)
  {
    //Initializing flag
    record[k].flag = false;
    record[k].send_buffer=1;
    record[k].opponent=-1;
    record[k].role=-1;
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

/*  if(rank==1)
  {
    printf("Roles of each of the players\n");
    int i;
    for(i=0;i<=rounds;i++)
    {
      printf("%d\n",record[i].role);
    }

    printf("Opponent flags for each of the players\n");
    for(i=0;i<=rounds;i++)
    {
      printf("%d\n",record[i].opponent);
    }

  }
  */
}

void tournament_barrier()
{
  int round = 0;
  int flag=1;
  int myrank;
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  
  //arrival
  while(flag)
  {
    round=round+1;
    if(round==1)
    {
//      printf("Processor %d reached barrier in round %d\n", myrank,round);
    }
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
      default:
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
      default:
        continue;        
    }
  }
//  printf("Processor %d finished barrier\n",rank);
}

void tournament_barrier_finish()
{
  free(record);
}


int main(int argc, char **argv)
{
  int my_id, my_dst, my_src, num_processes;
  int i,j,k;
  int test;
  char ch;

  MPI_Init(&argc, &argv);
  int count;
  int myrank;
  struct timeval tv;
  double curr_time_s;
  double curr_time_us;
  struct timeval tv1, tv2;
  int N=100000;
  double total_time;
  
  tournament_barrier_init();
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  
  if(rank == 0)
  {
    gettimeofday(&tv1, NULL);
  }

  for(k=0;k<N;k++)
  {
    
  /*  fflush(stdout);
    gettimeofday(&tv, NULL);
    curr_time_s=(double) tv.tv_usec + (double) tv.tv_sec*1000000;
    printf("Barrier %d reached by %d at time %f\n", count++, myrank,curr_time_s); */
    tournament_barrier();

  /*  fflush(stdout);
    gettimeofday(&tv, NULL);
    curr_time_s=(double) tv.tv_usec + (double) tv.tv_sec*1000000;
    printf("Barrier %d reached by %d at time %f\n", count++, myrank,curr_time_s); */

    tournament_barrier();
  /*  fflush(stdout);
    gettimeofday(&tv, NULL);
    curr_time_s=(double) tv.tv_usec + (double) tv.tv_sec*1000000;
    printf("Barrier %d reached by %d at time %f\n", count++, myrank,curr_time_s);
  */  
    tournament_barrier();
  
  /*  fflush(stdout);
    gettimeofday(&tv, NULL);
    curr_time_s=(double) tv.tv_usec + (double) tv.tv_sec*1000000;
    printf("Barrier %d reached by %d at time %f\n", count++, myrank,curr_time_s);
  */
    tournament_barrier();

    tournament_barrier();

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
  
 tournament_barrier_finish();

  MPI_Finalize();
//  while(1);
  return 0;
}