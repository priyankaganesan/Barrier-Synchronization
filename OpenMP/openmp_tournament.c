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
    int *opponent;
    int flag;
}record_t;

record_t** players;
int P, N;

void barrier_init()
{
	int i, k;
	int rounds = (int)(log(P)/log(2));
	players = (record_t**)malloc(P * sizeof(record_t*));
	for(i = 0; i < P; i++)
	{
		players[i] = (record_t*)malloc(sizeof(record_t));
	}
	for(i = 0; i < P; i++)
	{
		for(k=0;k<=rounds;k++)
		{
			//Initializing flag
			players[i][k].flag = 0;
			
			//Initializing role
			if (k==0){
				players[i][k].role = DROPOUT;
			}
			else if (k>0){
				if (i == 0 && (1<<k)>=P){
					players[i][k].role = CHAMPION;
					players[i][k].opponent = &players[i+(1<<(k-1))][k].flag;
				}
				else if (i%(1<<k) == 0){
					if (((i + (1<<(k-1))) < P) && ((1<<k) < P)){
                    				players[i][k].role = WINNER;
						players[i][k].opponent = &players[i+(1<<(k-1))][k].flag;
					} else if ((i + (1<<(k-1))) >= P){
						players[i][k].role = BYE;
					}
				}
				else if ((i%(1<<k)) == (1<<(k-1))){
					players[i][k].role = LOSER;
					players[i][k].opponent = &players[i-(1<<(k-1))][k].flag;
				}
			}
		}
	}		
}

void tournament_barrier(int *pid_sense)
{
	int i = 1;
	int vpid = omp_get_thread_num();
	for(i=1; i<=log(P)/log(2); i++)
	{
		switch (players[vpid][i].role)
		{
			case WINNER:
				while (players[vpid][i].flag == *pid_sense);
				break;
			case LOSER:
				players[vpid][i].opponent = pid_sense;
				break;
			case BYE: 
				break;
			case CHAMPION:
				while (players[vpid][i].flag == *pid_sense);
				players[vpid][i].opponent = pid_sense;
				break;
			case DROPOUT:
				break;
		}
	}
	for(i=3; i>0; i--)
	{
		switch (players[vpid][i].role)
		{
			case WINNER:
				players[vpid][i].opponent = pid_sense;
				break;
			case LOSER:
				break;
			case BYE:
				break;
			case CHAMPION:
				break;
			case DROPOUT:
				break;
		}
	}
	*pid_sense = !*pid_sense;	
}

int main(int argc, char* argv[])
{
	//Number of processors
	P = 8;
	//Number of loops
	N = 4;
	int pid_sense = 1;
	barrier_init();
	#pragma omp parallel num_threads(P) firstprivate(pid_sense, N)
	{
		int i;
		for(i=0;i<N;i++)
		{
			tournament_barrier(&pid_sense);
			tournament_barrier(&pid_sense);
			tournament_barrier(&pid_sense);
			tournament_barrier(&pid_sense);
			tournament_barrier(&pid_sense);
		}	
	}
	return 0;
}
