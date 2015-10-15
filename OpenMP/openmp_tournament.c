/*
 * TOURNAMENT BARRIER: OPENMP
 * To show correct functionality of barrier: Uncomment lines 142 and 149
 * To compile: gcc -o openmp_tournament openmp_tournament.c -lm -fopenmp
 * To run: ./openmp_tournament [num_threads num_barriers]
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
#include <stdbool.h>

#define WINNER 0
#define LOSER 1
#define BYE 2
#define CHAMPION 3
#define DROPOUT 4

typedef struct record{
    int role;
    int opponent;
    int flag;
}record_t;

record_t** players;
int P, N;

void barrier_init()
{
	int i, k;
	int rounds = (ceil(log(P)/log(2)));
	players = (record_t**)malloc(P * sizeof(record_t*));
	for(i = 0; i < P; i++)
	{
		players[i] = (record_t*)calloc(rounds+1, sizeof(record_t));
	}
	for(i = 0; i < P; i++)
	{
		for(k=0;k<=rounds;k++)
		{
			//Initializing
			players[i][k].role = BYE;
			players[i][k].opponent = -1;
			players[i][k].flag = 0;
			
			//Initializing role and opponent
			if (k==0){
				players[i][k].role = DROPOUT;
			}
			else if (k>0){
				if (i == 0 && (1<<k)>=P){
					players[i][k].role = CHAMPION;
					players[i][k].opponent = i+(1<<(k-1));
				}
				else if (i%(1<<k) == 0){
					if (((i + (1<<(k-1))) < P) && ((1<<k) < P)){
                    	players[i][k].role = WINNER;
						players[i][k].opponent = i+(1<<(k-1));
					}
					else if ((i + (1<<(k-1))) >= P){
						players[i][k].role = BYE;
					}
				}
				else if ((i%(1<<k)) == (1<<(k-1))){
					players[i][k].role = LOSER;
					players[i][k].opponent = i-(1<<(k-1));
				}
			}
		}
	}
}

void tournament_barrier(int *pid_sense)
{
	int rounds = (ceil(log(P)/log(2)));
	int round = 1;
	int vpid = omp_get_thread_num();
	int k;

	//Arrival tree
	while(1)
	{
		if (players[vpid][round].role == LOSER){
			players[players[vpid][round].opponent][round].flag = *pid_sense;
			while (players[vpid][round].flag != *pid_sense);
			break;
		}
		else if (players[vpid][round].role == WINNER){
			while (players[vpid][round].flag != *pid_sense);
		}
		else if (players[vpid][round].role == CHAMPION){
			while (players[vpid][round].flag != *pid_sense);
			players[players[vpid][round].opponent][round].flag = *pid_sense;
			break;
		}
		round += 1;
		if (round > rounds)
			break;
	}
	//Wakeup tree
	while(1)
	{
		if (round == -1)
			break;
		round -= 1;
		if (players[vpid][round].role == WINNER){
			players[players[vpid][round].opponent][round].flag = *pid_sense;
		}
		else if (players[vpid][round].role == DROPOUT)
			break;
	}
	*pid_sense = !*pid_sense;
}

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
	barrier_init();
	omp_set_num_threads(P);
	gettimeofday(&tv1, NULL);
	#pragma omp parallel shared (players,N) firstprivate(pid_sense)
	{
		int i;
		for(i=0;i<N;i++)
		{
			// printf("==============BARRIER %d=================\n", i);
			tournament_barrier(&pid_sense);
			tournament_barrier(&pid_sense);
			tournament_barrier(&pid_sense);
			tournament_barrier(&pid_sense);
			tournament_barrier(&pid_sense);
		}
		// printf("Barriers executed\n");	
	}
	gettimeofday(&tv2, NULL);
	total_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec)*1000000;
    printf("\nSUMMARY:\nTotal run-time for %d "
            "loops with 5 barriers per loop: %fs\n"
            "The average time per barrier: %fus\n",
            N, total_time/1000000, (double)(total_time/(N*5)));
	free(players);
	return 0;
}
