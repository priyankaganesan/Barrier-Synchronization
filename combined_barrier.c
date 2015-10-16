/*
 * COMBINED OPENMP-MPI BARRIER
 * OpenMP: Centralized sense reversal
 * MPI: Tournament
 * To show correct functionality of barrier: Uncomment lines 142 and 149
 * To compile: mpicc -o combined_barrier combined_barrier.c -lm -fopenmp
 * To run: ./openmp_tournament [num_threads num_barriers]
 */

#include <omp.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "mpi.h"

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
int P, N, T;
int rank;

int startcount;
bool globalSense = 1;
bool *localSense;

void SenseReversalBarrier_Init()
{
	int i;
  startcount = T;
	localSense = (bool*) malloc(sizeof(bool)*(T));
	for (i = 0; i < T; ++i) 
		localSense[i] = true;
	globalSense = true;
}

void tournament_barrier_init()
{
  int i, k;
  MPI_Comm_size(MPI_COMM_WORLD,&P);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  int rounds = ceil((log(P)/log(2)));
  record = (record_t*) calloc(rounds+1,sizeof(record_t));
  int power_of_round;

  for(k=0;k<=rounds;k++)
  {
    //Initializing
    record[k].flag = false;
    record[k].opponent = -1;
    record[k].role = -1;

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
  }
}

void centralized_tournament_init()
{
	SenseReversalBarrier_Init();
	tournament_barrier_init();
}

void tournament_barrier(int barrier)
{
  int round = 0;
  int flag=1;
  
  //arrival
  while(flag)
  {
    round=round+1;
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
}

//Gets the current count and decrements it. Also returns the current count
int FetchAndDecrementCount()
{ 
  int myCount;
  #pragma omp critical
  {
    myCount = startcount;
    startcount--;
  }
  return myCount;
}

void combined_barrier(int barrier)
{
  int j, myCount;
	int thread_num = omp_get_thread_num();
	printf("Thread %d, rank %d, Entering combined_barrier %d\n", thread_num, rank, barrier);
  localSense[thread_num] = !localSense[thread_num]; // Toggle private sense variable
  if (FetchAndDecrementCount() == 1)
  {
		startcount = T;
		globalSense = 1 - globalSense;
	}
	else
	{
		while (globalSense != localSense[thread_num]); // Spin
	}
	if(thread_num == 0){
    tournament_barrier(barrier);
  }
}

int main(int argc, char **argv)
{
	struct timeval tv1, tv2;
	double total_time;
	int ret_val = MPI_Init(&argc, &argv);
	if (ret_val != MPI_SUCCESS)
	{
	    printf("Failure initializing MPI\n");
	    MPI_Abort(MPI_COMM_WORLD, ret_val);
	}
	if (argc==3){
		if (sscanf (argv[1], "%d", &T)!=1) printf ("T - not an integer\n");
		if (sscanf (argv[2], "%d", &N)!=1) printf ("N - not an integer\n");
	}
	else {T = 3; N = 2;}
	centralized_tournament_init();
  gettimeofday(&tv1, NULL);
	#pragma omp parallel num_threads(T) shared(record, N)
	{
		int i;
		// thread_num = omp_get_thread_num();
		for (i = 0; i < N; ++i)
		{
      printf("==============BARRIER %d=================\n", i);
			combined_barrier(i);
      printf("==============BARRIER %d=================\n", i);
      combined_barrier(i);
      printf("==============BARRIER %d=================\n", i);
      combined_barrier(i);
      printf("==============BARRIER %d=================\n", i);
      combined_barrier(i);
      printf("==============BARRIER %d=================\n", i);
      combined_barrier(i);
		}
	}
  gettimeofday(&tv2, NULL);
  total_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec)*1000000;
  printf("\nSUMMARY:\nTotal run-time for %d "
            "loops with 5 barriers per loop: %fs\n"
            "The average time per barrier: %fus\n",
            N, total_time/1000000, (double)(total_time/(N*5)));
  free(record); 
  MPI_Finalize();
  return 0;
}